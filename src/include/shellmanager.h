#pragma once
#include "fs.h";

class shell_manager{

    private:
        dirTree* dir_tree_;  // 目录树指针
        dentry* current_dir_;  // 当前目录项
        
    
    public:
        shell_manager(dirTree* dir_tree) : dir_tree_(dir_tree), current_dir_(nullptr) {
            // 初始化当前目录为根目录
            current_dir_ = dir_tree_->get_root();
        }
        // execute shell command for dir only

        /** 
        * @brief 创建目录命令
        * @param dirname 目录名称
        */
        void command_mkdir(const std::string& dirname);

        /**
         * @brief 切换当前目录命令
         * @param dirname 目录名称
         */
        void command_cd(const std::string& dirname);
        /**
         * @brief 列出当前目录下的所有文件和子目录
         */
        void command_ls();
  
        /**
         * @brief 删除目录命令
         * @param dirname 目录名称
         */
        void command_rm(const std::string& dirname);

        /**
         * @brief 创建符号链接命令
         * @param dirname 目标文件或目录
         */
        void command_lkdir(const std::string& dirname);

        /**
         * @brief 创建硬链接命令
         * @param dirname 目标文件或目录
         */
        void command_lndir(const std::string& dirname);

        /**
         * @brief 查找文件命令
         * @param filename 文件名
         */
        void command_find(const std::string& filename);

        // execute shell command for file only
        /**
         * @brief 创建文件命令
         * @param filename 文件名
         */
        void command_touch(const std::string& filename);

        /**
         * @brief 删除文件命令
         * @param filename 文件名
         */
        void command_delete(const std::string& filename);

        /**
         * @brief 读取文件内容命令
         * @param filename 文件名
         */
        void command_cat(const std::string& filename);

        // shell commmand for user
        void exit();

        void format();

        void help();

};