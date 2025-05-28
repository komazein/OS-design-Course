#include"fs_types.h"
#include"superblock.h"
// #include"dentry.h"
class blockScheduler
{
public:
    blockScheduler(dirTree* dirtree)
    {
        dirtree_ = dirtree;
        sb = new super_block(dirtree_);
    }
    void loadchild(vector<dir_entry>&a,inode &id);
    //从磁盘中读取目录项
    void ftree(size_t block_id,size_t n,vector<dir_entry>&a);
    //loadchild的树，递归使用

    void writechild(dir_entry par,vector<dir_entry>&a,inode &id,size_t num);
    //向磁盘中写入目录项
    void fwtree(size_t block_id,size_t n,vector<dir_entry>&a);
    //writechild使用的树


    void changeDirentryToblockID(size_t now_block_id,size_t n,size_t block_id);//n with pwrent
    void writeBlockIDdir(size_t n,inode &id);// n noly child, old num
    //

    size_t getlastblockID(size_t n,inode &id);//n with pwrent,old num,用之前确认是否需要释放块
    size_t treeFindLastBlock(size_t now_block_id,size_t n);
    
    inode* iget(bool ifroot);               // 分配inode节点, 如果if_root为true则为根节点

    void new_disk();
    
    size_t cal_block_num_dir(size_t n_dir);//only child
    bool creatFILE(size_t old_num_only_child,inode &parid,inode &chid);
    size_t fastpow(size_t di,int x);
    void freeBlock_inode(inode &di,size_t n_child,inode &par,size_t n_parent);//

    void getallBlockDIR(inode &id,size_t n,vector<size_t>&a);//n for DIR:only child

    void getblockTree(size_t block_id,size_t n,vector<size_t>&a,vector<size_t>&forSIMfile,TYPE type);//获取所有目录项块号

    void getallBlockSIM(inode &id,vector<size_t>&a,vector<size_t>&forSIMfile);//SIM_FILE获取所有磁盘块号
    
    size_t calSIMblockNUM(size_t blockNUM);//根据无索引块的块数获取有索引时的磁盘块数

    void writeBlocknumFORsim(vector<size_t>&all,size_t n,inode&id,char*a); //对每个索引进行获取当前个数，然后交由simwriteTree进行递归查找

    void simwriteTree(size_t block_id,vector<size_t>&all,size_t n,vector<size_t>&newlist); //写文件时用的树

    char*readSIMfromBLOCK(inode&id);//读文件的外部接口

    bool writeSIMfromBLOCK(inode&id,char*a);//写文件的外部接口，完成重新分配磁盘块号后交由writeBlocknumFORsim实现

    void SIMwriteBK(vector<size_t>newlist,size_t n,char*a);//根据读取的磁盘块号获取字符串
    void freeblock(vector<pair<inode*,size_t>>&del_nodes,inode&par,size_t primsizeofchild);
    size_t getfreeblocknum();
    void ReWrinode(inode &ino,bool read);
    void writesuperblock();

private:
    super_block* sb;
    dirTree* dirtree_;
};

