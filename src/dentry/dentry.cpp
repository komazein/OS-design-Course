#include "dentry.h"
#include <spdlog/spdlog.h>


///////////////////////////////
///         dentry          ///
///////////////////////////////

void dentry::add_subdir(vector<dir_entry>& dir_entries)
{
    for(auto& dir_entry : dir_entries){
        dentry* child_dentry = new dentry(dir_entry.name, dir_entry.inode_num, this);
        d_child_[dir_entry.name] = child_dentry;

        // 日志显示
        spdlog::debug("Added subdir '{}' (inode={}) under '{}'", 
            dir_entry.name, dir_entry.inode_num, d_name_);
    }
}

void dentry::add_single_subdir(dentry* new_node)
{
    d_child_[new_node->d_name_] = new_node;     // 直接加入子节点中

    // 日志显示
    spdlog::debug("Add single subdir '{}' under '{}'", 
            new_node->d_name_, this->d_name_);
}

// 查找子节点
dentry* dentry::find_subdir(string& name)
{
    auto it = d_child_.find(name);
    if(it == nullptr) { return nullptr; }
    else { return it->second; }
}

bool dentry::has_subdir()
{
    return !d_child_.empty();
}

bool dentry::erase_subdir(string& name)
{
    size_t erasedCount = d_child_.erase(name);
    if(erasedCount > 0) {
        spdlog::info("Erased subdir '{}' from '{}'", name, d_name_);
        return true;
    } else {
        spdlog::warn("Failed to erase subdir '{}' from '{}': not found", name, d_name_);
        return false;
    }
}

// 添加引用计数
void dentry::add_ref()
{
    ++d_ref_;
}

// 减少引用计数
bool dentry::sub_ref()
{
    if(d_ref_ == 0)     return false;       // 减少引用计数失败

    --d_ref_;
    
    return true;
}

// 设置状态
void dentry::set_flag(DFALG flag)
{
    d_flag_ = flag;
}

// 设置时间
void dentry::set_time(time_t time)
{
    d_time_ = time;
}



///////////////////////////////
///         dcache          ///
///////////////////////////////

dentry* dcache::find_dentry(string& name, dentry* parent)
{
    auto it = dentry_table_.find({parent, name});
    if(it == dentry_table_.end())   { 
        spdlog::warn("Dentry '{}' miss in cache for parent '{}'", name, parent->get_name());    
        return nullptr; 
    }
    else {
        spdlog::info("Dentry '{}' hit in cache for parent '{}'", name, parent->get_name());
        return it->second; 
    }
}


bool dcache::add_dentry(string& name, dentry* dentry_node, dentry* parent)
{
    dentryKey dk = { parent, name };
    if(dentry_table_.count(dk) != 0) {
        spdlog::warn("Dentry '{}' already exists in cache for parent '{}'", name, parent->get_name());
        return false;
    }

    dentry_table_[dk] = dentry_node;
    spdlog::debug("Added dentry '{}' to cache for parent '{}'", name, parent->get_name());
    return true;
}



bool dcache::erase_dentry(const string& name, dentry* dentry_node)
{
    size_t erasedCount = dentry_table_.erase({dentry_node, name});
    if(erasedCount > 0) { 
        spdlog::info("Dentry '{}' already erase from cache for parent '{}'", name, dentry_node->get_name());
        return true; 
    }
    else { 
        spdlog::warn("Dentry '{}' didn't exist in the cache for parent '{}'", name, dentry_node->get_name());
        return false; 
    }
}



///////////////////////////////////
///         dirTree             ///
///////////////////////////////////

void dirTree::init_root(string root_name="/", size_t root_inode_num, inode* root_inode)
{
    root_ = new dentry(root_name, root_inode, root_inode_num, nullptr);     // root无父节点
    root_inode->i_type = DIR;

    auto cur_time = get_time();
    root_inode->i_atime = cur_time;
    root_inode->i_ctime = cur_time;
    root_inode->i_mtime = cur_time;

    spdlog::info("Initialize the file system root dentry node");

}


