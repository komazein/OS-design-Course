#include<fs.h>
void blockScheduler::loadchild(vector<dir_entry>&a,inode &id)
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+id.i_block[0]*512,SEEK_SET);
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+id.i_block[0]*512,SEEK_SET);
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
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+block_id*512,SEEK_SET);
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
    size_t treenum=fastpow(MAXnumInBlock,intavg);
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
size_t blockScheduler::cal_block_num_dir(size_t n_dir)//only child
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
bool blockScheduler::creatFILE(size_t old_num_only_child,inode &parid,inode &chid)
{
    if(sb->getfreeBlocknum()==0)
        return false;
    else if(sb->getfreeBlocknum()==1)
    {
        if(cal_block_num_dir(old_num_only_child)!=cal_block_num_dir(old_num_only_child+1))//cal:only child
            return false;
    }
    if(cal_block_num_dir(old_num_only_child)!=cal_block_num_dir(old_num_only_child+1))
    {
        writeBlockIDdir(old_num_only_child,parid);
    }
    if(chid.i_type==DIR)
    {
        vector<size_t>temp_vec;
        sb->getblock(1,temp_vec);
        chid.i_block[0]=temp_vec[0];
    }
    return true;
}
void blockScheduler::writeBlockIDdir(size_t n,inode &id)//only child, old num
{
    n++;// now with parent
    vector<size_t>temp_vec;
    sb->getblock(1,temp_vec);
    size_t block_id_temp=temp_vec[0];
    if(n<3*MAXnumInBlock)
    {
        id.i_block[n/MAXnumInBlock]=(size_t)block_id_temp;
        return ;
    }
    n-=MAXnumInBlock;
    if(n>=MAXnumInBlock+MAXnumInBlock*MAXnumInBlock&&n<2*MAXnumInBlock*MAXnumInBlock)
    {
        changeDirentryToblockID(id.i_block[1],n-MAXnumInBlock*MAXnumInBlock,(size_t)block_id_temp);
        return ;
    }
    if(n<MAXnumInBlock+MAXnumInBlock*MAXnumInBlock)
    {
        changeDirentryToblockID(id.i_block[2],n-MAXnumInBlock,(size_t)block_id_temp);
        return ;
    }
    if(n>=2*MAXnumInBlock*MAXnumInBlock)
    {
        changeDirentryToblockID(id.i_block[2],n-MAXnumInBlock*MAXnumInBlock,(size_t)block_id_temp);
        return ;
    }
}

void blockScheduler::changeDirentryToblockID(size_t now_block_id,size_t n,size_t block_id)//n with parent
{
    FILE *fp=fopen("../disk.img","r+");
    //if(n<MAXnumInBlock)不会存在该情况
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+now_block_id*512,SEEK_SET);
    double avg=log(double(n)/double(MAXnumInBlock))/log(double(MAXnumInBlock));
    size_t intavg=floor(avg);
    size_t treenum=fastpow(MAXnumInBlock,intavg);
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
    if(treenum==1)//寻找此时不会存在残差块
    {
        fseek(fp,sizeof(dir_entry)*mayx-sizeof(size_t),SEEK_CUR);
        fwrite(&block_id,sizeof(size_t),1,fp);
        fclose(fp);
        return;
    }
    if(mayH)
    {
        fseek(fp,mayx*sizeof(dir_entry)+sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        size_t temp;
        fread(&temp,sizeof(size_t),1,fp);
        fclose(fp);
        changeDirentryToblockID(temp,(size_t)(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx),block_id);
        return;
    }
    else
    {
        fseek(fp,mayx*sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        size_t temp;
        fread(&temp,sizeof(size_t),1,fp);
        fclose(fp);
        changeDirentryToblockID(temp,(size_t)(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx),block_id);
        return;
    }
}

size_t blockScheduler::getlastblockID(size_t now_block_id,size_t n,inode &id)
{
    n++;
    if(n<3*MAXnumInBlock)
        return id.i_block[n/MAXnumInBlock];
    else if(n<2*MAXnumInBlock+MAXnumInBlock*MAXnumInBlock)
        return treeFindLastBlock(id.i_block[2],n-2*MAXnumInBlock);
    else if(n<MAXnumInBlock+2*MAXnumInBlock*MAXnumInBlock)
        return treeFindLastBlock(id.i_block[1],n-MAXnumInBlock-MAXnumInBlock*MAXnumInBlock);
    else
        return treeFindLastBlock(id.i_block[2],n-MAXnumInBlock-MAXnumInBlock*MAXnumInBlock);
}
size_t blockScheduler::treeFindLastBlock(size_t now_block_id,size_t n)
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+now_block_id*512,SEEK_SET);
    double avg=log(double(n)/double(MAXnumInBlock))/log(double(MAXnumInBlock));
    size_t intavg=floor(avg);
    size_t treenum=fastpow(MAXnumInBlock,intavg);
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
    fseek(fp,mayx*sizeof(dir_entry)+sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
    size_t temp;
    fread(&temp,sizeof(size_t),1,fp);
    fclose(fp);
    if(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx==2)
        return temp;
    else
        return treeFindLastBlock(temp,n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx);
}

