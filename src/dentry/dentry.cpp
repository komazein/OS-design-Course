#include "dentry.h"
#include <spdlog/spdlog.h>
#include"ddq.h"


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
void dentry::getDir_entry(dir_entry&par,vector<dir_entry>&child)
{
    par.inode_num=(size_t)get_inode_num();
    string parname=get_name();
    strcpy(par.name,parname.c_str());
    par.type=get_inode()->i_type;
    for(auto&[name, dentry_node] : get_subdir()){

        dir_entry temp;
        strcpy(temp.name,name.c_str());
        temp.inode_num=dentry_node->get_inode_num();
        temp.type=dentry_node->get_inode()->i_type;
        child.push_back(temp);
    }
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

void dirTree::init_root(string root_name, size_t root_inode_num, inode* root_inode)
{
    root_ = new dentry(root_name, root_inode, root_inode_num, nullptr);     // root无父节点

    root_inode->i_type = DIR;

    root_inode->i_block[0]=0;
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


dentry* dirTree::name_travesal(string& path, dentry* work_dir)
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

        // 此时添加进入lru_list, 因为此时算上一次最新的访问
        dentry_replacer_->InsertDir(search);
        dcache_replacer_->Insert({search, name});

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
bool dirTree::alloc_dir(string& name, dentry* work_dir,inode* new_allocate_inode, TYPE type)
{
    if(!name_search_test(name, work_dir)) { 
        spdlog::warn("Directory allocation failed: '{}' already exists in '{}'", name, work_dir->get_name());
        return false; 
    }  // 如果找到了同名的, 则分配新目录失败

    /////////////////// 下面是可以创建新的目录项了

    /// TODO_finish:通知I/O分配新的inode

    inode* temp= bs->iget(false);
    (*new_allocate_inode)=(*temp);

    /// TODO_finish: 完善inode的信息
    auto cur_time = get_time();

    new_allocate_inode->i_type = DIR;
    new_allocate_inode->i_size = type;
    new_allocate_inode->i_atime = cur_time;
    new_allocate_inode->i_ctime = cur_time;
    new_allocate_inode->i_mtime = cur_time;

    ///  .....

    dentry* new_node = new dentry(name, new_allocate_inode, new_allocate_inode->i_num, work_dir);
    work_dir->add_single_subdir(new_node);      // 为当前工作路径加入新的子目录

    // 此目录被修改(因为增加了目录项), 所以设置脏位为true
    work_dir->set_dirty(true);

    // 还需要更新全局哈希
    cache_->add_dentry(name, new_node, work_dir);

    // 加入到lru_list中
    dentry_replacer_->InsertDir(new_node);
    dcache_replacer_->Insert({work_dir, name});

    spdlog::info("Allocated new directory '{}' under '{}', inode={}", 
                 name, work_dir->get_name(), new_allocate_inode->i_num);


    return true;
}

void dirTree::del_tree(dentry* dentry_root)
{
    if(!dentry_root) { return; }        // 仅仅是保证安全性, 应该不会执行此语句

    if(has_child_test(dentry_root)) {       // 此时已经完成了子树的完整构建
        // 如果此时有子节点
        auto& sub_dirs = dentry_root->get_subdir();
        // 遍历并递归删除所有子节点
        for (auto& [name, child_node] : sub_dirs) {
            cache_->erase_dentry(name, dentry_root);  // 在 child 被 delete 前移除哈希映射

            // 更新replacer
            dcache_replacer_->Erase({dentry_root, name});
            dentry_replacer_->Erase(child_node);

            del_tree(child_node);       // 递归删除子
        }

        dentry_root->clear_child();         // 必须释放完所有的子才能调用清空child_哈希表
    }

    /// TODO: 1. 通知I/O回收此块

    spdlog::info("Letting blockScheduler to recycle the '{}' children's inode and thier blocks", dentry_root->get_name());
    // 2. 释放此节点
    delete dentry_root;
    
    return;
}

bool dirTree::free_dir(string& name, dentry* work_dir)
{
    if(name == "/") { 

        spdlog::warn("Delete the root, clear the file system.");
        /// TODO_finish: 调用I/O的清空操作
        // bs->new_disk();
    }

    // 首先还是一样的查找验证过程
    if(name_search_test(name, work_dir))  { return false; }       // 如果没找到, 则删除失败


    // 此时将要释放以work_dir为根的树
    // 需要更新父的表项
    dentry* parent_node = work_dir->get_parent();

    parent_node->erase_subdir(work_dir->get_name());        // 待删除节点的根节点移除此项
    
    parent_node->set_dirty(true);                           // 设置父的节点脏位

    cache_->erase_dentry(name, parent_node);                   // 清除全局哈希

    dcache_replacer_->Erase({parent_node, name});           // dentry_replacer清除

    dentry_replacer_->Erase(work_dir);                      // dentry_replacer清除

    del_tree(work_dir);     // 此时可以删除树

    spdlog::debug("Deleted subdir '{}' under '{}'", name, parent_node->get_name());

    return true;
}

void dirTree::cut_dir(dentry* dentry_node, size_t& counter)
{

    
    // dentry_node->set_flag(CUT_SUBDIRS);     // 标记此节点为剪枝后的, 说明它有自己子, 只不过被换出了

    if(!dentry_node) { return; }        //仅为安全性检查, 正常不会执行

    if(dentry_node->has_subdir()) {

        auto& sub_dirs = dentry_node->get_subdir();         // 获取其子
        
        // 递归删除所有的子节点
        for (auto& [name, child_node] : sub_dirs) {
            cache_->erase_dentry(name, dentry_node);  // 在 child 被 delete 前移除哈希映射

            // 更新replacer
            dcache_replacer_->Erase({dentry_node, name});
            dentry_replacer_->Erase(child_node);

            cut_dir(child_node, counter);           // 释放节点
        }

        dentry_node->clear_child();         // 必须释放完所有的子才能调用清空child_哈希表
    }

    // 此时根据脏位判断是否需要写回disk中
    if(dentry_node->get_dirty()){

        // dirtry : true 需要写回disk中
        /// TODO: 通知bs刷盘


        spdlog::info("Before recycle the '{}' node, write back to disk.", 
                dentry_node->get_name());

    }

    spdlog::info("Free the '{}' node under '{}' for memory free.", 
        dentry_node->get_name(), dentry_node->get_parent()->get_name());

    delete dentry_node;
    
    ++counter;      // 统计删除的个数  

    return;

}


size_t dirTree::shrink_dcache()
{
    size_t counter = 0;         // 记录释放的个数
    while(dcache_replacer_->get_cur_size() > dcache_replacer_->get_max_size()) {

        // 如果此时的链表大小超过设定最大的大小, 则进行空间释放
        auto victim_opt = dcache_replacer_->Victim();
        
        if(!victim_opt.has_value()) break;

        const auto& victimnode = victim_opt.value();

        cache_->erase_dentry(victimnode.name, victimnode.parent);

        ++counter;
    }
    return counter;
}


size_t dirTree::cut_dirTree()
{
    size_t counter = 0;
    while(dentry_replacer_->get_cur_size() > dentry_replacer_->get_max_size()) {

        auto vitim_opt = dentry_replacer_->Victim();        // 选出最久未使用的dentry节点, **释放其子**
       
        if(!vitim_opt.has_value()) break;

        dentry* vitimnode = vitim_opt.value();

        auto child_nodes = vitimnode->get_subdir();

        for(auto& [_, node] : child_nodes){      // 释放其子节点
            cut_dir(node, counter);
        }
        
        if(counter){        // 如果释放了其子节点, 则标记为此节点被剪枝
            vitimnode->set_flag(CUT_SUBDIRS);
        }
    }

    return counter;
}