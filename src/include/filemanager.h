#pragma once

#include "fs.h"

class file_manager {

    public:

    /**
     * @brief 打开文件
     * @param filename 文件名
     * @param current_dir 当前目录项
     * open file and store the file descriptor in a structure
     * open func is used combined with close at any time
     */
    void openFile(const std::string& filename, dentry* current_dir);

    /**
     * @brief 关闭文件
     * @param filename 文件名
     * @param current_dir 当前目录项
     * close file and release the file descriptor
     */
    void closeFile(const std::string& filename, dentry* current_dir);
    
    /**
     * @brief 读取文件内容
     * @param filename 文件名
     * @param current_dir 当前目录项
     * read file content and print it to the console
     * the main purpose of this function is to read the file content
     * and print it to the console, not to return the content
     */
    void readFile(const std::string& filename, dentry* current_dir);
    
    /** 
     * @brief 写入文件内容
     * @param filename 文件名
     * @param content 写入的内容
     * @param current_dir 当前目录项
     * write file content to the file
     * if the file does not exist, create it
    */
    void writeFile(const std::string& filename, const std::string& content, dentry* current_dir);
    
    
    private:
        dirTree* dir_tree_;  // 目录树指针
        dentry* current_dir_;  // 当前目录项

        

        /*struct file
        {
          inode* f_inode;        // 指向打开文件对应的 inode
          FILEMODE di_mode;       // mode(权限)
          TYPE f_type;
          string f_path;          // 文件路径
          uint16_t f_ref;         // 引用计数
          size_t fd;              // 文件句柄
        };
        *
        */
        std::vector<file> open_files_;  // the list of open files

};