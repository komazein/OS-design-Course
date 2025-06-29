#include "file_system_manager.h"
#include <spdlog/spdlog.h>
#include "log_global.h"

void file_system_manager::command_mkdir(std::string& dirname) {
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
        
    }
    uint8_t owner= current_dir_->get_inode()->i_acl.owner;
    if(owner >> (logInUser_->getUid() - 1) & 1)
    bool result = dir_tree_->alloc_dir(dirname, current_dir_, nullptr, DIR);
    else
    {
        spdlog::warn("user '{}' has no authorization to build directory '{}'", logInUser_->getUid(), dirname);
        g_logger->warn("user '{}' has no authorization to build directory '{}'", logInUser_->getUid(), dirname); 
    }
    // exit(1);
}

void file_system_manager::command_cd(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
    }

    // 濡傛灉涓虹┖鍚嶅瓧(鍗冲悕瀛楀弬鏁版湭鎸囧畾, 閫€鍥炲埌鏍硅妭鐐?
    /// TODO: 瀹為檯涓婃槸閫€鍥炲埌鐢ㄦ埛鐩綍涓?
    if(dirname.empty()) { current_dir_ = dir_tree_->get_root(); }       

    cout <<current_dir_->get_name()<<current_dir_->get_subdir().size() <<endl;

    dentry* new_dir = dir_tree_->name_travesal(dirname, current_dir_);
    if (new_dir) {
        current_dir_ = new_dir;
        spdlog::info("Changed directory to '{}'", current_dir_->get_name());
    } else {
        spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
        g_logger->warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
    }
}

void file_system_manager::command_cdKC(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
    }

    // 濡傛灉涓虹┖鍚嶅瓧(鍗冲悕瀛楀弬鏁版湭鎸囧畾, 閫€鍥炲埌鏍硅妭鐐?
    /// TODO: 瀹為檯涓婃槸閫€鍥炲埌鐢ㄦ埛鐩綍涓?
    if(dirname.empty()) { current_dir_ = dir_tree_->get_root(); }       

    cout <<current_dir_->get_name()<<current_dir_->get_subdir().size() <<endl;

    dentry* new_dir = dir_tree_->name_travesal(dirname, current_dir_);
    if (new_dir) {
        current_dir_ = new_dir;
        spdlog::info("Changed directory to '{}'", current_dir_->get_name());
    } else {
        current_dir_ = dir_tree_->get_root();
        spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
    }
}

void file_system_manager::command_ls()
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot list contents");
        Exit();
    }

    g_logger->info("Listing contents of directory '{}':", current_dir_->get_name());
    spdlog::info("Listing contents of directory '{}':", current_dir_->get_name());
    
    int resultNum = 0;
    dir_tree_->has_child_test(current_dir_);                // 棣栧厛妫€鏌ヤ竴涓嬫槸鍚︽湁瀛?
    for (const auto& pair : current_dir_->get_subdir()) {
        std::string subdir_name = pair.first;
        dentry* subdir = pair.second;
        TYPE type = subdir->get_type();
        Color color = (type == DIR) ? Color::Blue : (type == SIM_FILE) ? Color::Green : Color::White;
        g_log_color_buffer.push_back(std::make_pair(subdir_name, color));
        printLsResult(subdir_name, type, ++resultNum); 
    }
    cout<<endl;
}

void file_system_manager::command_rm(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot remove directory '{}'", dirname);
        Exit();
    }

    uint8_t owner = current_dir_->get_inode()->i_acl.owner;
    if(owner >> (logInUser_->getUid() - 1) & 1)
    {
        cout<<"("<<bs_->getfreeblocknum()<<","<<bs_->getfreeinodenum()<<")"<<endl;
        bool result = dir_tree_->free_dir(dirname, current_dir_);
        if (!result) {
            spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
            g_logger->warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
        }
        cout<<"("<<bs_->getfreeblocknum()<<","<<bs_->getfreeinodenum()<<")"<<endl;
    }
    else
    {
        spdlog::warn("user '{}' has no authorization to remove directory '{}'", logInUser_->getUid(), dirname);
        g_logger->warn("user '{}' has no authorization to remove directory '{}'", logInUser_->getUid(), dirname);
    }
}


