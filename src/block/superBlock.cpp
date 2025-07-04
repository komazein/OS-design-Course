#include <fs.h>
#include<iostream>
void super_block::init(){}
void super_block::load(inode*root_inode,dirTree*dir_tree_)
{
    // FILE *fp=fopen("../disk.img","r+");

    // fseek(fp,0,SEEK_SET);
    // fread(this,sizeof(super_block),1,fp);
    // auto root_inode=iget(true);
    // fseek(fp,sizeof(super_block),SEEK_SET);
    // fread(root_inode,sizeof(inode),1,fp);
    
    //fclose(fp);

    ///////////////////////构建根节点
    // s_root=(dentry*)malloc(sizeof(dentry));
    // char name[]="root";

    // dentry temp_root;
    // temp_root.init(name,temp_inode,NULL,0);///////////需要更改
    // s_root=&temp_root;
    
    //dirtree->init_root("/", ROOT_INODE_NUMBER, root_inode);
    dirtree=dir_tree_;
    dirtree->load_root(root_inode);     // 创建根节点

}

void super_block::newdisk()
{
    int LEFT=DATANUM-1;//第0块给根节点
    s_block_num=LEFT;//初始化剩余块号
    s_inode_num=INODENUM-1;//初始化剩余inode,第0个分配根节点
    for(int i=0;i<s_inode_num;i++)
        stack_inode[i]=i+1;
    int base=0;
    int IN=1;
    int firstempty=1;
    while(LEFT)
    {
        s_free_num[base]=min(LEFT,S_FREE_NUM-1);
        base++;
        if(firstempty)
        {
            s_free_num[base]=0;
            base++;
        }
        for(int i=firstempty+1;i<=min(LEFT,S_FREE_NUM-1);i++)
        {
            s_free_num[base]=IN;
            base++;
            IN++;
        }
        LEFT-=min(S_FREE_NUM-1-firstempty,LEFT);
        firstempty=0;
        if(LEFT)
        {
            
            // cout<<"PUT IN THE "<<IN<<"-TH BLOCK"<<endl;
            // for(int i=0;i<base;i++)
            //     cout<<"("<<i<<","<<s_free_num[i]<<")";
            // cout<<endl;
            
            base=0;
            
            FILE *fp=fopen("../disk.img","r+");
            fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+IN*512,SEEK_SET);//写回第IN快磁盘
            fwrite(s_free_num, sizeof(int),S_FREE_NUM,fp);
            fclose(fp);
            
        }
        // else
        // {
        //     for(int i=0;i<base;i++)
        //         cout<<"("<<i<<","<<s_free_num[i]<<")";
        //     cout<<endl;
        // }
    }
    // struct inode*root_inode=(struct inode*)malloc(sizeof(struct inode));
    auto root_inode=iget(true);
    // 构建根节点//////////////////////////////////////
    dirtree->init_root("/", ROOT_INODE_NUMBER, root_inode);     // 创建根节点
    // dentry temp_root;
    // temp_root.init(name,temp_inode,NULL,0);//////////缺参数
    // s_root=&temp_root;
    
}
bool super_block::getblock(int n,vector<size_t>&a)
{
    if(n>s_block_num)
        return 0;
    s_block_num-=n;
    for(int i=0;i<n;i++)
    {
        a.push_back((size_t)s_free_num[s_free_num[0]]);
        //cout<<"("<<s_free_num[0]<<","<<a[i]<<")";
        if(s_free_num[0]==1)
        {
            FILE *fp=fopen("../disk.img","r+");
            fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+s_free_num[1]*512,SEEK_SET);
            fread(s_free_num, sizeof(int),S_FREE_NUM,fp);
            fclose(fp);
            //从磁盘中加载第s_free_num[1]块
        }
        else
            s_free_num[0]--;
    }
    return 1;
}//需根据更改的具体参数，以及具体情况进行更改
void super_block::releaseblock(int n,vector<size_t>&a)
{
    //cout<<"free:"<<n<<endl;
    for(int i=0;i<n;i++)
    {
        if(s_free_num[0]==S_FREE_NUM-1)
        {
            FILE *fp=fopen("../disk.img","r+");
            fseek(fp,sizeof(super_block)+INODENUM*sizeof(inode)+a[a.size()-1]*512,SEEK_SET);

            fwrite(s_free_num, sizeof(int),S_FREE_NUM,fp);
            fclose(fp);
            /*
            for(int i=0;i<=s_free_num[0];i++)
                cout<<"("<<i<<","<<s_free_num[i]<<")";
            cout<<endl;
            */
            s_free_num[0]=0;
        }
        s_free_num[0]++;
        s_free_num[s_free_num[0]]=a[a.size()-1];
        a.erase(a.end()-1);
    }
    s_block_num+=n;
}//需根据更改的具体参数，以及具体情况进行更改
inode* super_block::iget(bool ifroot)
{
    inode*ino=(inode*)malloc(sizeof(inode));
    ino->di_link_count=0;
    if(ifroot){
        ino->i_num=0;
        return ino;
    }
    if(s_inode_num==0)
        return NULL;
    ino->i_num=stack_inode[s_inode_num-1];
    s_inode_num--;
    return ino;
}
size_t super_block::getfreeBlocknum()
{
    return s_block_num;
}
void super_block::freeinode(size_t ino)
{
    stack_inode[s_inode_num]=ino;
    s_inode_num++;
}
size_t super_block::getfreeinodenum()
{
    return s_inode_num;
}