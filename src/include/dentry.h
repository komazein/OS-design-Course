#pragma once
#include "fs_types.h"

struct inode;        // 前向声明
class super_block;   // 前向声明

/**
 * 
 * 内存项缓存(缓存树形目录)
 * 
 */ 
class dentry
{
public:
    dentry(string d_name, inode* d_inode, size_t d_inode_num, dentry* d_parent) : 
        d_name_(d_name), d_inode_(d_inode), d_inode_num_(d_inode_num), d_parent_(d_parent) 
    { }

    // 父节点初始化子节点时实例化子节点
    // param: 此时了解到的子节点信息只有名称和子节点inode号
    dentry(string d_name, size_t d_inode_num, dentry* d_parent) : 
        d_name_(d_name), d_inode_num_(d_inode_num), d_parent_(d_parent)   
    { } 

    // 链接子节点
    // param: 通过读取目录的inode的数据块获取的目录项构建子节点
    void add_subdir(vector<dir_entry>& dir_entries);

    // 查找子节点
    dentry* find_subdir(string& name);

    // 新增引用计数
    void add_ref();

    // 减少引用计数 删除相关文件时调用
    bool sub_ref();

    // 设置状态
    void set_flag(DFALG flag);

    // 设置时间
    void set_time(time_t time);

    // 获取状态
    DFALG get_flag()    { return d_flag_; }

    // 获取时间
    time_t get_time() { return d_time_; }



private:
    
    string d_name_;                  // 当前目录项名

    inode* d_inode_;                // 指向 inode
    
    size_t d_inode_num_;            // 当前节点的inode编号
    
    dentry* d_parent_;              // 父目录指针
    
    // std::list<dentry*> d_child;    // 子目录列表
    std::unordered_map<std::string, dentry*> d_child_; // 子目录(文件)字典
    
    DFALG d_flag_;                  // 状态信息
    
    time_t d_time_;                 // 修改时间         (这个可以结合替换策略)
    
    uint16_t d_ref_;                // 引用计数
};


/**
 * 提供目录树过于庞大导致大量占用内存问题, 提供剪枝
 * 也针对dchache防止内存占用过大
 */

class dentryReplacer
{


public:
    list<dentry*> lru_lists;                // 替换策略的链表
};


/**
 * 缓存dentry目录项的集合(加速路径查找)
 * 
 * 路径名 <---> inode
 * 
 * 真正实现**按名**查找
 * 
 * 注意: 这里的路径名不是存储整个完整路径, 而是存储(父目录dentry, 文件(目录名)的name)对 
 */


/**
 * 全局目录项缓存键值对
 */
struct dentryKey{
    dentry* parent;
    string name;

    bool operator==(const dentryKey dk) const {
        return parent == dk.parent && name == dk.name;
    }
};

// 由于使用自定义的键, 所以得自定义一个hash函数
struct dentryKeyHash{
    size_t operator()(const dentryKey& key) const {
        return hash<dentry*>()(key.parent) ^ hash<string>()(key.name);
    }
};

class dcache{
public:
    
    friend dentryReplacer;

    /**
     * @brief 快速查找定位
     * 
     * @return 如果没找到则返回nullptr(此时只能沿路径查找), 否则找到直接返回dentry节点 
     */
    dentry* find_dentry(string& name, dentry* parent);
    
    /**
     * @brief 增加表项
     * 
     * @return true if add success, false if not add to the hash table(has exists)
     */
    bool add_dentry(string& name, dentry* dentry_node, dentry* parent);


private:

    dentry* root_;

    // 维护全局 路径名<--->文件树节点的映射关系, 加速查找
    unordered_map<dentryKey, dentry*, dentryKeyHash> dentry_table_;     

    dentryReplacer replacer_;               // 替换策略

    size_t dchache_max_size_;               // 限制的最大的缓存块大小
};


////////////////////////////////////////////////
// 目录树                                   ////
// 进行查找, 替换等操作, 作为整个目录的对外接口////
///////////////////////////////////////////////
class dirTree
{
public:

    friend dentryReplacer;      // 随意访问dirTree的内容

    friend dcache;              

    /**
     * @brief 全局哈希表快速查找(未找到再按名查找)
     * 
     * @param 此时name仅为目录的名称, 不为路径, 还需要提供父节点以供快速查找
     * 
     * @return nullptr if not search
     * 
     */
    dentry* hash_search(string& name, dentry* parent); 


    /**
     * @brief 按路径查找, (还要考虑当前的工作路径问题)
     * 
     * @param path 传入的可以是相对路径(针对于当前工作路径),
     * 也可以是绝对路径, 此时parent即默认的nullptr
     * 
     * 比如 test/src                (相对路径)
     * 也可以是: /usr/kaku/os_keshe (绝对路径)
     * 
     * @param work_dir 传入当前的工作路径, 默认为根目录开始查找
     * 
     * @return nullptr 没找到
     */
    dentry* name_travesal(string& path, dentry* work_dir);

    // 加入hash
    bool add_hash(string& name, dentry* parent, dentry* dentry_node);

    /**
     * 
     * @brief 现在(假设) 只有在当前工作目录下创建新的目录
     * 
     * @param name 传入要创建的目录名称
     * 
     * @param workdir 当前的工作路径
     * 
     * @return false 如果已存在该目录, 否则正常创建
     * 可能还需要考虑如果空闲磁盘块不足而创建失败
     */
    bool alloc_dir(string& name, dentry* work_dir);
    
    // 释放目录项
    bool free_dir();

private:

    dentry* root_;                      // 目录树根节点
    
    dentryReplacer repalcer_;           // 替换策略

    dcache cache_;                      // 目录项缓存

    // 与磁盘I/O调度器的接口, 此调度器可以提供给功能如下:
    // - 可以根据inode查找其子目录项并返回vector<dir_entry> dir_entries
    // 如
    // blockScheduler bs;
};