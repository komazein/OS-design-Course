
#include "../include/filemanager.h"
#include <spdlog/spdlog.h>

void file_manager::openFile(std::string& filename, dentry* current_dir)
{
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot open file '{}'", filename);
        return;
    }

    // finding the file in the current directroy
    dentry* file_entry = current_dir->find_subdir(filename);
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }

    // check if the file is already open
    for (const auto& f : open_files_) {
        if (f.f_path == file_entry->get_name()) {
            spdlog::warn("File '{}' is already open", filename);
            return;
        }
    }

    // create a new file structure to represent the opened file 
    file new_file;
    new_file.f_inode = file_entry->get_inode();
    new_file.f_type = SIM_FILE;  // type is file 
    new_file.f_path = file_entry->get_name(); // path is the name of the file
    new_file.f_ref = 1;  // open the file, so reference count is 1
    //fd starts from 0 and is incremented for each new file opened
    new_file.fd = open_files_.size();  // fd is the index in the open_files_ vector
    
    open_files_.push_back(new_file);
    spdlog::info("File '{}' opened successfully", filename);
}

void file_manager::closeFile(std::string& filename, dentry* current_dir)
{
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot close file '{}'", filename);
        return;
    }

    // get the file in the current directroy
    dentry* file_entry = current_dir->find_subdir(filename);
    // close file which is not exist
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }

    // check the file in the open files list
    for (auto it = open_files_.begin(); it != open_files_.end(); ++it) {
        if (it->f_path == file_entry->get_name()) {
            // decrease reference count
            /*
            according to the file struct's definition,  
            close file will not immdeiatiely remove the file from the open files list,
            but just decrease the reference count, and will remove the file from the open files 
            untile the reference count reaches 0.
            */
            if (--it->f_ref == 0) {
                open_files_.erase(it);  // remove the file from the open files list
                spdlog::info("File '{}' closed successfully", filename);
            } else {
                spdlog::info("File '{}' reference count decreased to {}", filename, it->f_ref);
            }
            return;
        }
    }

    spdlog::info("File '{}' is not currently open", filename);
}

void file_manager::readFile(std::string& filename, dentry* current_dir)
{
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot read file '{}'", filename);
        return;
    }

    // get the file in the current directroy
    dentry* file_entry = current_dir->find_subdir(filename);
    // file not exist
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }

    // check if the file is open
    for (const auto& f : open_files_) {
        if (f.f_path == file_entry->get_name()) {
            // read the file content and print it to the console
            // here we just print a message, in real implementation, we would read the content from disk
            spdlog::info("Reading file '{}': [Content would be displayed here]", filename);
            return;
        }
    }

    spdlog::warn("File '{}' is not currently open", filename);
}
