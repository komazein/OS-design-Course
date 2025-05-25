#include "dentry.h"

// 根据目录项添加子节点
// 需要子节点的inode, 子节点的inode可以当遍历到子节点时访问磁盘获得自己的inode再自行设置
void dentry::add_subdir(vector<dir_entry>& dir_entries)
{
    for(auto& dir_entry : dir_entries){
        dentry* child_dentry = new dentry(dir_entry.name, dir_entry.inode_num, this);
        d_child_[dir_entry.name] = child_dentry;
    }
}

// 查找子节点
dentry* dentry::find_subdir(string& name)
{
    auto it = d_child_.find(name);
    if(it == nullptr) { return nullptr; }
    else { return it->second; }
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




///////////////////////////////////
///         dirTree             ///
///////////////////////////////////

dentry* dirTree::hash_search(string& name, dentry* parent)
{
    return cache_.find_dentry(name, parent);
}


dentry* dirTree::name_travesal(string& path, dentry* work_dir=nullptr)
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
            cache_.add_dentry(name, dentry_next, search);
        }

        search = dentry_next;       // 继续寻找

        // 否则继续找子节点中是否有这个路径
    }

    return search;

}


bool dirTree::add_hash(string& name, dentry* parent, dentry* dentry_node)
{
    return cache_.add_dentry(name, dentry_node, parent);
}


bool dirTree::alloc_dir(string& name, dentry* work_dir)
{
    // 首先初步利用哈希查找是否存在该目录项(保证不能重名)
    if(hash_search(name, work_dir)) { return false; }

    // 再保证当前目录下确实无此目录
    auto sub_dir = work_dir->find_subdir(name);

    if(sub_dir != nullptr)  { return false; }      


}

bool dirTree::free_dir()
{

}