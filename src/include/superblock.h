#pragma once
#include "fs_types.h"
// #include "dentry.h"
// class dentry; // 前向声明
class dirTree;

// 超级块结构
class super_block {
private:
    size_t s_block_size;                 // 块大小
    size_t s_block_num;                  // 块数量
    // dentry* s_root;                      // 根目录指针   统一使用dirTree作为目录的对外接口, 此时获取s_root即为dirtree->get_root()
    int s_free_num[S_FREE_NUM];          // 空闲数据块数量列表
    size_t s_inode_num;
    int stack_inode[INODENUM];

    // 其他模块的接口
    dirTree* dirtree;


public:
    super_block(dirTree* dirtree) : dirtree(dirtree) {}
    inode* iget(bool ifroot);    // 分配 inode
    void idel();         // 删除 inode
    void init();         // 初始化超级块
    bool getblock(int n,vector<size_t>&a);
    void releaseblock(int n,vector<size_t>&a);
    void newdisk();
    void load();
    void freeinode(size_t ino);
    size_t getfreeBlocknum();
};
