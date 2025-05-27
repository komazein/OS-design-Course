#pragma once
#include "fs.h";

class shell_manager{

    private:
        dirTree* dir_tree_;  // Ŀ¼��ָ��
        dentry* current_dir_;  // ��ǰĿ¼��
        
    
    public:
        shell_manager(dirTree* dir_tree) : dir_tree_(dir_tree), current_dir_(nullptr) {
            // ��ʼ����ǰĿ¼Ϊ��Ŀ¼
            current_dir_ = dir_tree_->get_root();
        }
        // execute shell command for dir only

        /** 
        * @brief ����Ŀ¼����
        * @param dirname Ŀ¼����
        */
        void command_mkdir(const std::string& dirname);

        /**
         * @brief �л���ǰĿ¼����
         * @param dirname Ŀ¼����
         */
        void command_cd(const std::string& dirname);
        /**
         * @brief �г���ǰĿ¼�µ������ļ�����Ŀ¼
         */
        void command_ls();
  
        /**
         * @brief ɾ��Ŀ¼����
         * @param dirname Ŀ¼����
         */
        void command_rm(const std::string& dirname);

        /**
         * @brief ����������������
         * @param dirname Ŀ���ļ���Ŀ¼
         */
        void command_lkdir(const std::string& dirname);

        /**
         * @brief ����Ӳ��������
         * @param dirname Ŀ���ļ���Ŀ¼
         */
        void command_lndir(const std::string& dirname);

        /**
         * @brief �����ļ�����
         * @param filename �ļ���
         */
        void command_find(const std::string& filename);

        // execute shell command for file only
        /**
         * @brief �����ļ�����
         * @param filename �ļ���
         */
        void command_touch(const std::string& filename);

        /**
         * @brief ɾ���ļ�����
         * @param filename �ļ���
         */
        void command_delete(const std::string& filename);

        /**
         * @brief ��ȡ�ļ���������
         * @param filename �ļ���
         */
        void command_cat(const std::string& filename);

        // shell commmand for user
        void exit();

        void format();

        void help();

};