#include "fs.h";
#include "dentry.h"
const int SUPERBLOCKSIZE = sizeof(class super_block);

class super_block{
private:
    size_t s_block_size;         //块大小
    size_t s_block_num;                   // 块数
    dentry* s_root;             // 指向根节点inode
    int s_free_num[S_FREE_NUM]; // 空闲Block manange
    // size_t s_free_num;          // 空闲块数量
    /// 成组连接法相关数据结构

    
public:
    inode*    iget();             // 分配
    void      idel();             // 销毁 
    void      init();             // 初始化

};