bool validateAndExtractUserNumber(const std::string& target_path, int& user_num) {
    // 妫€鏌ュ瓧绗︿覆闀垮害鏄惁瓒冲
    if (target_path.length() < 6 || target_path[0] != '/') {
        return false;
    }
    
    // 妫€鏌ュ墠5涓瓧绗︽槸鍚︽槸 "/user"
    if (target_path.substr(0, 5) != "/user") {
        return false;
    }
    
    // 妫€鏌ョ6涓瓧绗︽槸鍚︽槸鏁板瓧1-9
    char c = target_path[5];
    if (c < '1' || c > '9') {
        return false;
    }
    
    // 妫€鏌ョ7涓瓧绗︽槸鍚︽槸 '/'
    if (target_path.length() < 7 || target_path[6] != '/') {
        return false;
    }
    
    // 鎻愬彇鏁板瓧
    user_num = c - '0';
    return true;
}


void file_system_manager::command_lkdir(std::string& source_path, std::string& target_path)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create symbolic link in the path '{}'", target_path);
        return;
    }
    
    /// TODO: 淇敼
    // uint8_t owner = current_dir_->get_inode()->i_acl.owner;
    // /user
    int user_id;
    if(!validateAndExtractUserNumber(target_path, user_id)){
        spdlog::error("Invalid target path '{}'", target_path);
        g_logger->warn("Invalid target path '{}'", target_path);
        return;
    }

    // 澧炲姞target_dir鐨勬牸寮忓垽鏂?

    if((user_id >> (logInUser_->getUid() - 1)) & 1)
        dir_tree_->add_soft_link(source_path, target_path, current_dir_);
    else
        spdlog::warn("user '{}' has no authorization to link directory path '{}'", user_id, source_path);
        g_logger->warn("user '{}' has no authorization to link directory path '{}'", logInUser_->getUid(), source_path);
    
}

void file_system_manager::command_lndir(std::string& source_path, std::string& target_path)
{
    cout<<current_dir_->get_name()<<":::::::::cur"<<endl;
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create hard link in path '{}'", target_path);
        Exit();
    }
    if(current_dir_->get_inode()==nullptr)
    {
        inode* new_inode = (inode*)malloc(sizeof(inode));
        new_inode->i_num = current_dir_->get_inode_num();
        get_bs()->ReWrinode(*new_inode, true);
        current_dir_->set_inode(new_inode);
    }
    // uint8_t owner = current_dir_->get_inode()->i_acl.owner;
    // if(owner >> (logInUser_->getUid() - 1) & 1)
    // {   
    //     file_manager_->fileHardLink(source_path, target_path, logInUser_->getUid(), current_dir_);
    // }
    // else
    // {
    //     spdlog::warn("user '{}' has no authorization to link directory path '{}'", logInUser_->getUid(), source_path);
    // }
    cout<<current_dir_->get_name()<<":::::::::cur"<<endl;
    file_manager_->fileHardLink(source_path, target_path, logInUser_->getUid(), current_dir_);
}

void file_system_manager::command_find(std::string& filename, bool fuzzy)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot find file '{}'", filename);
        Exit();
    }

    vector<string> foundPath;

    

    dir_tree_->findNameInDirtree(filename, current_dir_, current_dir_, fuzzy, foundPath);
    
    for(const auto& path : foundPath) {
        g_log_buffer.push_back(path);
    }

    if (foundPath.size() != 0) {
        spdlog::info("File '{}' found in '{}'", filename, current_dir_->get_name());
        printPath(foundPath);
    } else {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        g_logger->warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
    }
}

