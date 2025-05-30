
#include "user.h"
#include <spdlog/spdlog.h>

void user::authorize(dentry* accessed_file, user* authorized_user)
{
    ACL acl = accessed_file->get_inode()->i_acl;
    int uid = authorized_user->uid_;

    // Check if the user has been the owner of the file
    if((acl.owner >> uid) & 1)
    {
        spdlog::warn("User {} has already been the owner of the file {}", uid, accessed_file->get_name());
    }
    else
    {
        // if the user is not the owner, set the owner bit 1 which represents the user has been the owner of the file
        acl.owner |= (1 << uid);
        spdlog::info("User {} is now authorized to access the file {}", uid, accessed_file->get_name());
    }
}

void user::recoverAuthorization(dentry* accessed_file, user* authorized_user)
{
    ACL acl = accessed_file->get_inode()->i_acl;
    int uid = authorized_user->uid_;

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

}

void user::login(std::string& username, std::string& password)
{
    // This function would typically check the username and password against a database or a file.
    
}