void blockScheduler::writechild(dir_entry par,vector<dir_entry>&a,inode &id,size_t num)
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+id.i_block[0]*512,SEEK_SET);
    fwrite(&par,sizeof(dir_entry),1,fp);
    fwrite(&num,sizeof(size_t),1,fp);
    for(int i=0;i<min(num,(size_t)MAXnumInBlock-1);i++)
    {
        fwrite(&a[a.size()-1],sizeof(dir_entry),1,fp);
        a.erase(a.end()-1);
    }   
    num-=min(num,(size_t)MAXnumInBlock-1);
    fclose(fp);
    if(num==0)
        return;
    if(num<=MAXnumInBlock*(MAXnumInBlock+1))
    {
        fwtree(id.i_block[1],min((size_t)MAXnumInBlock,num),a);
        num-=min((size_t)MAXnumInBlock,num);
        if(num==0)
            return ;
        fwtree(id.i_block[2],num,a);
    }
    else if(num>MAXnumInBlock*(MAXnumInBlock+1)&&num<=2*MAXnumInBlock*MAXnumInBlock)
    {
        fwtree(id.i_block[1],num-MAXnumInBlock*MAXnumInBlock,a);
        fwtree(id.i_block[2],MAXnumInBlock*MAXnumInBlock,a);
    }
    else
    {
        fwtree(id.i_block[1],MAXnumInBlock*MAXnumInBlock,a);
        fwtree(id.i_block[2],num-MAXnumInBlock*MAXnumInBlock,a);
    }
}
void blockScheduler::fwtree(size_t block_id,size_t n,vector<dir_entry>&a)
{
    queue<pair<size_t,size_t>>tree;//防止递归产生文件错误
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+block_id*512,SEEK_SET);
    if(n<=MAXnumInBlock)
    {
        for(size_t i=0;i<n;i++)
        {
            fwrite(&a[a.size()-1],sizeof(dir_entry),1,fp);
            a.erase(a.end()-1);
        }
        fclose(fp);
        return;
    }
    double avg=log(double(n)/double(MAXnumInBlock))/log(double(MAXnumInBlock));
    size_t intavg=floor(avg);
    size_t treenum=fastpow(MAXnumInBlock,intavg);
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
            fwrite(&a[a.size()-1],sizeof(dir_entry),1,fp);
            a.erase(a.end()-1);
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
        fwtree(tree.front().first,tree.front().second,a);
        tree.pop();
    }
}
void blockScheduler::getallBlockDIR(inode &id,size_t num,vector<size_t>&a)
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+id.i_block[0]*512,SEEK_SET);
    fseek(fp,sizeof(dir_entry),SEEK_CUR);
    fwrite(&num,sizeof(size_t),1,fp);
    fseek(fp,min(num,(size_t)MAXnumInBlock-1),SEEK_CUR);   
    num-=min(num,(size_t)MAXnumInBlock-1);
    fclose(fp);
    if(num==0)
        return;
    vector<size_t>notneed;
    if(num<=MAXnumInBlock*(MAXnumInBlock+1))
    {
        getblockTree(id.i_block[1],min((size_t)MAXnumInBlock,num),a,notneed,DIR);
        num-=min((size_t)MAXnumInBlock,num);
        if(num==0)
            return ;
        getblockTree(id.i_block[2],num,a,notneed,DIR);
    }
    else if(num>MAXnumInBlock*(MAXnumInBlock+1)&&num<=2*MAXnumInBlock*MAXnumInBlock)
    {
        getblockTree(id.i_block[1],num-MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
        getblockTree(id.i_block[2],MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
    }
    else
    {
        getblockTree(id.i_block[1],MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
        getblockTree(id.i_block[2],num-MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
    }
}
void blockScheduler::getblockTree(size_t block_id,size_t n,vector<size_t>&a,vector<size_t>&forSIMfile,TYPE type)
{
    queue<pair<size_t,size_t>>tree;//防止递归产生文件错误
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+block_id*512,SEEK_SET);
    dir_entry temp;
    if(n<=MAXnumInBlock)
    {
        if(type==SIM_FILE)
        {
            for(size_t i=0;i<n;i++)
            {
                size_t blk_id;
                fread(&blk_id,sizeof(size_t),1,fp);
                a.push_back(blk_id);
                forSIMfile.push_back(blk_id);
            }
        }
        fclose(fp);
        return;
    }
    double avg=log(double(n)/double(MAXnumInBlock))/log(double(MAXnumInBlock));
    size_t intavg=floor(avg);
    size_t treenum=fastpow(MAXnumInBlock,intavg);
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
            if(type==SIM_FILE)
            {
                fread(&temp_block_id,sizeof(size_t),1,fp);
                a.push_back(temp_block_id);
                forSIMfile.push_back(temp_block_id);
            }
            else
            {
                fseek(fp,mayx*sizeof(dir_entry),SEEK_CUR);
                break;
            }
        }
        else
        {
            if(type==DIR)
                fseek(fp,sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
            fread(&temp_block_id,sizeof(size_t),1,fp);
            tree.push({temp_block_id,(size_t)treenum});
            a.push_back(temp_block_id);
        }
    }
    if(mayH)
    {
        if(type==DIR)
            fseek(fp,sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        fread(&temp_block_id,sizeof(size_t),1,fp);
        tree.push({temp_block_id,(size_t)(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx)});
        a.push_back(temp_block_id);
    }
    for(int i=0;i<mayMx;i++)
    {
        if(type==DIR)
            fseek(fp,sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        fread(&temp_block_id,sizeof(size_t),1,fp);
        tree.push({temp_block_id,(size_t)(treenum*MAXnumInBlock)});
        a.push_back(temp_block_id);
    }
    fclose(fp);
    // for(int i=0;i<tree.size();i++)
    // {
    //     auto m=tree.front();
    //     tree.pop();
    //     cout<<"("<<m.first<<","<<m.second<<")";
    //     tree.push(m);
    // }
    // cout<<endl;
    while(!tree.empty())
    {
        getblockTree(tree.front().first,tree.front().second,a,forSIMfile,type);
        tree.pop();
    }
}
void blockScheduler::getallBlockSIM(inode &id,vector<size_t>&a,vector<size_t>&forSIMfile)
{
    //cout<<"byte"<<id.i_size<<endl;
    size_t num=(id.i_size+511)/512;
    for(size_t i=0;i<min((size_t)ONLY_DIRECT_SIM,num);i++)
    {
        a.push_back(id.i_block[i]);
        forSIMfile.push_back(id.i_block[i]);
    }
    num-=min((size_t)ONLY_DIRECT_SIM,num);
    if(num==0)
        return;
    if(num<=MAXnumInBlock*ABLE_ONE_SIM)
    {
        for(size_t i=ONLY_DIRECT_SIM;i<ABLE_DIRECT_SIM;i++)
        {
            size_t TEMPNUM=1;
            if(TEMPNUM+(ABLE_DIRECT_SIM-1-i)*MAXnumInBlock<num)
            {
                TEMPNUM=MAXnumInBlock;
                if(TEMPNUM+(ABLE_DIRECT_SIM-1-i)*MAXnumInBlock>num)
                    TEMPNUM=num-(ABLE_DIRECT_SIM-1-i)*MAXnumInBlock;
            }
            if(TEMPNUM==1)
            {
                forSIMfile.push_back(id.i_block[i]);
                a.push_back(id.i_block[i]);
            }
            else
            {
                a.push_back(id.i_block[i]);
                getblockTree(id.i_block[i],TEMPNUM,a,forSIMfile,SIM_FILE);
            }
            num-=TEMPNUM;
            if(num==0)
                return;
        }
    }
    for(size_t i=ONLY_DIRECT_SIM;i<ONLY_DIRECT_SIM+ONLY_ONE_SIM;i++)
    {
        a.push_back(id.i_block[i]);
        getblockTree(id.i_block[i],MAXnumInBlock,a,forSIMfile,SIM_FILE);
        num-=MAXnumInBlock;
    }
    a.push_back(id.i_block[ABLE_DIRECT_SIM-ABLE_MULTI_SIM]);
    getblockTree(id.i_block[ABLE_DIRECT_SIM-ABLE_MULTI_SIM],num,a,forSIMfile,SIM_FILE);
    // for(int i=0;i<forSIMfile.size();i++)
    //     cout<<"("<<i<<","<<forSIMfile[i]<<")";
    // cout<<endl;
}
char*blockScheduler::readSIMfromBLOCK(inode&id)
{
    vector<size_t>mid;
    vector<size_t>real_block_id;
    getallBlockSIM(id,mid,real_block_id);
    char*a=(char*)calloc(id.i_size+10,sizeof(char));
    FILE *fp=fopen("../disk.img","r+");
    size_t num=id.i_size;
    size_t delta=0;
    for(size_t i=0;i<real_block_id.size();i++)
    {
        fseek(fp,real_block_id[i]*512+sizeof(super_block)+sizeof(inode)*INODENUM,SEEK_SET);
        fread(&a[delta],sizeof(char),min((size_t)512,num),fp);
        delta+=min((size_t)512,num);
        num-=min((size_t)512,num);
    }
    fclose(fp);
    return a;
}
size_t blockScheduler::calSIMblockNUM(size_t blockNUM)
{
    if(blockNUM<=ABLE_DIRECT_SIM)
        return blockNUM;
    return blockNUM+(blockNUM-ABLE_DIRECT_SIM+MAXnumInBlock-2)/(MAXnumInBlock-1);
}
bool blockScheduler::writeSIMfromBLOCK(inode&id,char*a)
{
    vector<size_t>all;
    vector<size_t>real_block_id;
    getallBlockSIM(id,all,real_block_id);
    id.i_size=strlen(a);
    //cout<<id.i_size<<",";
    size_t num_block_need=(id.i_size+511)/512;
    //cout<<num_block_need<<","<<endl;
    size_t need_num=calSIMblockNUM(num_block_need);
    bool ablegetblock=0;
    // for(int i=0;i<all.size();i++)
    //     cout<<all[i]<<" ";
    // cout<<endl;
    if(need_num>all.size())
    {
        ablegetblock=sb->getblock(need_num-all.size(),all);
        if(ablegetblock==0)
            return false;
    }
    if(need_num<all.size())
        sb->releaseblock(all.size()-need_num,all);
    // cout<<"bknum"<<all.size()<<endl;
    // for(int i=0;i<all.size();i++)
    //     cout<<all[i]<<" ";
    // cout<<endl;
    writeBlocknumFORsim(all,strlen(a),id,a);
    for(int i=0;i<all.size();i++)
        cout<<i<<" "<<all[i]<<endl;
    writeBlocknumFORsim(all,strlen(a),id,a);
    return true;
}
void blockScheduler::SIMwriteBK(vector<size_t>newlist,size_t n,char*a)
{
    size_t now_byte=n;
    FILE *fp=fopen("../disk.img","r+");
    size_t delta=0;
    for(int i=0;i<newlist.size();i++)
    {
        fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+newlist[i]*512,SEEK_SET);
        fwrite(&a[delta],sizeof(char),min(now_byte,(size_t)512),fp);
        delta+=min(now_byte,(size_t)512);
        now_byte-=min(now_byte,(size_t)512);
    }
    fclose(fp);
}
void blockScheduler::writeBlocknumFORsim(vector<size_t>&all,size_t n,inode&id,char*a)
{
    size_t num=(id.i_size+511)/512;
    vector<size_t>newlist;
    for(size_t i=0;i<min((size_t)ONLY_DIRECT_SIM,num);i++)
    {
        id.i_block[i]=all[all.size()-1];
        newlist.push_back(all[all.size()-1]);
        all.erase(all.end()-1);
    }
    num-=min((size_t)ONLY_DIRECT_SIM,num);
    if(num==0)
    {
        SIMwriteBK(newlist,id.i_size,a);
        return;
    }
    if(num<=MAXnumInBlock*ABLE_ONE_SIM)
    {
        for(size_t i=ONLY_DIRECT_SIM;i<ABLE_DIRECT_SIM;i++)
        {
            size_t TEMPNUM=1;
            if(TEMPNUM+(ABLE_DIRECT_SIM-1-i)*MAXnumInBlock<num)
            {
                TEMPNUM=MAXnumInBlock;
                if(TEMPNUM+(ABLE_DIRECT_SIM-1-i)*MAXnumInBlock>num)
                    TEMPNUM=num-(ABLE_DIRECT_SIM-1-i)*MAXnumInBlock;
            }
            // cout<<"num"<<num<<endl;
            // cout<<"TEMPNUM:"<<TEMPNUM<<" "<<(ABLE_DIRECT_SIM-1-i)*MAXnumInBlock<<endl;
            if(TEMPNUM==1)
            {
                newlist.push_back(all[all.size()-1]);
                id.i_block[i]=all[all.size()-1];
                all.erase(all.end()-1);
            }
            else
            {
                id.i_block[i]=all[all.size()-1];
                all.erase(all.end()-1);
                simwriteTree(id.i_block[i],all,TEMPNUM,newlist);
            }
            num-=TEMPNUM;
            if(num==0)
            {
                SIMwriteBK(newlist,id.i_size,a);
                return;
            }
        }
    }
    for(size_t i=ONLY_DIRECT_SIM;i<ONLY_DIRECT_SIM+ONLY_ONE_SIM;i++)
    {
        id.i_block[i]=all[all.size()-1];
        all.erase(all.end()-1);
        simwriteTree(id.i_block[i],all,MAXnumInBlock,newlist);
        num-=MAXnumInBlock;
    }
    id.i_block[ABLE_DIRECT_SIM-ABLE_MULTI_SIM]=all[all.size()-1];
    all.erase(all.end()-1);
    simwriteTree(id.i_block[ABLE_DIRECT_SIM-ABLE_MULTI_SIM],all,MAXnumInBlock,newlist);
    size_t now_byte=n;
    FILE *fp=fopen("../disk.img","r+");
    for(int i=0;i<newlist.size();i++)
        
    for(int i=0;i<newlist.size();i++)
    {
        fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+newlist[i]*512,SEEK_SET);
        fwrite(a,sizeof(char),max(now_byte,(size_t)512),fp);
        now_byte-=max(now_byte,(size_t)512);
    }
}
void blockScheduler::simwriteTree(size_t block_id,vector<size_t>&all,size_t n,vector<size_t>&newlist)
{
    // cout<<endl<<block_id<<" "<<n<<endl;
    queue<pair<size_t,size_t>>tree;//防止递归产生文件错误
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+block_id*512,SEEK_SET);
    dir_entry temp;
    if(n<=MAXnumInBlock)
    {
        for(size_t i=0;i<n;i++)
        {
            fwrite(&all[all.size()-1],sizeof(size_t),1,fp);
            newlist.push_back(all[all.size()-1]);
            all.erase(all.end());
        }
        fclose(fp);
        return;
    }
    double avg=log(double(n)/double(MAXnumInBlock))/log(double(MAXnumInBlock));
    size_t intavg=floor(avg);
    size_t treenum=fastpow(MAXnumInBlock,intavg);
    //cout<<double(n)/double(MAXnumInBlock)<<" "<<log(double(MAXnumInBlock))<<" "<<intavg<<endl;
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
    // cout<<mayx<<" "<<mayH<<" "<<mayMx<<endl;
    for(int i=0;i<mayx;i++)
    {
        if(treenum==1)
        {
            fwrite(&all[all.size()-1],sizeof(size_t),1,fp);
            newlist.push_back(all[all.size()-1]);
            //cout<<"x:all"<<all[all.size()-1]<<" ";
            all.erase(all.end());
        }
        else
        {
            fwrite(&all[all.size()-1],sizeof(size_t),1,fp);
            tree.push({all[all.size()-1],(size_t)treenum});
            //cout<<"x:_trall"<<all[all.size()-1]<<" ";
            all.erase(all.end());
        }
    }
    if(mayH)
    {
        fwrite(&all[all.size()-1],sizeof(size_t),1,fp);
        tree.push({all[all.size()-1],(size_t)(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx)});
        //cout<<"H:_trall"<<all[all.size()-1]<<" ";
        all.erase(all.end());
    }
    for(int i=0;i<mayMx;i++)
    {
        fwrite(&all[all.size()-1],sizeof(size_t),1,fp);
        tree.push({all[all.size()-1],(size_t)(treenum*MAXnumInBlock)});
        //cout<<"Mx:_trall"<<all[all.size()-1]<<" ";
        all.erase(all.end());
    }
    fclose(fp);
    // for(int i=0;i<tree.size();i++)
    // {
    //     auto m=tree.front();
    //     tree.pop();
    //     cout<<"("<<m.first<<","<<m.second<<")";
    //     tree.push(m);
    // }
    // cout<<endl;
    while(!tree.empty())
    {
        simwriteTree(tree.front().first,all,tree.front().second,newlist);
        tree.pop();
    }
}

void blockScheduler::new_disk()
{

    sb->newdisk();
}
inode*blockScheduler::iget(bool ifroot)
{
    return sb->iget(ifroot);
}
size_t blockScheduler::getfreeblocknum()
{
    return sb->getfreeBlocknum();
}
