
#include "user.h"

#include "dentry.h"
#include <spdlog/spdlog.h>

void user::authorize(dentry* accessed_file, int authorized_user_id)
{
    

    
    ACL acl = accessed_file->get_inode()->i_acl;
    int uid = authorized_user_id;

    // Check if the user has been the owner of the file
    if((acl.owner >> (uid-1) & 1))
    {
        spdlog::warn("User {} has already been the owner of the file {}", uid, accessed_file->get_name());
    }
    else
    {
        // if the user is not the owner, set the owner bit 1 which represents the user has been the owner of the file
        acl.owner |= (1 << (uid-1));
        spdlog::info("User {} is now authorized to access the file {}", uid, accessed_file->get_name());
    }
}

void user::recoverAuthorization(dentry* accessed_file, int authorized_user_id)
{
    
    ACL acl = accessed_file->get_inode()->i_acl;
    int uid = authorized_user_id;

    // Check if the user is not the owner of the file
    // owner bit is 0 which represents the user has not right and no need to recover
    if(!(acl.owner >> uid) & 1)
    {
        spdlog::info("User {} is not authorized to access the file {}, no need to recovery", uid, accessed_file->get_name());
    }
    else
    {
        // if the user is  the owner, set the owner bit 0 which represents the user has no more been the owner of the file
        acl.owner &= ~(1 << uid);
        spdlog::info("User {} is now authorized to access the file {}", uid, accessed_file->get_name());
    }
}

void user::fomat()
{   
    string user_name = "user" + std::to_string(uid_);
    auto user_dentry = dir_tree_->name_travesal(user_name);
    dir_tree_->free_dir(user_name, user_dentry);
}

void user::login(std::string& username, std::string& password)
{
    // This function would typically check the username and password against a database or a file.
    
}

// void user::AuthorizedCat(std::string& filename)
// {
//     // This function would typically check if the user is authorized to read the file.
//     // If authorized, it would read the file and return its content.
    
//     dentry* file_entry = dir_tree_->name_travesal(filename);
//     if (file_entry == nullptr) {
//         spdlog::warn("File '{}' not found", filename);
//         return;
//     }

//     // Check if the user is authorized to read the file
//     // This is a placeholder for actual authorization logic
//     if (file_entry->get_inode()->i_acl.owner & (1 << uid_)) {
//         spdlog::info("User {} is authorized to read file '{}'", uid_, filename);
//         fileSystem->command_cat(filename);
//         // Read and return the content of the file
//     } else {
//         spdlog::warn("User {} is not authorized to read file '{}'", uid_, filename);
//     }
// }

// void user::AuthorizedEdit(std::string& filename, std::string& content)
// {
//     // This function would typically check if the user is authorized to write to the file.
//     // If authorized, it would write the content to the file.
    
//     dentry* file_entry = dir_tree_->name_travesal(filename);
//     if (file_entry == nullptr) {
//         spdlog::warn("File '{}' not found", filename);
//         return;
//     }

//     // Check if the user is authorized to write to the file
//     // This is a placeholder for actual authorization logic
//     if (file_entry->get_inode()->i_acl.owner & (1 << uid_)) {
//         spdlog::info("User {} is authorized to edit file '{}'", uid_, filename);
//         fileSystem->command_edit(filename, content);
//         // Write the content to the file
//     } else {
//         spdlog::warn("User {} is not authorized to edit file '{}'", uid_, filename);
//     }
// }