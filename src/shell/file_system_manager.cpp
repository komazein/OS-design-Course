#include "file_system_manager.h"
#include <spdlog/spdlog.h>


void file_system_manager::command_mkdir(std::string& dirname) {
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
    }
    
    bool result = dir_tree_->alloc_dir(dirname, current_dir_, nullptr, DIR);
    if(result){
        spdlog::info("Directory '{}' created successfully in '{}'", dirname, current_dir_->get_name());
    }else
    {
        spdlog::warn("Directory '{}' already exists in '{}'", dirname, current_dir_->get_name());
    }

}

void file_system_manager::command_cd(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
    }

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
    for (const auto& pair : current_dir_->get_subdir()) {
        std::string subdir_name = pair.first;
        dentry* subdir = pair.second;
        TYPE type = subdir->get_inode()->i_type;
        printLsResult(subdir_name, type, ++resultNum); 
    }
}

void file_system_manager::command_rm(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        Exit();
    }

    bool result = dir_tree_->free_dir(dirname, current_dir_);
    if (result) {
        spdlog::info("Directory '{}' removed successfully from '{}'", dirname, current_dir_->get_name());
    } else {
        spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
    }
}

//TODO: finish func of symbolic link dir
void file_system_manager::command_lkdir(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create symbolic link '{}'", dirname);
        Exit();
    }

}

//TODO: finish func of hard link dir
void file_system_manager::command_lndir(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create hard link '{}'", dirname);
        Exit();
    }

}

void file_system_manager::command_find(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot find file '{}'", filename);
        Exit();
    }

    vector<string> foundPath;
    bool fuzzy = false;         // 精确搜索
    dir_tree_->findNameInDirtree(filename, current_dir_, current_dir_, fuzzy, foundPath);
    if (foundPath.size() != 0) {
        spdlog::info("File '{}' found in '{}'", filename, current_dir_->get_name());
        printPath(foundPath);
    } else {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
    }
}

//TODO: finish func of touch file
void file_system_manager::command_touch(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create file '{}'", filename);
        Exit();
    }

}

//TODO: finish func of delete file
void file_system_manager::command_delete(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot delete file '{}'", filename);
        Exit();
    }

}

//TODO: finish func of cat file
void file_system_manager::command_cat(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot read file '{}'", filename);
        Exit();
    }

}

void file_system_manager::Exit()
{
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
}

void file_system_manager::help()
{
    showHelpMessage();
}