void file_system_manager::command_touch(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create file '{}'", filename);
        Exit();
    }
    // Check if the file already exists
    if(current_dir_->find_subdir(filename) != nullptr) {//////////alloc_dir鍐呴儴妫€娴嬩簡锛屾澶勫彲浠ュ垹鎺?
        spdlog::warn("File '{}' already exists in '{}'", filename, current_dir_->get_name());
        return;
    }
    
    uint8_t owner = current_dir_->get_inode()->i_acl.owner;
    if(owner >> (logInUser_->getUid() - 1) & 1)
    {
        dir_tree_->alloc_dir(filename, current_dir_, nullptr, SIM_FILE);
    }
    else
    {
        spdlog::warn("user '{}' has no authorization to build file '{}'", logInUser_->getUid(), filename);
        g_logger->warn("user '{}' has no authorization to build file '{}'", logInUser_->getUid(), filename);
    }

}

//TODO: finish func of delete file
void file_system_manager::command_delete(std::string& filename)///////////rm dir鏇夸唬
{
    cout<<bs_->getfreeblocknum()<<endl;
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot delete file '{}'", filename);
        Exit();
    }
    dir_tree_->free_dir(filename,current_dir_);
    cout<<bs_->getfreeblocknum()<<endl;
}

//TODO: finish func of cat file
void file_system_manager::command_cat(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot read file '{}'", filename);
        Exit();
    }
    
    // Check if the file exists
    dir_tree_->has_child_test(current_dir_);
    dentry* file_entry = current_dir_->find_subdir(filename);
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        g_logger->warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        return;
    }

    // Call file manager to edit the file
    if(file_entry->get_inode()==nullptr)
    {
        inode* new_inode = (inode*)malloc(sizeof(inode));
        new_inode->i_num = file_entry->get_inode_num();
        bs_->ReWrinode(*new_inode, true);
        file_entry->set_inode(new_inode);
    }
    if(((file_entry->get_inode()->i_acl.owner>>(logInUser_->getUid()-1))&1) == 1)
    {
        file_manager_->openFile(filename, current_dir_);
        file_manager_->readFile(filename, current_dir_);
        file_manager_->closeFile(filename, current_dir_);
    }
    else
    {
         spdlog::warn("user '{}' has no authorization to read file '{}'", logInUser_->getUid(), filename);
         g_logger->warn("user '{}' has no authorization to read file '{}'", logInUser_->getUid(), filename);
    }
}

void file_system_manager::command_edit(std::string& filename, std::string& content)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot edit file '{}'", filename);
        Exit();
    }

    // Check if the file exists
    dir_tree_->has_child_test(current_dir_);
    dentry* file_entry = current_dir_->find_subdir(filename);
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        g_logger->warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        return;
    }
    if(file_entry->get_inode()==nullptr)
    {
        inode* new_inode = (inode*)malloc(sizeof(inode));
        new_inode->i_num = file_entry->get_inode_num();
        bs_->ReWrinode(*new_inode, true);
        file_entry->set_inode(new_inode);
    }
    if(((file_entry->get_inode()->i_acl.owner>>(logInUser_->getUid()-1))&1) == 1)
    {
        // Call file manager to edit the file
        file_manager_->openFile(filename, current_dir_);
        file_manager_->writeFile(filename, content, current_dir_);
        file_manager_->closeFile(filename, current_dir_);
    }
    else
    {
         spdlog::warn("user '{}' has no authorization to edit file '{}'", logInUser_->getUid(), filename);
         g_logger->warn("user '{}' has no authorization to edit file '{}'", logInUser_->getUid(), filename);
    }
}
void file_system_manager::command_exe(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot execute file '{}'", filename);
        Exit();
    }

    // Check if the file exists
    dir_tree_->has_child_test(current_dir_);
    dentry* file_entry = current_dir_->find_subdir(filename);
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        return;
    }

    if(file_entry->get_inode()==nullptr)
    {
        inode* new_inode = (inode*)malloc(sizeof(inode));
        new_inode->i_num = file_entry->get_inode_num();
        bs_->ReWrinode(*new_inode, true);
        file_entry->set_inode(new_inode);
    }
    if(((file_entry->get_inode()->i_acl.owner>>(logInUser_->getUid()-1))&1) == 1)
    {
        // Call file manager to execute the file
        file_manager_->openFile(filename, current_dir_);
        file_manager_->executeFile(filename, current_dir_);
        file_manager_->closeFile(filename, current_dir_);
    }
    else
    {
        spdlog::warn("user '{}' has no authorization to execute file '{}'", logInUser_->getUid(), filename);
        g_logger->warn("user '{}' has no authorization to execute file '{}'", logInUser_->getUid(), filename);
    }
}
void file_system_manager::Exit()
{
    size_t counter = 0;
    dir_tree_->cut_tree(dir_tree_->get_root(), counter, false);
    bs_->writesuperblock();
    spdlog::info("Exiting shell...");
    current_dir_=dir_tree_->get_root();
    g_logger->info("Exiting shell...");
    // Perform any necessary cleanup here
    // add any cleanup code if needed
    // print any manual or help information
}
bool file_system_manager::CMPuser(string username_,string password_)
{
    int uid_,gid_;
    string x=bs_->getUSERroot(username_,password_,uid_,gid_);
    if(x=="")
        return false;
    logInUser_->setprivate(username_,password_,uid_,gid_);
    return true;
}

