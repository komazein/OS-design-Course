#include "file_system_manager.h"
#include <spdlog/spdlog.h>

void file_system_manager::command_mkdir(std::string& dirname) {
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
        
    }
    cout<<"MKdir:freeblock&inode_num:"<<bs_->getfreeblocknum()<<" "<<bs_->getfreeinodenum()<<endl;
    bool result = dir_tree_->alloc_dir(dirname, current_dir_, nullptr, DIR);
    cout<<"After:MKdir:freeblock&inode_num:"<<bs_->getfreeblocknum()<<" "<<bs_->getfreeinodenum()<<endl;
    // exit(1);
}

void file_system_manager::command_cd(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
    }

    // 如果为空名字(即名字参数未指定, 退回到根节点
    /// TODO: 实际上是退回到用户目录下
    if(dirname.empty()) { current_dir_ = dir_tree_->get_root(); }       

    cout <<current_dir_->get_name()<<current_dir_->get_subdir().size() <<endl;

    dentry* new_dir = dir_tree_->name_travesal(dirname, current_dir_);
    if (new_dir) {
        current_dir_ = new_dir;
        spdlog::info("Changed directory to '{}'", current_dir_->get_name());
    } else {
        
        spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
    }
}

void file_system_manager::command_ls()
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot list contents");
        Exit();
    }

    spdlog::info("Listing contents of directory '{}':", current_dir_->get_name());
    
    int resultNum = 0;
    dir_tree_->has_child_test(current_dir_);                // 首先检查一下是否有子
    for (const auto& pair : current_dir_->get_subdir()) {
        std::string subdir_name = pair.first;
        dentry* subdir = pair.second;
        TYPE type = subdir->get_type();
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

    bool result = dir_tree_->free_dir(dirname, current_dir_);
    if (!result) {
        spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
    }
}

void file_system_manager::command_lkdir(std::string& source_path, std::string& target_path)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create symbolic link in the path '{}'", target_path);
        
    }
    
    dir_tree_->add_soft_link(source_path, target_path, current_dir_);
    
}

void file_system_manager::command_lndir(std::string& source_path, std::string& target_path)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create hard link in path '{}'", target_path);
        Exit();
    }

    file_manager_->fileHardLink(source_path, target_path);
    spdlog::info("Hard link created from '{}' to '{}'", source_path, target_path);
}

void file_system_manager::command_find(std::string& filename, bool fuzzy)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot find file '{}'", filename);
        Exit();
    }

    vector<string> foundPath;

    

    dir_tree_->findNameInDirtree(filename, current_dir_, current_dir_, fuzzy, foundPath);
    
    

    if (foundPath.size() != 0) {
        spdlog::info("File '{}' found in '{}'", filename, current_dir_->get_name());
        printPath(foundPath);
    } else {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
    }
}

void file_system_manager::command_touch(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create file '{}'", filename);
        Exit();
    }
    // Check if the file already exists
    if(current_dir_->find_subdir(filename) != nullptr) {
        spdlog::warn("File '{}' already exists in '{}'", filename, current_dir_->get_name());
        return;
    }
    
    
    dir_tree_->alloc_dir(filename, current_dir_, nullptr, SIM_FILE);
    spdlog::info("File '{}' created successfully in '{}'", filename, current_dir_->get_name());

}

//TODO: finish func of delete file
void file_system_manager::command_delete(std::string& filename)
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
//void file_system_manager::command_cat(std::string& filename)
string file_system_manager::command_cat(std::string& filename)////////////////////////////
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot read file '{}'", filename);
        Exit();
    }
    
    // Check if the file exists
    dentry* file_entry = current_dir_->find_subdir(filename);
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        return "";
    }

    // Call file manager to edit the file
    file_manager_->openFile(filename, current_dir_);
    string RET=file_manager_->readFile(filename, current_dir_);
    file_manager_->closeFile(filename, current_dir_);
    return RET;
}

void file_system_manager::command_edit(std::string& filename, std::string& content)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot edit file '{}'", filename);
        Exit();
    }

    // Check if the file exists
    dentry* file_entry = current_dir_->find_subdir(filename);
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
        return;
    }
    
    // Call file manager to edit the file
    cout<<"Wr:freeblock&inode_num:"<<bs_->getfreeblocknum()<<" "<<bs_->getfreeinodenum()<<endl;
    file_manager_->openFile(filename, current_dir_);
    file_manager_->writeFile(filename, content, current_dir_);
    file_manager_->closeFile(filename, current_dir_);
    cout<<"After:Wr:freeblock&inode_num:"<<bs_->getfreeblocknum()<<" "<<bs_->getfreeinodenum()<<endl;
}

void file_system_manager::Exit()
{
    size_t counter = 0;
    dir_tree_->cut_tree(dir_tree_->get_root(), counter);
    bs_->writesuperblock();
    spdlog::info("Exiting shell...");
    
    // Perform any necessary cleanup here
    // add any cleanup code if needed
    // print any manual or help information
}

void file_system_manager::format()
{
    spdlog::info("Formatting the file system...");
    // Perform formatting operations here
    // This might involve clearing the directory tree and resetting the file system state
    dir_tree_->get_bs()->new_disk();  // Assuming clear() is a method to reset the root directory
    dir_tree_->get_bs()->freshUSER();
}

void file_system_manager::help()
{
    showHelpMessage();
}