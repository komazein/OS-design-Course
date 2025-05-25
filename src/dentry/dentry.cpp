#include "dentry.h"

///////////////////////////////
///         dentry          ///
///////////////////////////////

void dentry::add_subdir(vector<dir_entry>& dir_entries)
{
    for(auto& dir_entry : dir_entries){
        dentry* child_dentry = new dentry(dir_entry.name, dir_entry.inode_num, this);
        d_child_[dir_entry.name] = child_dentry;
    }
}

void dentry::add_single_subdir(dentry* new_node)
{
    d_child_[new_node->d_name_] = new_node;     // 直接加入子节点中
}

// 查找子节点
dentry* dentry::find_subdir(string& name)
{
    auto it = d_child_.find(name);
    if(it == nullptr) { return nullptr; }
    else { return it->second; }
}

bool dentry::erase_subdir(string& name)
{
    size_t erasedCount = d_child_.erase(name);
    if(erasedCount > 0) { return true; }
    else { return false; }
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
    if(it == dentry_table_.end())   { return nullptr; }
    else { return it->second; }
}


bool dcache::add_dentry(string& name, dentry* dentry_node, dentry* parent)
{
    dentryKey dk = { parent, name };
    if(dentry_table_.count(dk) != 0) { return false; }

    dentry_table_[dk] = dentry_node;        // 成功加入

    return true;
}


bool dcache::erase_dentry(string& name, dentry* dentry_node)
{
    size_t erasedCount = dentry_table_.erase({dentry_node, name});
    if(erasedCount > 0) { return true; }
    else { return false; }
}



///////////////////////////////////
///         dirTree             ///
///////////////////////////////////

void dirTree::init_root(string root_name="/", size_t root_inode_num, inode* root_inode)
{
    root_ = new dentry(root_name, root_inode, root_inode_num, nullptr);     // root无父节点
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
                    // TODO: 磁盘中访问 search 的 inode, 读取其中的目录项
                    
                    /// 此处进行I/O操作获得子目录项存入dir_entries
                    vector<dir_entry> dir_entries;              // 假定已经返回了目录项

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

    if(sub_dir != nullptr && sub_dir->get_flag() == FIRST_LOAD_TO_MEMORY)  { return false; }    
    
    // 最后保证磁盘中确实无此目录

    if(sub_dir->get_flag() == CUT_SUBDIRS) {

        /// 此处进行I/O操作获得子目录项存入dir_entries
        vector<dir_entry> dir_entries;              // 假定已经返回了目录项

        work_dir->add_subdir(dir_entries);       // 这时加入了新的子节点

        auto new_add_dir = work_dir->find_subdir(name);
        if(new_add_dir != nullptr)  { return false; }       // 如果此时新加入的节点中有, 则创建新目录失败

    }

    return true;        // 没找到同名的节点
}

bool dirTree::alloc_dir(string& name, dentry* work_dir)
{
    if(!name_search_test(name, work_dir)) { return false; }  // 如果找到了同名的, 则分配新目录失败

    /////////////////// 下面是可以创建新的目录项了

    // TODO:通知I/O分配新的inode

    // 如果此时已经获得了inode(内存)
    inode* new_allocate_inode;

    // 完善inode的信息
    new_allocate_inode->i_type = DIR;
    ///  .....

    dentry* new_node = new dentry(name, new_allocate_inode, new_allocate_inode->i_num, work_dir);
    work_dir->add_single_subdir(new_node);      // 为当前工作路径加入新的子目录
    // 还需要更新全局哈希
    cache_->add_dentry(name, new_node, work_dir);

    return true;
}

bool dirTree::free_dir(string& name, dentry* work_dir)
{
    // 首先还是一样的查找验证过程
    if(name_search_test(name, work_dir))  { return false; }       // 如果没找到, 则删除失败

    // TODO: 通知I/O回收该节点


    if(work_dir->erase_subdir(name)){
        // 子节点删除成功后再删除全局哈希表项
        if(!cache_->erase_dentry(name, work_dir))   { return false; }
    } 
    else{ 
        return false;       // maybe more serious
    }
    return true;
}