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

    ~dentry()
    {
        for (auto& pair : d_child_) {
            delete pair.second;
        }
        d_child_.clear(); 
    }


    /**
     * 
     * @brief 根据目录项添加子节点, 需要子节点的inode, 
     * 子节点的inode可以当遍历到子节点时访问磁盘获得自己的inode再自行设置
     * 
     * @param `dir_entries` 通过读取目录的inode的数据块获取的目录项构建子节点,
     * 只是目录项, 虽然内容不足以完全确定子节点,
     * 但是可以在后续(按需)访问磁盘时获取其全部信息
     * 
     */
    void add_subdir(vector<dir_entry>& dir_entries);

    // 新增目录项(如mkdir)
    void add_single_subdir(dentry* new_node);

    // 查找子节点
    dentry* find_subdir(string& name);

    // 删除子节点
    bool erase_subdir(string& name);

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
    
    std::unordered_map<std::string, dentry*> d_child_; // 子目录(文件)字典
    
    DFALG d_flag_;                  // 状态信息
    
    time_t d_time_;                 // 修改时间         (这个可以结合替换策略)
    
    uint16_t d_ref_;                // 引用计数
};


/**
 * 提供目录树过于庞大导致大量占用内存问题, 提供剪枝
 * 也针对dcache防止内存占用过大
 * (应该)可以实现dache和dentry树的替换策略
 */

class LRUReplacer
{
public:

    void Insert(const dentry* dentry_node);

    bool Victim(dentry* dentry_node);

    bool Erase(const dentry* dentry_node);

private:
    
    list<dentry*> active_lists;     // 活跃链表, 经常访问的节点
    list<dentry*> inactive_lists;   // 不活跃链表     
    
    size_t 
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

    /**
     * @brief 快速查找定位
     * 
     * @return 如果没找到则返回nullptr(此时只能沿路径查找), 否则找到直接返回dentry节点 
     */
    dentry* find_dentry(string& name, dentry* parent);
    
    /**
     * 
     * @brief 增加表项
     * 
     * @return true if add success, false if not add to the hash table(has exists)
     */
    bool add_dentry(string& name, dentry* dentry_node, dentry* parent);

    /**
     * 
     * @brief 删除表项
     * 
     * 
     */
    bool erase_dentry(string& name, dentry* dentry_node);


private:

    dentry* root_;

    // 维护全局 路径名<--->文件树节点的映射关系, 加速查找
    unordered_map<dentryKey, dentry*, dentryKeyHash> dentry_table_;     

    LRUReplacer<>* replacer_;               // 替换策略

    size_t dchache_max_size_;               // 限制的最大的缓存块大小
};


////////////////////////////////////////////////
// 目录树                                   ////
// 进行查找, 替换等操作, 作为整个目录的对外接口////
///////////////////////////////////////////////
class dirTree
{
public:

    /**
     * 
     * @brief 初始化目录树的根节点
     * 
     * @param root_name 根名称, 默认为 "/"
     * 
     * @param root_inode, root_inode_num 分配给根的inode节点 以及其inode号
     * 
     * 
     */
    void init_root(string root_name, size_t root_inode_num, inode* root_inode);

    /**
     * 
     * @brief 获取根节点
     * 
     */
    dentry* get_root()  { return root_; }

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
     * 也可以是绝对路径, 此时`parent`即默认的`nullptr`,
     * 比如 `test/src`                (相对路径)
     * 也可以是: `/usr/kaku/os_keshe` (绝对路径)
     * 
     * @param work_dir 传入当前的工作路径, 默认为根目录开始查找
     * 
     * @return `nullptr` 没找到
     */
    dentry* name_travesal(string& path, dentry* work_dir = nullptr);

    // 加入hash
    bool add_hash(string& name, dentry* parent, dentry* dentry_node);


    /**
     * 
     * @brief 辅助函数, 作用是按名查找验证是否存在于文件系统中
     * 
     * 进行的过程如下:
     * 
     * 1. 首先查找全局哈希
     * 
     * 2. 失败后查找本目录下是否存在
     * 
     * 3. 最后调用I/O查看磁盘中是否存在此目录项
     * 
     * 
     * @param name 传入要创建的目录名称
     * 
     * @param workdir 当前的工作路径
     * 
     * @param update_hash 是否需要将找到的结果加入到全局哈希表中 
     * 如果仅仅是顺便查找周边的节点就不需要加入到全局hash, 但是如果是mkdir等访问操作就需要添加
     * 
     * 
     * @return `true` 如果没找到了重名的
     * 
     *  `false` 如果找到重名的 
     * 
     */
    bool name_search_test(string& name, dentry* work_dir/*, bool update_hash*/);



    /**
     * 
     * @brief 现在(假设) 只有在当前工作目录下创建新的目录, 如: `mkdir`
     * 
     * @param name 传入要创建的目录名称
     * 
     * @param workdir 当前的工作路径
     * 
     * @return `false` 如果已存在该目录, 否则正常创建
     * 可能还需要考虑如果空闲磁盘块不足而创建失败
     */
    bool alloc_dir(string& name, dentry* work_dir);
    
    /**
     * 
     * @brief 删除目录项, 如: `rmdir`
     * 
     * @param name 传入要创建的目录名称
     * 
     * @param workdir 当前的工作路径
     * 
     * @return `true` 删除成功, `false` 如果不存在此目录项
     * 
     */
    bool free_dir(string& name, dentry* work_dir);

private:

    dentry* root_;                      // 目录树根节点
    
    LRUReplacer* replacer_;           // 替换策略

    dcache* cache_;                      // 目录项缓存

    // 与磁盘I/O调度器的接口, 此调度器可以提供给功能如下:
    // - 可以根据inode查找其子目录项并返回vector<dir_entry> dir_entries
    // 如
    // blockScheduler bs;  
};





