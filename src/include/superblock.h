#pragma once
#include "fs_types.h"

class dentry; // 前向声明

// 超级块结构
class super_block {
private:
    size_t s_block_size;                 // 块大小
    size_t s_block_num;                  // 块数量
    dentry* s_root;                      // 根目录指针
    int s_free_num[S_FREE_NUM];          // 空闲数据块数量列表
    size_t s_inode_num;
    int stack_inode[INODENUM];


public:
    inode*iget(bool ifmain,TYPE type,FILEMODE i_mode,uint8_t i_uid,uint8_t i_gid,size_t i_size,uint32_t i_flag,uint16_t di_link_count);// 分配 inode
    void idel();         // 删除 inode
    void init();         // 初始化超级块
    bool getblock(int n,int a[]);
    void releaseblock(int n,int a[]);
    void newdisk();
};
