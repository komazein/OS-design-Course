#pragma once
#include "fs_types.h"
#include <spdlog/logger.h>

struct inode;        // 前向声明
class super_block;   // 前向声明
class blockScheduler;
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
    { 
                        
    }

    // 父节点初始化子节点时实例化子节点
    // param: 此时了解到的子节点信息只有名称和子节点inode号
    dentry(string d_name, size_t d_inode_num, dentry* d_parent) : 
        d_name_(d_name), d_inode_num_(d_inode_num), d_parent_(d_parent)   
    { } 

    ~dentry()     // 根节点管理子节点的释放
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

    // 有子节点(仅仅是确定FIRST_LOAD_TO_MEMORY情况下确实无字, 但是不保证其他情况)
    bool has_subdir();

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

    // 获取所有子节点
    std::unordered_map<std::string, dentry*>& get_subdir() { return d_child_; }

    // 获取父节点, 如果为"/"则返回nullptr
    dentry* get_parent() { return d_parent_; }

    // 获取名称
    string& get_name() { return d_name_; }

    // 获取inode号
    int get_inode_num() { return d_inode_num_; }

    // 获取inode
    inode* get_inode()  { return d_inode_; }



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

    explicit LRUReplacer(size_t max_size = 1024) : max_size_(max_size) {}


    /**
     * 
     * @brief 将节点插入到`lru_list`的首部,
     * 如果存在于链表中, 则将其移动到链表的首部
     * 
     * @param dentry_node 待插入lru链表中的节点
     * 
     */
    void Insert(dentry* dentry_node);


    /**
     * 
     * @brief 将当前节点以及其父辈的节点插入到`lru_list`的首部,
     * 适用于剪枝操作时, 由于一个访问时间较近的节点不能被替换出去, 也就是说明其父辈节点也不能换出
     * 所以将其沿着父辈方向的节点依次插入`lru_list`
     * 
     * 
     */
    void InsertDir(dentry* dentry_node);

    /**
     * 
     * @brief 优先替换出`lrulist`中的尾部元素
     * 
     * @return `false` 没有可以替换出的节点, 说明此事lru_list为空
     * 
     */
    bool Victim();

    /**
     * 
     * @brief 就是指定需要移除的节点, 即无理由替换
     * 
     * @param dentry_node 强制替换出的节点
     * 
     * @return `ture` 如果在表中并替换成功
     * 
     */
    bool Erase(dentry* dentry_node);


    /**
     * 
     * @brief 可以动态的更新最大链表的限制
     * 
     */
    void set_max_size(size_t max_size) { max_size_ = max_size; }

private:
    
    // 由于本文件系统只考虑到了单进程, 所以不存在引用计数的问题, 
    // 所以只需要一个链表记录缓存节点即可
    // 其替换的优先级就是LRU
    list<dentry*> lru_list;     // lru链表 
    
    size_t max_size_;           // 最大的链表大小
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
    bool erase_dentry(const string& name, dentry* dentry_node);


private:

    dentry* root_;

    // 维护全局 路径名<--->文件树节点的映射关系, 加速查找
    unordered_map<dentryKey, dentry*, dentryKeyHash> dentry_table_;     

    LRUReplacer* replacer_;               // 替换策略

    size_t dchache_max_size_;               // 限制的最大的缓存块大小
};


////////////////////////////////////////////////
// 目录树                                   ////
// 进行查找, 替换等操作, 作为整个目录的对外接口////
///////////////////////////////////////////////
class dirTree
{
public:

    dirTree(LRUReplacer* replacer, dcache* cache/*, blockScheduler*bs*/)
        : replacer_(replacer), cache_(cache)
    {
    }

    void set_bs(blockScheduler* bs) { this->bs = bs; }
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
     * @brief 删除根节点root_, 必须要先调用free_dir是传入的是"/"才能调用,
     * 因为仅仅是简单的释放根的空间, 直接调用可能会引发内存泄漏
     * 
     */
    void del_root() { delete root_; }

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
     * @brief 在进行删除目录的时候, 需要级联删除, 
     * 即如果rmdir dir后, 需要删除以dir为根的目录, 同时也要删除磁盘中的相关所有目录项,
     * 此操作可能设计I/O操作, 所以不能使用dentry来实现功能
     * 
     * 
     * @param dentry_node 待检查的节点
     * 
     * @return `true` 如果没有释放完所有的子, 即还有子节点项  
     * 
     */
    bool has_child_test(dentry* dentry_node);

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
    bool alloc_dir(string& name, dentry* work_dir,inode* new_allocate_inode);
    

    /**
     * 
     * @brief 递归删除以dentry_root为根的树
     * 
     * @param dentry_node 待删除的树根
     * 
     * 
     */
    void del_tree(dentry* dentry_root);


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


    time_t get_time() { return cur_time; }

    /**
     * 
     * @brief 释放指定的dentry空间, 并不是删除, 而是释放此dentry占用的空间
     * 
     * 此函数也是递归删除以dentry_node为根节点的子树
     * 
     * @return `true`释放成功
     */
    bool cut_dir(dentry* dentry_node);

    /**
     * 
     * @brief 如果达到了某个阈值(或者通过手动指定释放空间), 则触发释放一些dcache空间
     * 此操作不会改变目录树的结构, 仅仅会减少dcache的表项, 减少的空间主要为目录的名称
     * 
     * @return 成功释放的存储的目录项(dentry)的个数, 如果为0则说明释放失败
     * 
     */
    size_t shrink_dcache();


    /**
     * 
     * @brief 如果达到了某个阈值(或者通过手动指定释放空间), 则触发释放一些目录树空间,
     * 会修剪掉一些不常用的目录项为根的子树, 为了方便管理, 同时也会释放相应的dcache
     * 此操作会改变原先的树形结构, 会根本的释放dentry节点, 释放的原则就是最不常用的目录项
     * 并且要注意, 如果一个节点处于较为靠近头的位置, 其父辈节点也处于高的优先级, 同样不能置换
     * 
     * @return 成功释放的存储的目录项(dentry)的个数, 如果为0则说明释放失败
     * 
     */
    size_t cut_dirTree();

private:

    dentry* root_;                      // 目录树根节点
    
    LRUReplacer* dcache_replacer_;             // 替换策略(dache释放)

    LRUReplacer* dentry_replacer_;             // 替换策略(树剪枝)

    dcache* cache_;                     // 目录项缓存

    blockScheduler* bs;                 // 与磁盘I/O调度器的接口

    time_t cur_time;                    // 当前时间, 需要外部的全局计数器模块获取时间
};  





