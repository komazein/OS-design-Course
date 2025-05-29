#pragma once

#include "fs.h"

class user {

    public:
        
        user(std::string username, std::string password, int uid, int gid, ACL user_acl)
            : username_(username), password_(password), uid_(uid), gid_(gid) {}
        
        /*
        * @brief Get the username of the user
        * recevies password as input to check if the user is authenticated
        */
        void login(std::string& password);
         
        void authorize(dentry* accessed_file, user* authorized_user);
           








    private:
        std::string username_;  // username
        std::string password_;  // password
        int uid_;               // user ID
        int gid_;               // group ID
};