#include <fs.h>
#include<iostream>
void super_block::init()
{
    FILE *fp=fopen("../disk.img","r+");
    fseek(fp,0,SEEK_SET);
    fread(this,sizeof(super_block),1,fp);

    inode* root_inode = (inode*)malloc(sizeof(inode));
    fseek(fp,sizeof(super_block),SEEK_SET);
    fread(root_inode,sizeof(inode),1,fp);
    fclose(fp);

    ///////////////////////构建根节点
    // s_root=(dentry*)malloc(sizeof(dentry));
    // char name[]="root";

    // dentry temp_root;
    // temp_root.init(name,temp_inode,NULL,0);///////////需要更改
    // s_root=&temp_root;

    dirtree->init_root("/", ROOT_INODE_NUMBER, root_inode);     // 创建根节点

}

void super_block::newdisk()
{
    int LEFT=BLOCKSIZE-1;//第0块给根节点
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
            /*
            cout<<"PUT IN THE "<<IN<<"-TH BLOCK"<<endl;
            for(int i=0;i<base;i++)
                cout<<"("<<i<<","<<s_free_num[i]<<")";
            cout<<endl;
            */
            base=0;
            FILE *fp=fopen("../disk.img","r+");
            fseek(fp,sizeof(super_block)+INODENUM*sizeof(dinode)+IN*512,SEEK_SET);//写回第IN快磁盘
            fwrite(s_free_num, sizeof(int),S_FREE_NUM,fp);
            fclose(fp);
        }
        /*else
        {
            for(int i=0;i<base;i++)
                cout<<"("<<i<<","<<s_free_num[i]<<")";
            cout<<endl;
        }*/
    }
    struct inode*root_inode=(struct inode*)malloc(sizeof(struct inode));
    char name[]="root";
    root_inode=iget(1,DIR,ALL,0,00,0,0,0);

    // 构建根节点//////////////////////////////////////


    dirtree->init_root("/", ROOT_INODE_NUMBER, root_inode);     // 创建根节点
    // dentry temp_root;
    // temp_root.init(name,temp_inode,NULL,0);//////////缺参数
    // s_root=&temp_root;
}
bool super_block::getblock(int n,int a[])
{
    if(n>s_block_num)
        return 0;
    for(int i=0;i<n;i++)
    {
        a[i]=s_free_num[s_free_num[0]];
        //cout<<"("<<s_free_num[0]<<","<<a[i]<<")";
        if(s_free_num[0]==1)
        {
            FILE *fp=fopen("../disk.img","r+");
            fseek(fp,sizeof(super_block)+INODENUM*sizeof(dinode)+s_free_num[1]*512,SEEK_SET);
            fread(s_free_num, sizeof(int),S_FREE_NUM,fp);
            fclose(fp);
            //从磁盘中加载第s_free_num[1]块
        }
        else
            s_free_num[0]--;
    }
    return 1;
}//需根据更改的具体参数，以及具体情况进行更改
void super_block::releaseblock(int n,int a[])
{
    for(int i=0;i<n;i++)
    {
        if(s_free_num[0]==S_FREE_NUM-1)
        {
            FILE *fp=fopen("../disk.img","r+");
            fseek(fp,sizeof(super_block)+INODENUM*sizeof(dinode)+a[i]*512,SEEK_SET);

            fwrite(s_free_num, sizeof(int),S_FREE_NUM,fp);
            fclose(fp);
            /*
            for(int i=0;i<=s_free_num[0];i++)
                cout<<"("<<i<<","<<s_free_num[i]<<")";
            cout<<endl;
            s_free_num[0]=0;
            */
        }
        s_free_num[0]++;
        s_free_num[s_free_num[0]]=a[i];
    }
}//需根据更改的具体参数，以及具体情况进行更改
inode* super_block::iget(bool ifmain,TYPE type,FILEMODE i_mode,uint8_t i_uid,uint8_t i_gid,size_t i_size,uint32_t i_flag,uint16_t di_link_count)
{
    if(s_inode_num==0)
        return NULL;
    inode*ino=(inode*)malloc(sizeof(inode));
    if(ifmain)
    {
        ino->i_num=0;
        type=DIR;
        i_mode=ALL;
        i_uid=0;
    }
    else
    {
        ino->i_num=s_inode_num-1;
        s_inode_num--;
    }
    ino->i_type=type;
    ino->i_gid=i_gid;
    ino->i_uid=i_uid;
    ino->i_flag=i_flag;
    ino->di_link_count=di_link_count;
    if(ino->i_type==DIR)
        ino->i_size==0;
    else
        ino->i_size=i_size;
    ino->i_ctime=ino->i_mtime=time(&(ino->i_atime));
    return ino;
    //没写完，该函数定义模糊
}
