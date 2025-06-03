#include<fs.h>
void blockScheduler::loadchild(vector<dir_entry>&a,inode &id)
{
    FILE *fp=fopen("../disk.img","r+");
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
    {
        return;
    }
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
    cout<<block_id<<" "<<n<<endl;
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
    if(n_dir<=3*MAXnumInBlock)
        return (n_dir+MAXnumInBlock-1)/MAXnumInBlock;
    size_t RZ=3;
    n_dir-=3*MAXnumInBlock;//1,0_{1},0_{2},0_{MAXnumInblock-2}
    return RZ+(n_dir+MAXnumInBlock-2)/(MAXnumInBlock-1);
}
bool blockScheduler::creatFILE(size_t old_num_only_child,inode &parid,inode &chid)
{
    if(sb->getfreeBlocknum()==0)
        return false;
    else if(sb->getfreeBlocknum()==1&&chid.i_type==DIR)
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
        cout<<id.i_block[n/MAXnumInBlock]<<endl;
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
    if(mayH)///完全树中,残缺块大小会为完全树。
    {
        if(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx==1)
        {

            fseek(fp,mayx*sizeof(dir_entry)+sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
            fwrite(&block_id,sizeof(size_t),1,fp);
            fclose(fp);
            return;
        }
        fseek(fp,mayx*sizeof(dir_entry)+sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        size_t temp;
        fread(&temp,sizeof(size_t),1,fp);
        fclose(fp);
        changeDirentryToblockID(temp,(size_t)(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx),block_id);
        return;
    }
    else if(treenum==1)
    {
        fseek(fp,sizeof(dir_entry)*mayx-sizeof(size_t),SEEK_CUR);
        fwrite(&block_id,sizeof(size_t),1,fp);
        fclose(fp);
        return;
    }
    else if(treenum>1)
    {
        fseek(fp,mayx*sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
        size_t temp;
        fread(&temp,sizeof(size_t),1,fp);
        fclose(fp);
        changeDirentryToblockID(temp,(size_t)treenum,block_id);
        return;
    }
    //下面情况无
    // else
    // {
    //     fseek(fp,mayx*sizeof(dir_entry)-sizeof(size_t),SEEK_CUR);
    //     size_t temp;
    //     fread(&temp,sizeof(size_t),1,fp);
    //     fclose(fp);
    //     changeDirentryToblockID(temp,(size_t)(n-treenum*mayx-(treenum*MAXnumInBlock)*mayMx),block_id);
    //     return;
    // }
}

size_t blockScheduler::getlastblockID(size_t n,inode &id)
{
    n++;//MAXnumInBlock>3，下面不用取等，取等不释放
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
void blockScheduler::freeblock(vector<pair<inode, size_t>>&del_nodes,inode&par,size_t primsizeofchild)
{
    vector<size_t>need;
    vector<size_t>emptyvec;
    if(cal_block_num_dir(primsizeofchild)!=cal_block_num_dir(primsizeofchild-1))
        need.push_back(getlastblockID(primsizeofchild,par));
    for(size_t i=0;i<del_nodes.size();i++)
    {
        if(del_nodes[i].first.i_type==DIR)
            getallBlockDIR(del_nodes[i].first,del_nodes[i].second,need);
        else
            getallBlockSIM(del_nodes[i].first,need,emptyvec);
        sb->freeinode(del_nodes[i].first.i_num);
    }
    sb->releaseblock(need.size(),need);
    return;
}
void blockScheduler::freeSIMFILE(inode&ino)/////////未测试
{
    vector<size_t>need;
    vector<size_t>emptyvec;
    getallBlockSIM(ino,need,emptyvec);
    sb->freeinode(ino.i_num);
    sb->releaseblock(need.size(),need);
    return;
}
void blockScheduler::writechild(dir_entry par,vector<dir_entry>&a,inode &id,size_t num)
{
    ReWrinode(id,false);
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
    cout<<block_id<<" "<<n<<endl;
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
    a.push_back(id.i_block[0]);
    fclose(fp);
    if(num==0)
        return;
    vector<size_t>notneed;
    if(num<=MAXnumInBlock*(MAXnumInBlock+1))
    {
        a.push_back(id.i_block[1]);
        getblockTree(id.i_block[1],min((size_t)MAXnumInBlock,num),a,notneed,DIR);
        num-=min((size_t)MAXnumInBlock,num);
        if(num==0)
            return ;
        a.push_back(id.i_block[2]);
        getblockTree(id.i_block[2],num,a,notneed,DIR);
    }
    else if(num>MAXnumInBlock*(MAXnumInBlock+1)&&num<=2*MAXnumInBlock*MAXnumInBlock)
    {
        a.push_back(id.i_block[1]);
        getblockTree(id.i_block[1],num-MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
        a.push_back(id.i_block[2]);
        getblockTree(id.i_block[2],MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
    }
    else
    {
        a.push_back(id.i_block[1]);
        getblockTree(id.i_block[1],MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
        a.push_back(id.i_block[2]);
        getblockTree(id.i_block[2],num-MAXnumInBlock*MAXnumInBlock,a,notneed,DIR);
    }
}
void blockScheduler::getblockTree(size_t block_id,size_t n,vector<size_t>&a,vector<size_t>&forSIMfile,TYPE type)
{
    // cout<<"rn"<<n<<endl;
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
                // cout<<"nk::"<<block_id<<"bk::::: "<<blk_id<<" "<<ftell(fp)<<endl;
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
                // cout<<"temp:"<<temp_block_id<<endl;
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
        // cout<<"now:"<<block_id<<" "<<temp_block_id<<endl;
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
    // cout<<"::"<<id.i_block[ABLE_DIRECT_SIM-ABLE_MULTI_SIM]<<endl;
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
    // for(size_t i=0;i<real_block_id.size();i++)
    //     cout<<real_block_id[i]<<" ";
    // cout<<endl;
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
    size_t num_block_need=(strlen(a)+511)/512;
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
    else if(need_num<all.size())
        sb->releaseblock(all.size()-need_num,all);
    id.i_size=strlen(a);
    // cout<<"bknum"<<all.size()<<endl;
    // for(int i=0;i<all.size();i++)
    //     cout<<all[i]<<" ";
    // cout<<endl;
    writeBlocknumFORsim(all,strlen(a),id,a);
    return true;
}
void blockScheduler::SIMwriteBK(vector<size_t>newlist,size_t n,char*a)
{
    // cout<<",,,"<<newlist.size()<<endl;
    size_t now_byte=n;
    FILE *fp=fopen("../disk.img","r+");
    size_t delta=0;
    for(int i=0;i<newlist.size();i++)
    {
        // cout<<i<<","<<newlist[i]<<endl;
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
    // cout<<"writeBlocknumFORsim"<<endl;
    // for(int i=0;i<all.size();i++)
    //     cout<<all[i]<<" ";
    // cout<<endl;
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
    // cout<<"WR_last:iblock"<<id.i_block[ABLE_DIRECT_SIM-ABLE_MULTI_SIM]<<endl;
    all.erase(all.end()-1);
    simwriteTree(id.i_block[ABLE_DIRECT_SIM-ABLE_MULTI_SIM],all,num,newlist);
    size_t now_byte=n;
    FILE *fp=fopen("../disk.img","r+");
    // cout<<"get_new_list"<<endl;
    // for(int i=0;i<newlist.size();i++)
    // {
    //     cout<<newlist[i]<<" ";
    // }
    // cout<<endl;
    SIMwriteBK(newlist,id.i_size,a);
}
void blockScheduler::simwriteTree(size_t block_id,vector<size_t>&all,size_t n,vector<size_t>&newlist)
{
    // cout<<endl<<block_id<<" "<<n<<endl;
    queue<pair<size_t,size_t>>tree;//防止递归产生文件错误
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+block_id*512,SEEK_SET);
    dir_entry temp;
    // cout<<"n:"<<n<<endl;
    if(n<=MAXnumInBlock)
    {
        for(size_t i=0;i<n;i++)
        {
            fwrite(&all[all.size()-1],sizeof(size_t),1,fp);
            // cout<<"nk::"<<block_id<<"bk::::: "<<all[all.size()-1]<<" "<<ftell(fp)<<endl;
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
size_t blockScheduler::getfreeinodenum()
{
    return sb->getfreeinodenum();
}
void blockScheduler::ReWrinode(inode &ino,bool read)
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+sizeof(inode)*ino.i_num,SEEK_SET);
    cout<<"ino_inum"<<ino.i_num<<" "<<read<<" "<<endl;
    if(read)
    {
        auto it=inode_hash.find(ino.i_num);
        if(it!=inode_hash.end())
        {
            (*it).second.second++;
            ino=*((*it).second.first);
        }
        else
        {
            fread(&ino,sizeof(inode),1,fp);
            inode_hash.emplace(ino.i_num,pair(ino,1));
        }
    }
    else
    {
        auto it=inode_hash.find(ino.i_num);
        if(it!=inode_hash.end())
        {
            (*it).second.second++;
            ino=*((*it).second.first);
        }
        else
        {
            fread(&ino,sizeof(inode),1,fp);
            inode_hash.emplace(ino.i_num,pair(ino,1));
        }
    }
    fclose(fp);
    return;
}
void blockScheduler::writesuperblock()
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,0,SEEK_SET);
    fwrite(sb,sizeof(super_block),1,fp);
    fclose(fp);
}
void blockScheduler::freeinode(size_t ino)
{
    sb->freeinode(ino);
}
void blockScheduler::load(dirTree*dir_tree_)
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,0,SEEK_SET);
    fread(sb,sizeof(super_block),1,fp);
    auto root_inode=iget(true);
    fseek(fp,sizeof(super_block),SEEK_SET);
    fread(root_inode,sizeof(inode),1,fp);
    cout<<"freenum"<<getfreeblocknum()<<endl;
    //fclose(fp);

    ///////////////////////构建根节点
    // s_root=(dentry*)malloc(sizeof(dentry));
    // char name[]="root";

    // dentry temp_root;
    // temp_root.init(name,temp_inode,NULL,0);///////////需要更改
    // s_root=&temp_root;
    
    //dirtree->init_root("/", ROOT_INODE_NUMBER, root_inode);
    //dirtree->load_root(root_inode);     // 创建根节点
    sb->load(root_inode,dir_tree_);
}
string blockScheduler::getUSERroot(string username,string passwoard,int&uid,int&gid)
{
    d_user tempuser;
    if(username.size()!=1||passwoard.size()>MAXPASSWOREDLEN-3)
        return "";
    if(username[0]-'0'<=USERNUM&&username[0]-'0'>=1)
    {
        size_t int_user_name=username[0]-'0';
        FILE *fp=fopen("../disk.img","r+");
        fseek(fp,sizeof(super_block)+sizeof(inode)*INODENUM+512*DATANUM+(int_user_name-1)*sizeof(d_user),SEEK_SET);
        fread(&tempuser,sizeof(d_user),1,fp);
        string str_pss_getfrom_disk=tempuser.d_password;
        if(str_pss_getfrom_disk==passwoard)
        {
            uid=(int)tempuser.uid;
            gid=(int)tempuser.gid;
            string RETroot="user";
            RETroot+=to_string(uid);
            return RETroot;
        }
    }
    return "";
}
void blockScheduler::freshUSER()
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,sizeof(super_block)+sizeof(inode)*INODENUM+512*DATANUM,SEEK_SET);        
    for(size_t i=0;i<USERNUM;i++)
    {
        d_user tempuser;
        tempuser.uid= i+1;
        string PASSroot="100";
        PASSroot+=to_string(i+1);
        tempuser.gid=(i>>1)+1;
        strcpy(tempuser.d_password,PASSroot.c_str());
        fwrite(&tempuser,sizeof(d_user),1,fp);
        string RETroot="user";
        RETroot+=to_string(i+1);
        dirtree_->alloc_dir_init(RETroot, dirtree_->get_root(), nullptr, DIR,i+1);
    }
    fclose(fp);
    return;
}