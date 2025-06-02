#pragma once

#include "dentry.h"
#include "ddq.h"
#include <string>

class blockScheduler;
class user {

    public:
        
        user(dirTree* dir_tree, std::string& username, std::string& password )
            :  dir_tree_(dir_tree)  {
                dir_tree_->get_bs()->getUSERroot(username, password, uid_, gid_);
            }

        
        /*
        * @brief Get the username of the user
        * recevies username and password from fronter and check with password from disk
        */
        void login(std::string& username, std::string& password);
         
        /*
        * @brief Authorize a user to access a file
        * @param accessed_file The file that the user is trying to access
        * @param authorized_user The user who is trying to access the file
        * This function checks if the user has permission to access the file
        * and authrorize the user throught the ACL (Access Control List) of the file.
        */
        void authorize(std::string& accessed_file, int authorized_user_id);
        
        /*
        * @brief Recover authorization for a user to access a file
        * @param accessed_file The file that the user is trying to access
        * @param authorized_user The user who is trying to access the file
        * This function checks if the user has permission to access the file
        * and recover the authorization for the user through the ACL (Access Control List) of the file.
        */
        void recoverAuthorization(std::string& accessed_file, user* authorized_user);
        
        /*
        * @brief Read a file with authorization
        * @param filename The name of the file to read
        * This function checks if the user is authorized to read the file
        * and reads the file content if authorized.
        */
        void AuthorizedCat(std::string& filename);
        
        /*
        * @brief Edit a file with authorization
        * @param filename The name of the file to edit
        * @param content The content to write to the file
        * This function checks if the user is authorized to edit the file
        * and writes the content to the file if authorized.
        */
        void AuthorizedEdit(std::string& filename, std::string& content);

        
        /*
        * @brief Format the user's directory tree
        * This function formats the user's directory tree, deleting all files and directories
        */
        void fomat();
        
        //void createUser();

        






    private:
        std::string username_;  // username
        std::string password_;  // password
        dirTree* dir_tree_;      // directory tree for the user
        int uid_;               // user ID
        int gid_;               // group ID
};