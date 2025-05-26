#include"fs_types.h"
class blockScheduler
{
public:
    void loadchild(vector<dir_entry>&a,inode id);
    void ftree(size_t block_id,size_t n,vector<dir_entry>&a);

    void writechild(dir_entry par,vector<dir_entry>&a,inode id);
    void fwtree(dir_entry par,vector<dir_entry>&a,inode id);
    void writeBlock(int num);
    
    inode* iget(bool ifroot);               // 分配inode节点, 如果if_root为true则为根节点

    void new_disk();
    
    size_t cal_block_num_dir(size_t n_dir);//only child
    
    size_t fastpow(size_t di,int x);

private:
    super_block* sb;
};