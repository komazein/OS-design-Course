#include"fs_types.h"
class blockScheduler
{
public:
    void loadchild(vector<dir_entry>&a,inode &id);
    void ftree(size_t block_id,size_t n,vector<dir_entry>&a);

    void writechild(dir_entry par,vector<dir_entry>&a,inode &id,size_t num);
    void fwtree(size_t block_id,size_t n,vector<dir_entry>&a);

    void changeDirentryToblockID(size_t now_block_id,size_t n,size_t block_id);//n with pwrent
    void writeBlockIDdir(size_t n,inode &id);// n noly child, old num
    
    size_t getlastblockID(size_t now_block_id,size_t n,inode &id);//n with pwrent,old num,用之前确认是否需要释放块
    size_t treeFindLastBlock(size_t now_block_id,size_t n);
    
    size_t cal_block_num_dir(size_t n_dir);//only child
    
    size_t fastpow(size_t di,int x);

    void freeDirentry(inode &di,size_t n);
    void getallBlockDIR(inode &id,size_t n,vector<size_t>&a);//n for DIR:only child
    void getblockTree(size_t block_id,size_t n,vector<size_t>&a,vector<size_t>&forSIMfile,TYPE type);
    void getallBlockSIM(inode &id,vector<size_t>&a,vector<size_t>&forSIMfile);
    char*readSIMfromBLOCK(inode&id);
    bool writeSIMfromBLOCK(inode&id,char*a);
    size_t calSIMblockNUM(size_t blockNUM);
    void writeBlocknumFORsim(vector<size_t>&all,size_t n,inode&id); 

    void simwriteTree(size_t block_id,vector<size_t>&all,size_t n,vector<size_t>newlist); 
private:
    super_block* sb;
};