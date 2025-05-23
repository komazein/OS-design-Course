#include "fs.h"


class dentry
{
private:
    char* d_name;          // 文件名
    inode* d_inode;         // 指向目录项的 inode
    dentry* d_parent;         // 父(构建树形结构)
    std::list<dentry*> d_child;     // 子
    DFALG d_flag;             // 状态信息
    time_t d_time;     // 若干访问时间
    uint16_t d_ref;         // 引用计数
    //super_block d_sb;       // 指向所属的文件系统
    //// 可选用缓存替换策略(如lru)

    // 存储目录项可以使用哈希表存储, 提高查找效率(也可顺序表存储)
    std::unordered_map<std::string, dir_entry*> entries; // 存储目录项

};