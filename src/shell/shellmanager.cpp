#include "../include/shellmanager.h"
#include <spdlog/spdlog.h>


void shell_manager::command_mkdir(std::string& dirname) {
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        exit();
    }
    
    bool result = dir_tree_->alloc_dir(dirname, current_dir_);
    if(result){
        spdlog::info("Directory '{}' created successfully in '{}'", dirname, current_dir_->get_name());
    }else
    {
        spdlog::warn("Directory '{}' already exists in '{}'", dirname, current_dir_->get_name());
    }

}

void shell_manager::command_cd(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        exit();
    }

    dentry* new_dir = dir_tree_->name_travesal(dirname, current_dir_);
    if (new_dir) {
        current_dir_ = new_dir;
        spdlog::info("Changed directory to '{}'", current_dir_->get_name());
    } else {
        spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
    }
}

void shell_manager::command_ls()
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot list contents");
        exit();
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

void shell_manager::command_rm(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create directory '{}'", dirname);
        exit();
    }

    bool result = dir_tree_->free_dir(dirname, current_dir_);
    if (result) {
        spdlog::info("Directory '{}' removed successfully from '{}'", dirname, current_dir_->get_name());
    } else {
        spdlog::warn("Directory '{}' not found in '{}'", dirname, current_dir_->get_name());
    }
}

//TODO: finish func of symbolic link dir
void shell_manager::command_lkdir(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create symbolic link '{}'", dirname);
        exit();
    }

}

//TODO: finish func of hard link dir
void shell_manager::command_lndir(std::string& dirname)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create hard link '{}'", dirname);
        exit();
    }

}

void shell_manager::command_find(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot find file '{}'", filename);
        exit();
    }

    vector<string> foundPath = dir_tree_->findNameInDirtree(filename, current_dir_);
    if (found_file) {
        spdlog::info("File '{}' found in '{}'", filename, current_dir_->get_name());
        printPath(found_file);
    } else {
        spdlog::warn("File '{}' not found in '{}'", filename, current_dir_->get_name());
    }
}

//TODO: finish func of touch file
void shell_manager::command_touch(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create file '{}'", filename);
        exit();
    }

}

//TODO: finish func of delete file
void shell_manager::command_delete(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot delete file '{}'", filename);
        exit();
    }

}

//TODO: finish func of cat file
void shell_manager::command_cat(std::string& filename)
{
    if (current_dir_ == nullptr) {
        spdlog::warn("Current directory is not set. Cannot read file '{}'", filename);
        exit();
    }

}

void shell_manager::exit()
{
    spdlog::info("Exiting shell...");
    // Perform any necessary cleanup here
    // add any cleanup code if needed
    // print any manual or help information
}

void shell_manager::format()
{
    spdlog::info("Formatting the file system...");
    // Perform formatting operations here
    // This might involve clearing the directory tree and resetting the file system state
    dir_tree_->get_block_scheduler()->new_disk();  // Assuming clear() is a method to reset the root directory
}

void shell_manager::help()
{
    showHelpMessage();
}