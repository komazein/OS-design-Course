#include "dentry.h"
#include <spdlog/spdlog.h>
#include"ddq.h"




///////////////////////////////
///         dentry          ///
///////////////////////////////

void dentry::add_subdir(vector<dir_entry>& dir_entries)
{
    cout<<dir_entries.size()<<endl;
    for(auto& dir_entry : dir_entries){
        dentry* child_dentry = new dentry(dir_entry.name, dir_entry.inode_num, this,dir_entry.type);
        d_child_[dir_entry.name] = child_dentry;

        // 日志显示
        spdlog::info("Added subdir '{}' (inode={}) under '{}'", 
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
        temp.type=dentry_node->get_type();
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
    root_ = new dentry(root_name, root_inode, root_inode_num, nullptr,DIR);     // root无父节点

    root_inode->i_type = DIR;

    root_inode->i_block[0]=0;
    auto cur_time = get_time();
    root_inode->i_atime = cur_time;
    root_inode->i_ctime = cur_time;
    root_inode->i_mtime = cur_time;
    root_->set_flag(FIRST_LOAD_TO_MEMORY);
    root_->set_dirty(true);

    spdlog::info("Initialize the file system root dentry node");

}
void dirTree::load_root(inode*root_inode)
{
    string root_name="/";
    auto node = new dentry(root_name, root_inode, root_inode->i_num, nullptr,DIR);
    root_ = node;
  

    // root_ = new dentry(root_name, root_inode, root_inode->i_num, nullptr);    // root无父节点

    ////////注意时间
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

    bool start = true;      // 只有第一个以'/'开头的才表示绝对路径, 后面可能是这样的形式: a/b/, 此时最后这个就得忽略

    string name;        // 存储分割的字符串
    while(getline(ss, name, '/')){

        dentry* dentry_next;

        if(name.empty() && start == true){
            // 如果此时以"/"为开始的路径名, 则说明为绝对路径
            // 从root_开始查找
            search = get_root();                //////////////////////
            start = false;
            continue;
        }

        start = false;  

        if(name == "." || (name.empty() && start == false)){
            // 如果有当前路径的标识或者最后的/(见上), 自动跳过
            continue;
        }

        if(name == ".."){
            if(search->get_parent() != nullptr){
                // 如果有到上一级目录
                //dentry_next = search->get_parent();     // 回退到上一级
                search = search->get_parent();     // 回退到上一级
                
                continue;
            }
            else{
                spdlog::error("You have been at the root directory, can't access root above!");
                return nullptr;
            }
        }

        // 还是优先哈希查找
        dentry_next = hash_search(name, search);

        if(dentry_next == nullptr){
            // 没找到则直接查找本dentry的child
            dentry_next = search->find_subdir(name);

            if(dentry_next == nullptr) {
                if(search->get_flag() == FIRST_LOAD_TO_MEMORY) {
                    // 如果还没找到, 就是确实没有
                    return nullptr;         // 查找失败
                } else {
                    cout<<"nowfind_name:"<<name<<endl;

                    // 说明此时为剪枝后才没有其子目录项, 需要重新进行I/O读取其子目录
                    // TODO_finish: 磁盘中访问 search 的 inode, 读取其中的目录项
                    spdlog::info("In order to ensure '{}' whether have '{}',loading it's children from disk.", search->get_name(), name);
                    
                    /// 此处进行I/O操作获得子目录项存入dir_entries
                    vector<dir_entry> dir_entries;              // 假定已经返回了目录项
                    if(search->get_inode()==nullptr)
                    {
                        inode*temp=(inode*)malloc(sizeof(inode));
                        temp->i_num=search->get_inode_num();
                        search->set_inode(temp);
                        bs->ReWrinode(*search->get_inode(),true);
                    }
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
    ////if(search->get_inode()==nullptr)已经删了,报错再恢复
    spdlog::info("Traversal success: reached '{}'", search->get_name());

    if(search->get_type() == SIM_FILE){
        spdlog::error("You are trying to step into a file '{}'!", search->get_name());
        return nullptr;
    }

    if(search->get_type() == LINK){
        // 如果为link, 则需要读出数据块内容(得到了绝对路径), 并转到那个绝对路径指向的地方
        ///TODO: 读取数据块内容

        if(search->get_inode()==nullptr)
        {
            inode*tempinode=(inode*)malloc(sizeof(inode));
            tempinode->i_num=search->get_inode_num();
            bs->ReWrinode(*tempinode,true);
            search->set_inode(tempinode);
        }
        char* fileContents = bs->readSIMfromBLOCK(*search->get_inode());
        string sourse_name=fileContents;
        search = name_travesal(sourse_name, get_root());         // 从/开始
        spdlog::info("Traversal to the LINK point towards '{}'.", sourse_name);
    }
    return search;

}

bool dirTree::add_hash(string& name, dentry* parent, dentry* dentry_node)
{
    return cache_->add_dentry(name, dentry_node, parent);
}

bool dirTree::name_search_test(string& name, dentry* work_dir/*, bool update_hash*/)
{
    if(work_dir->get_flag() == UNLOAD_CHILD_FROM_DISK) {   


        // spdlog::info("In order to ensure '{}' whether have '{}',loading it's children from disk.", work_dir->get_name(), name);
        /// TODO_finish:此处进行I/O操作获得子目录项存入dir_entries
        

        vector<dir_entry> dir_entries;              // 假定已经返回了目录项
        
        cout<<work_dir->get_inode()->i_num<<endl;
        bs->loadchild(dir_entries, *work_dir->get_inode());


        for(int i=0;i<dir_entries.size();i++)
        {
            cout<<dir_entries[i].name<<" ";
        }
        cout<<endl;

        work_dir->set_flag(FIRST_LOAD_TO_MEMORY);
        
        work_dir->add_subdir(dir_entries);       // 这时加入了新的子节点
        
        auto new_add_dir = work_dir->find_subdir(name);
        if(new_add_dir != nullptr)  { return false; }       
    }
    // 首先初步利用哈希查找是否存在该目录项(保证不能重名)
    if(hash_search(name, work_dir)) { return false; }

    // 再保证当前目录下确实无此目录
    auto sub_dir = work_dir->find_subdir(name);

    if(sub_dir != nullptr /*&& work_dir->get_flag() == FIRST_LOAD_TO_MEMORY*/)  { return false; }    
    
    // 最后保证磁盘中确实无此目录

    

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
        ///////////////////////////
        // 此时待删除的dentry_node还没有加载进它的全部子, 所以他的inode为空, 所以先要加载它的子
        // 访问此inode的磁盘, 获取它的全部孩子并创建

        /// TODO: 获取dentry_node的inode
        inode* new_inode = (inode*)malloc(sizeof(inode));
        new_inode->i_num = dentry_node->get_inode_num();
        bs->ReWrinode(*new_inode, true);
        dentry_node->set_inode(new_inode);
        

        if(dentry_node->get_type()==DIR)
        {
            bs->loadchild(dir_entries, *dentry_node->get_inode());
            dentry_node->add_subdir(dir_entries);       // 加入子
        }
        dentry_node->set_flag(FIRST_LOAD_TO_MEMORY);

    //                     if(dentry_node->get_name() == "user5"){
    //     cout << dentry_node->get_flag();
    //     exit(1);
    // }
        // 此时检查是否加入了新的
        if(!dentry_node->has_subdir())  { return true; }
        else {  
            return false;
               }      // 加入了新的
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

    //inode* temp= bs->iget(false);
    
    // if(temp==NULL)
    //     return false;//无充足inode  
    // (*new_allocate_inode)=(*temp);
    new_allocate_inode = bs->iget(false);
    if(new_allocate_inode == nullptr){
        spdlog::warn("NO FREE INODE");
        return false;
    }
    

    /// TODO_finish: 完善inode的信息
    auto cur_time = get_time();

    new_allocate_inode->i_type = type;
    if(new_allocate_inode->i_type == SIM_FILE || new_allocate_inode->i_type == LINK){
        //new_allocate_inode->i_size = 0;
        new_allocate_inode->i_mode = {'-',"rwx","r--","r--" };
    }
    if(new_allocate_inode->i_type == DIR){
        //new_allocate_inode->i_size = 1;
        new_allocate_inode->i_mode = {'d',"rwx","r--","r--" };
    }
    new_allocate_inode->i_size = 0;
    new_allocate_inode->i_atime = cur_time;
    new_allocate_inode->i_ctime = cur_time;
    new_allocate_inode->i_mtime = cur_time;
    
    ///  .....
    bool ableGetBlock= bs->creatFILE(work_dir->get_subdir().size(),*work_dir->get_inode(),*new_allocate_inode);
    
    if(ableGetBlock==false)
    {
        bs->freeinode(new_allocate_inode->i_num);
        spdlog::warn("NO FREE BLOCK");
        return false;///块不够
    }

    dentry* new_node = new dentry(name, new_allocate_inode, new_allocate_inode->i_num, work_dir,type);
    new_node->set_flag(FIRST_LOAD_TO_MEMORY);
    new_node->set_dirty(true);//////////////check_it after consider SIM_FILE

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


void dirTree::findNameInDirtree(const string& filename, dentry* work_dir, dentry* cur_dentry, bool fuzzy, vector<string>& name_list)
{

    cout << work_dir->get_name() << "   "<<  filename << endl;

    if(has_child_test(cur_dentry) && work_dir != cur_dentry){
        // 如果此时已经查找到了最根部, 才算最终完成



        if(has_string(filename, cur_dentry->get_name(), fuzzy)){
            string path;
            get_full_path(path, work_dir, cur_dentry);
           

            name_list.push_back(path);/////////////////////////////////
            if(!path.empty()){
                if(fuzzy){
                    spdlog::debug("Find a new file/dir name '{}' has '{}'", cur_dentry->get_name(), filename);
                }
                else{
                    spdlog::debug("Find a new file/dir name '{}'", filename);
                }
            }
        }

        return;
    }

    // 检查当前节点是否满足名称条件
    // cout<<"notget    "<<filename<<"   "<<cur_dentry->get_name()<<endl;
    // cout << has_string(filename, cur_dentry->get_name(), fuzzy) << endl;
    if(has_string(filename, cur_dentry->get_name(), fuzzy) && work_dir != cur_dentry){
        string path;
        get_full_path(path, work_dir, cur_dentry);

        if(!path.empty()){
            name_list.push_back(path);//////////////////////////////////
            if(fuzzy){
                spdlog::debug("Find a new file/dir name '{}' has '{}'", cur_dentry->get_name(), filename);
            }
            else{
                spdlog::debug("Find a new file/dir name '{}'", filename);
            }
            
        }
    }

    cout << cur_dentry->get_subdir().size() << endl;
    

    auto sub_dir = cur_dentry->get_subdir();
    for(auto& [name, dentry_node] : sub_dir){

        // 对此节点的所有子节点进行查找
        
        findNameInDirtree(filename, work_dir, dentry_node, fuzzy, name_list);        
    }


}




bool dirTree::has_string(const string& name1, const string& name2, bool fuzzy)
{
    if(fuzzy){
        // 进行模糊搜索
        auto idx = name2.find(name1);
        if(idx == string::npos){        // 不包含
            return false;
        }
        else { return true; }
    }
    return name1 == name2;
}


void dirTree::get_full_path(string& path, dentry* work_dir, dentry* cur_dentry)
{
    auto search_dentry = cur_dentry;
    bool is_root = false;

    if(work_dir == nullptr) { return; }


    while(search_dentry != nullptr && search_dentry != work_dir){

        search_dentry = search_dentry->get_parent();
        if(search_dentry->get_name()!="/"){
            is_root = false;
            path = search_dentry->get_name() + "/" + path;
        }
        else{
            is_root = true;
        }
    }

    if(search_dentry == nullptr) { // 保证有意外的终止条件(即向上遍历完了所有节点, 甚至在根节点之上)
        spdlog::error("Invalid path '{}' or Invalid workdir '{}'", path, work_dir->get_name());
        exit(1);
        return;
    }

    path = path  + cur_dentry->get_name();
    if(!is_root){
        path = "./" + path;
    }else{
        path = "/" + path;
    }
}


void dirTree::del_tree(dentry* dentry_root,vector<pair<inode ,size_t>>&del_nodes)
{
    if(!dentry_root) { return; }        // 仅仅是保证安全性, 应该不会执行此语句

    if(!has_child_test(dentry_root)) {       // 此时已经完成了子树的完整构建


        // 如果此时有子节点
        auto& sub_dirs = dentry_root->get_subdir();

        

        // 遍历并递归删除所有子节点
        del_nodes.push_back({*dentry_root->get_inode(),sub_dirs.size()});



        for (auto& [name, child_node] : sub_dirs) {
            del_tree(child_node,del_nodes);       // 递归删除子

            cache_->erase_dentry(name, dentry_root);  // 在 child 被 delete 前移除哈希映射

            // 更新replacer
            dcache_replacer_->Erase({dentry_root, name});
            dentry_replacer_->Erase(child_node);

        }

        dentry_root->clear_child();         // 必须释放完所有的子才能调用清空child_哈希表
    }else{
        del_nodes.push_back({*dentry_root->get_inode(),0});
    }
    

    /// TODO——: 1. 通知I/O回收此块

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

    if(name_search_test(name, work_dir))  { 
        return false; 
    }       // 如果没找到, 则删除失败

    // 此时将要释放以work_dir的以name为名的根的树
    // 需要更新父的表项
    // dentry* parent_node = work_dir/*->get_parent()*/;

    auto child_node = work_dir->get_subdir()[name];     ///////////////// ATTENTION

    if(child_node->get_type()!=DIR)
    {
        if(child_node->get_inode()==nullptr)
        {
            inode* new_inode = (inode*)malloc(sizeof(inode));
            new_inode->i_num = child_node->get_inode_num();
            bs->ReWrinode(*new_inode, true);
            child_node->set_inode(new_inode);
        }
        bs->freeSIMFILE(*child_node->get_inode());
        spdlog::info("Deleted FILE '{}' under '{}'", name, work_dir->get_name());
        return true;
    }
    size_t pri_parent_num_of_children=work_dir->get_subdir().size();

    vector<pair<inode, size_t>>del_nodes;






    // auto child_node = name_travesal(name, work_dir);

    

    
    // string child_name = child_node->get_name();

   
    del_tree(child_node,del_nodes);     // 此时可以删除树

    cout<<"del_nodes"<<del_nodes.size()<<endl;


    work_dir->erase_subdir(name);        // 待删除节点的根节点移除此项
    
    work_dir->set_dirty(true);                           // 设置父的节点脏位


    

    cache_->erase_dentry(name, work_dir);                   // 清除全局哈希

    dcache_replacer_->Erase({work_dir, name});           // dentry_replacer清除

    dentry_replacer_->Erase(child_node);                      // dentry_replacer清除

    for(int i=0;i<del_nodes.size();i++)
    {
        cout<<i<<" "<<del_nodes[i].first.i_num<<" "<<del_nodes[i].second<<endl;
    }

    bs->freeblock(del_nodes,*work_dir->get_inode(),pri_parent_num_of_children);

    spdlog::info("Deleted subdir '{}' under '{}'", name, work_dir->get_name());

    return true;
}

void dirTree::cut_tree(dentry* dentry_node, size_t& counter)
{
    // dentry_node->set_flag(CUT_SUBDIRS);     // 标记此节点为剪枝后的, 说明它有自己子, 只不过被换出了

    if(!dentry_node) { return; }        //仅为安全性检查, 正常不会执行

        auto& sub_dirs = dentry_node->get_subdir();         // 获取其子
        //cout<<dentry_node->get_name()<<" "<<dentry_node->get_dirty()<<endl;
        // 此时根据脏位判断是否需要写回disk中
        if(dentry_node->get_dirty()){

            // dirtry : true 需要写回disk中
            /// TODO: 通知bs刷盘
            if(dentry_node->get_type()==DIR)
            {
                vector<dir_entry>child;
                dir_entry par;
                dentry_node->getDir_entry(par,child);
                bs->writechild(par,child,*dentry_node->get_inode(),child.size());//dentry_node->get_subdir().size()
            }
            else
            {
                cout<<dentry_node->get_name()<<" "<<dentry_node->get_inode()->i_size<<endl;
                bs->ReWrinode(*dentry_node->get_inode(),false);
            }
            // bs->writeBlocknumFOR>sim();//mulu
            //writeinode
            spdlog::info("Before recycle the '{}' node, write back to disk.", 
                    dentry_node->get_name());

        }


        // 递归删除所有的子节点
        for (auto& [name, child_node] : sub_dirs) {
            cache_->erase_dentry(name, dentry_node);  // 在 child 被 delete 前移除哈希映射

            // 更新replacer
            dcache_replacer_->Erase({dentry_node, name});
            dentry_replacer_->Erase(child_node);

            cut_tree(child_node, counter);           // 释放节点
        }
        dentry_node->clear_child();         // 必须释放完所有的子才能调用清空child_哈希表

    
    if(dentry_node->get_parent() != nullptr){
        spdlog::info("Free the '{}' node under '{}' for memory free.", 
            dentry_node->get_name(), dentry_node->get_parent()->get_name());
    }

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


size_t dirTree::cut_dirTree(double cut_ratio)
{
    size_t counter = 0;

    if(cut_ratio != 0){

        // 获得需要剪枝的个数
        size_t shrink_num = static_cast<size_t>(ceil(dentry_replacer_->get_cur_size() * cut_ratio));

        while(counter < shrink_num && dentry_replacer_->get_cur_size() != 0){
            auto vitim_opt = dentry_replacer_->Victim();        // 选出最久未使用的dentry节点, **释放其子**
       
            if(!vitim_opt.has_value()) { break; }           // 这时无节点可置换

            dentry* vitimnode = vitim_opt.value();

            auto child_nodes = vitimnode->get_subdir();

            for(auto& [_, node] : child_nodes){      // 释放其子节点
                cut_tree(node, counter);
            }
            
            if(counter){        // 如果释放了其子节点, 则标记为此节点被剪枝
                vitimnode->set_flag(UNLOAD_CHILD_FROM_DISK);
            }
        }
        spdlog::info("Ultimate free '{}' dentry_nodes from memory, now currunt nodes size is '{}'", 
                        counter, dentry_replacer_->get_cur_size());
        return counter;
    }
    
    while(dentry_replacer_->get_cur_size() > dentry_replacer_->get_max_size()) {

        auto vitim_opt = dentry_replacer_->Victim();        // 选出最久未使用的dentry节点, **释放其子**
       
        if(!vitim_opt.has_value()) { break; }           // 这时无节点可置换

        dentry* vitimnode = vitim_opt.value();

        auto child_nodes = vitimnode->get_subdir();

        for(auto& [_, node] : child_nodes){      // 释放其子节点
            cut_tree(node, counter);
        }
        
        if(counter){        // 如果释放了其子节点, 则标记为此节点被剪枝
            vitimnode->set_flag(UNLOAD_CHILD_FROM_DISK);
        }

        spdlog::info("Ultimate free '{}' dentry_nodes from memory, now currunt nodes size is '{}'", 
                        counter, dentry_replacer_->get_cur_size());
    }

    return counter;
}

bool dirTree::add_soft_link(string& sourse_dir, string& target_dir, dentry* work_dir)
{
    // 首先查找sourse_dir是否存在
    auto sourse_dir_dentry = name_travesal(sourse_dir, work_dir);

    if(sourse_dir_dentry == nullptr){
        spdlog::warn("Add hard link failed. Because of the '{}' doesn't exists.", sourse_dir);
        return false;
    }

    auto sourse_inode = sourse_dir_dentry->get_inode();
    

    /// TODO__: 注意, 这里只考虑了a/b/c/d, 没有考虑a/b/c/d/的情况
    string target_dir_prefix = target_dir;           // target_dir的去除最后一个/前的前缀字符串
    if (!target_dir_prefix.empty() && target_dir_prefix.back() == '/') {
        target_dir_prefix.pop_back();
    }

    size_t pos = target_dir.rfind('/');        // 找到最后一个/的位置

    if (pos != string::npos) {
        target_dir_prefix = target_dir.substr(0, pos);   
    }
    else{
        /// TODO: 有错误的输入
        spdlog::error("Invalid path input");
        return false;
    }

    string basename = target_dir.substr(pos + 1);

    cout << "basename" << basename << endl;

    cout << "++++++++++++++++++++(((((((())))))))\n";
    cout << target_dir_prefix << "     " << endl;

    auto target_dir_dentry = name_travesal(target_dir_prefix, work_dir);
   

    // 为软连接新建文件, 此文件仅为LINK类型, 内容记录sourse_dir的绝对路径
    if(!alloc_dir(basename, target_dir_dentry, nullptr, LINK)){
        return false;       // 失败
    }


    string store_name;          // 存储进文件的路径名称(绝对路径)
    get_full_path(store_name, root_, sourse_dir_dentry);

    cout <<store_name <<endl;

    ///TODO: 将store_name写入文件中
    char* c_store_name = (char*)malloc(store_name.size() + 1);
    strcpy(c_store_name, store_name.c_str());
    target_dir_dentry->set_dirty(true);
    target_dir_dentry=target_dir_dentry->get_subdir()[basename];
    target_dir_dentry->set_dirty(true);
    if(!bs->writeSIMfromBLOCK(*target_dir_dentry->get_inode(), c_store_name)){
        spdlog::error("Error occur when write link file back to disk.");
        return false;
    }
    spdlog::info("Add soft link from '{}' to '{}'", target_dir, sourse_dir);
    return true;
}