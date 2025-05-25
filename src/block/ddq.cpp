#include<fs.h>
void blockScheduler::loadchild(vector<dir_entry>&a,inode id)
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(dinode)+id.i_block[0]*512,SEEK_SET);
    dir_entry root;
    size_t num;
    fread(&root,sizeof(dir_entry),1,fp);
    fread(&num,sizeof(size_t),1,fp);
    dir_entry temp;
    for(int i=0;i<min(num,(size_t)MAXnumInBlock-1);i++)
    {
        fread(&temp,sizeof(dir_entry),1,fp);
        a.push_back(temp);
    }   
    num-=min(num,(size_t)MAXnumInBlock-1);
    fclose(fp);
    if(num==0)
        return;
    if(num<=MAXnumInBlock*(MAXnumInBlock+1))
    {
        ftree(id.i_block[1],min((size_t)MAXnumInBlock,num),a);
        num-=min((size_t)MAXnumInBlock,num);
        if(num==0)
            return ;
        ftree(id.i_block[2],num,a);
    }
    else if(num>MAXnumInBlock*(MAXnumInBlock+1)&&num<=2*MAXnumInBlock*MAXnumInBlock)
    {
        ftree(id.i_block[1],num-MAXnumInBlock*MAXnumInBlock,a);
        ftree(id.i_block[2],MAXnumInBlock*MAXnumInBlock,a);
    }
    else
    {
        ftree(id.i_block[1],MAXnumInBlock*MAXnumInBlock,a);
        ftree(id.i_block[2],num-MAXnumInBlock*MAXnumInBlock,a);
    }
}
size_t blockScheduler::fastpow(size_t di,int x)
{
    if(x==0)
        return 1;
    size_t mid=fastpow(di,x/2);
    if(x%2==0)
        return mid*mid;
    else
        return mid*mid*di;
}
void blockScheduler::ftree(size_t block_id,size_t n,vector<dir_entry>&a)
{
    queue<pair<size_t,size_t>>tree;//防止递归产生文件错误
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(dinode)+block_id*512,SEEK_SET);
    dir_entry temp;
    if(n<=MAXnumInBlock)
    {
        for(size_t i=0;i<n;i++)
        {
            fread(&temp,sizeof(dir_entry),1,fp);
            a.push_back(temp);
        }
        fclose(fp);
        return;
    }
    double avg=log(double(n)/double(MAXnumInBlock))/log(double(MAXnumInBlock));
    size_t intavg=floor(avg);
    size_t treenum=fastpow(intavg,MAXnumInBlock);
    if(treenum*MAXnumInBlock>n)
    {
        treenum=1;
        while(treenum*MAXnumInBlock<=n)
            treenum=treenum*MAXnumInBlock;
    }
    int mayMx=n/(treenum*MAXnumInBlock);
    int mayx=MAXnumInBlock-mayMx;
    int mayH=0;
    if(mayMx*(treenum*MAXnumInBlock)+mayx*treenum<n)
    {
        mayx--;
        mayH++;
    }
    else if(mayMx*(treenum*MAXnumInBlock)+mayx*treenum>n)
    {
        mayMx--;
        mayH++;
    }
    size_t temp_block_id;
    for(int i=0;i<mayx;i++)
    {
        if(treenum==1)
        {
            fread(&temp,sizeof(dir_entry),1,fp);
            a.push_back(temp);
        }
        else
        {
            fseek(fp,sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
            fread(&temp_block_id,sizeof(size_t),1,fp);
            tree.push({temp_block_id,(size_t)treenum});
        }
    }
    if(mayH)
    {
        fseek(fp,sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        fread(&temp_block_id,sizeof(size_t),1,fp);
        tree.push({temp_block_id,(size_t)(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx)});
    }
    for(int i=0;i<mayMx;i++)
    {
        fseek(fp,sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        fread(&temp_block_id,sizeof(size_t),1,fp);
        tree.push({temp_block_id,(size_t)(treenum*MAXnumInBlock)});
    }
    fclose(fp);
    while(!tree.empty())
    {
        ftree(tree.front().first,tree.front().second,a);
        tree.pop();
    }
}
size_t blockScheduler::cal_block_num_dir(size_t n_dir)
{
    n_dir++;//现在包含父节点的个数
    if(n_dir<3*MAXnumInBlock)
        return (n_dir+MAXnumInBlock-1)/MAXnumInBlock;
    size_t RZ=1;//只有直接索引所占块数
    n_dir-=3;//抛去直接索引的内容
    if(n_dir<=2*MAXnumInBlock*MAXnumInBlock)//最后一块还没有进行多级索引
    {
        RZ+=2;//加上i_block直接指向的块数目,
        //其余再次基础上快数目与前一次增加量分别为//{1,0_{1},0_{2},...0_{MAXnumInBlock-2}}循环
        //相当于将之前位写磁盘块号，将其移入新磁盘，产生空闲位置MAXnumInBlock-2
        //循环节大小为MAXnumInBlock-1,除法，向上取整
        n_dir-=2*MAXnumInBlock;
        RZ+=(n_dir+MAXnumInBlock-2)/(MAXnumInBlock-1);
        return RZ;
    }
    else
    {
        RZ+=(1+MAXnumInBlock);//加上一级索引以及其对应的块数目
        n_dir-=MAXnumInBlock*MAXnumInBlock;//减去以及索引的条目;
        //现在形成一个完全N叉树,N=MAXnumInBlock,n_dir个子节点
        //一个树的所有子节点都占用一个磁盘块，计算出所有节点个数n,ceil((n-1)/MAXnumInBlock)即为块号(未采用)
        //当前MAXnumInBlock节点，(在这种情况下dir个数大于MAXnumInBlock)占一块
        //增长方式依然{1,0_{1},0_{2},...0_{MAXnumInBlock-2}}循环
        n_dir-=MAXnumInBlock;
        RZ+=1;
        RZ+=(n_dir+MAXnumInBlock-2)/(MAXnumInBlock-1);
        return RZ;
    }
}