dentry* dirTree::hash_search(string& name, dentry* parent)
{
    return cache_->find_dentry(name, parent);
}


dentry* dirTree::name_travesal(string& path, dentry* work_dir = nullptr)
{

    dentry* search = root_;     // 此时默认为绝对路径
    if(work_dir != nullptr){
        search = work_dir;      // 否则是相对路径, 从此处开始查找
    }

    stringstream ss(path);      // 字符串分割

    string name;        // 存储分割的字符串
    while(getline(ss, name, '/')){

        // 还是优先哈希查找
        auto dentry_next = hash_search(name, search);

        if(dentry_next == nullptr){
            // 没找到则直接查找本dentry的child
            dentry_next = search->find_subdir(name);

            if(dentry_next == nullptr) {
                if(search->get_flag() == FIRST_LOAD_TO_MEMORY) {
                    // 如果还没找到, 就是确实没有
                    return nullptr;         // 查找失败
                } else {
                    // 说明此时为剪枝后才没有其子目录项, 需要重新进行I/O读取其子目录
                    // TODO_finish: 磁盘中访问 search 的 inode, 读取其中的目录项
                    spdlog::info("In order to ensure '{}' whether have '{}',loading it's children from disk.", search->get_name(), name);
                    
                    /// 此处进行I/O操作获得子目录项存入dir_entries
                    vector<dir_entry> dir_entries;              // 假定已经返回了目录项

                    bs->loadchild(dir_entries, *search->get_inode());

                    search->add_subdir(dir_entries);        // 此时已经加入了, 重新查找

                    dentry_next = search->find_subdir(name);            // 此时重新查找是否存在

                    if(dentry_next == nullptr) { return nullptr; }      // 此时是真的找不到了
                }
                
            }
            
            // 更新全局hash表
            cache_->add_dentry(name, dentry_next, search);
        }

        search = dentry_next;       // 继续寻找

        // 否则继续找子节点中是否有这个路径
    }

    spdlog::info("Traversal success: reached '{}'", search->get_name());

    return search;

}

bool dirTree::add_hash(string& name, dentry* parent, dentry* dentry_node)
{
    return cache_->add_dentry(name, dentry_node, parent);
}

bool dirTree::name_search_test(string& name, dentry* work_dir/*, bool update_hash*/)
{
    // 首先初步利用哈希查找是否存在该目录项(保证不能重名)
    if(hash_search(name, work_dir)) { return false; }

    // 再保证当前目录下确实无此目录
    auto sub_dir = work_dir->find_subdir(name);

    if(sub_dir != nullptr /*&& work_dir->get_flag() == FIRST_LOAD_TO_MEMORY*/)  { return false; }    
    
    // 最后保证磁盘中确实无此目录

    if(work_dir->get_flag() == CUT_SUBDIRS) {   

        spdlog::info("In order to ensure '{}' whether have '{}',loading it's children from disk.", work_dir->get_name(), name);
        /// TODO_finish:此处进行I/O操作获得子目录项存入dir_entries
        vector<dir_entry> dir_entries;              // 假定已经返回了目录项

        bs->loadchild(dir_entries, *work_dir->get_inode());

        work_dir->add_subdir(dir_entries);       // 这时加入了新的子节点

        auto new_add_dir = work_dir->find_subdir(name);
        if(new_add_dir != nullptr)  { return false; }       

    }

    return true;        // 没找到同名的节点
}