void file_system_manager::command_athrz(std::string& filename, int uid)
{
    dir_tree_->has_child_test(current_dir_);
    auto dentry_entry = (current_dir_->has_subdir()) ? current_dir_->find_subdir(filename) : nullptr;
    if(dentry_entry == nullptr)
    {
        spdlog::warn("Can not fine file '{}'", filename);
        return;
    }
    if(dentry_entry->get_inode()==nullptr)
    {
        inode* new_inode = (inode*)malloc(sizeof(inode));
        new_inode->i_num = dentry_entry->get_inode_num();
        bs_->ReWrinode(*new_inode, true);
        dentry_entry->set_inode(new_inode);
    }
    if(dentry_entry->get_inode()->i_uid==logInUser_->getUid())
    {
        logInUser_->authorize(dentry_entry, uid);
    }
    else
    {
        spdlog::warn("user '{}' has no right to to authorize file '{}' ", uid, filename);
        g_logger->warn("user '{}' has no right to to authorize file '{}' ", uid, filename);
    }
}

void file_system_manager::command_rathrz(std::string& filename, int uid)
{
    dir_tree_->has_child_test(current_dir_);
    auto dentry_entry = (current_dir_->has_subdir()) ? current_dir_->find_subdir(filename) : nullptr;
    if(dentry_entry == nullptr)
    {
        spdlog::warn("Can not fine file '{}'", filename);
        return;
    }
    uint8_t owner = current_dir_->get_inode()->i_acl.owner;
    if(dentry_entry->get_inode()==nullptr)
    {
        inode* new_inode = (inode*)malloc(sizeof(inode));
        new_inode->i_num = dentry_entry->get_inode_num();
        bs_->ReWrinode(*new_inode, true);
        dentry_entry->set_inode(new_inode);
    }
    if(dentry_entry->get_inode()->i_uid==logInUser_->getUid())
    {
        logInUser_->recoverAuthorization(dentry_entry, uid);
    }
    else{
        spdlog::warn("user '{}' has no right to to recoverauthorize file '{}' ", uid, filename);
        g_logger->warn("user '{}' has no right to to recoverauthorize file '{}' ", uid, filename);
    }
}


void file_system_manager::command_free_memory(double ratio)
{
    auto free_counter = dir_tree_->cut_dirTree(ratio, current_dir_);
    if(free_counter == 0){
        spdlog::warn("No free node can be free!");
        g_logger->warn("No free node can be free!");
    }
    else{
        spdlog::info("Free {} node now", free_counter);
        g_logger->info("Free {} node now", free_counter);
    }
}


void file_system_manager::format()
{
    spdlog::info("Formatting the user '{}'...",logInUser_->getUid());
    g_logger->info("Formatting the user '{}'...",logInUser_->getUid());
    // Perform formatting operations here
    // This might involve clearing the directory tree and resetting the file system state
    logInUser_->fomat();
    current_dir_ =dir_tree_ ->get_root();
}

void file_system_manager::help()
{
    showHelpMessage();
}

 