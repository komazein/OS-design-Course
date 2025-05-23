#pragma once
#include "fs_types.h"

struct inode;        // 前向声明
class super_block;   // 前向声明

// 内存项缓存(缓存树形目录)
class dentry
{
private:
    char* d_name;                  // 当前目录项名
    inode* d_inode;                // 指向 inode
    dentry* d_parent;              // 父目录指针
    std::list<dentry*> d_child;    // 子目录列表
    DFALG d_flag;                  // 状态信息
    time_t d_time;                 // 修改时间
    uint16_t d_ref;                // 引用计数

    //super_block d_sb;           // 超级块(可选)

    //std::unordered_map<std::string, dir_entry*> entries; // (备用方式)
};
