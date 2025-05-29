#pragma once

#include "dentry.h"

class file_manager {

    public:

    file_manager(dirTree* dir_tree, dentry* current_dir) :
        dir_tree_(dir_tree), current_dir_(current_dir) 
    { }
    /**
     * @brief open file
     * @param filename the name of the file to open
     * @param current_dir the current directory
     * open file and store the file descriptor in a structure
     * open func is used combined with close at any time
     */
    void openFile(std::string& filename, dentry* current_dir);

    /**
     * @brief close file
     * @param filename name of the file to close
     * @param current_dir the current directory
     * close file and release the file descriptor
     */
    void closeFile(std::string& filename, dentry* current_dir);
    
    /**
     * @brief read file
     * @param filename the name of the file to read
     * @param current_dir the current directory
     * read file content and print it to the console
     * the main purpose of this function is to read the file content
     * and print it to the console, not to return the content
     */
    void readFile(std::string& filename, dentry* current_dir);
    
    /** 
     * @brief write file
     * @param filename file name to write
     * @param content content to write to the file
     * @param current_dir the current directory
     * write file content to the file
     * if the file does not exist, create it
    */
    void writeFile(std::string& filename, std::string& content, dentry* current_dir);

    /*
    * @brief create a hard link to a file
    * @param filename the name of the file to link
    * @param current_dir the current directory
    * create a hard link to a file in the current directory
    */
    void file_manager::fileHardLink(std::string& source_path, std::string& target_path);

    
    
    private:
        dirTree* dir_tree_;  
        dentry* current_dir_;  

        

        /*struct file
        {
          inode* f_inode;        //  inode
          FILEMODE di_mode;       // mode
          TYPE f_type;
          string f_path;          // name
          uint16_t f_ref;         // reference count
          size_t fd;              // file descriptor
        };
        *
        */
        std::vector<file> open_files_;  // the list of open files

};