bool dirTree::has_child_test(dentry* dentry_node)
{
    // 还是一样流程, 只需要进行当前节点子检查和磁盘检查即可

    // 首先扫描此节点中的所有child(如果有的话)
    if(dentry_node->get_flag() == FIRST_LOAD_TO_MEMORY){
        // 保证此时的child全部加载进入内存中了
        if(!dentry_node->has_subdir()) { return true; } // 此时确实无子节点了
        else { return false; }      // 存在则有子节点
    }  
    else{       // 此时得去磁盘中查找看是否有子节点
        
        spdlog::info("In order to ensure '{}' whether have children node, loading it's children from disk.", dentry_node->get_name());
        /// TODO_finish:此处调用磁盘I/O控制器申请获得此目录的子目录项

        // 此时如果已经获得了子目录项
        vector<dir_entry> dir_entries;              // 假定已经返回了目录项

        bs->loadchild(dir_entries, *dentry_node->get_inode());

        dentry_node->add_subdir(dir_entries);       // 加入子

        // 此时检查是否加入了新的
        if(!dentry_node->has_subdir())  { return true; }
        else { return false; }      // 加入了新的
    }

}

bool dirTree::alloc_dir(string& name, dentry* work_dir)
{
    if(!name_search_test(name, work_dir)) { 
        spdlog::warn("Directory allocation failed: '{}' already exists in '{}'", name, work_dir->get_name());
        return false; 
    }  // 如果找到了同名的, 则分配新目录失败

    /////////////////// 下面是可以创建新的目录项了

    /// TODO_finish:通知I/O分配新的inode

    auto new_allocate_inode = bs->iget(false);

    /// TODO: 完善inode的信息
    auto cur_time = get_time();
    new_allocate_inode->i_type = DIR;
    new_allocate_inode->i_size = 1;
    new_allocate_inode->i_atime = cur_time;
    new_allocate_inode->i_ctime = cur_time;
    new_allocate_inode->i_mtime = cur_time;

    ///  .....

    dentry* new_node = new dentry(name, new_allocate_inode, new_allocate_inode->i_num, work_dir);
    work_dir->add_single_subdir(new_node);      // 为当前工作路径加入新的子目录
    // 还需要更新全局哈希
    cache_->add_dentry(name, new_node, work_dir);

    spdlog::info("Allocated new directory '{}' under '{}', inode={}", 
                 name, work_dir->get_name(), new_allocate_inode->i_num);
    return true;
}

void dirTree::del_tree(dentry* dentry_root)
{
    if(!dentry_root) { return; }        // 仅仅是保证安全性, 应该不会执行此语句
    if(!has_child_test(dentry_root)){
        // 此时已经到达叶子节点

        /// TODO: 1. 通知I/O回收此块

        spdlog::info("Letting blockScheduler to recycle the '{}' children's inode and thier blocks", dentry_root->get_name());
        // 2. 释放此节点子节点

        delete dentry_root;
        return;
    }

    // 此时说明还有子节点

    auto& sub_dirs = dentry_root->get_subdir();
    // 遍历并递归删除所有子节点
    for (auto& [name, child_node] : sub_dirs) {
        del_tree(child_node);
        cache_->erase_dentry(name, dentry_root);  // 在 child 被 delete 前移除哈希映射
    }
}

bool dirTree::free_dir(string& name, dentry* work_dir)
{
    if(name == "/") { 

        spdlog::info("Delete the root, clear the file system.");
        /// TODO_finish: 调用I/O的清空操作
        bs->new_disk();

        return true;
    }

    // 首先还是一样的查找验证过程
    if(name_search_test(name, work_dir))  { return false; }       // 如果没找到, 则删除失败

    del_tree(work_dir);     // 删除树

    // 此时仅仅释放了以work_dir为根的树, 但是work_dir此时没有被释放, 因为析构函数定义的释放规则是仅释放子节点和d_child_哈希表
    // 所以需要通过其父节点删除work_dir和在父节点中移除其表项
    dentry* parent_node = work_dir->get_parent();

    if(parent_node == nullptr)  { // 此时不应该发生, 如果发生则是错误
        spdlog::error("{} occur error!",  __func__);
        exit(1);
    }

    parent_node->erase_subdir(work_dir->get_name());        // 待删除节点的根节点移除此项
    cache_->erase_dentry(name, parent_node);                   // 清除全局哈希

    delete work_dir;                                        // 这时释放此节点即可

    spdlog::debug("Deleted subdir '{}' under '{}'", name, parent_node->get_name());

    return true;
}