#pragma once
#include "filemanager.h"
#include "tool.h"
#include "dentry.h"
#include "ddq.h"

class file_system_manager{

    private:
        dirTree* dir_tree_;     
        dentry* current_dir_;  
        file_manager * file_manager_;
        blockScheduler* bs_; 
    
    public:
        file_system_manager() {
            dir_tree_ = new dirTree;

            bs_ = new blockScheduler(dir_tree_);

            dir_tree_->set_bs(bs_);

            bs_->new_disk();

            file_manager_ = new file_manager(dir_tree_, dir_tree_->get_root());
        }
        // execute shell command for dir only

        /** 
        * @brief ����Ŀ¼����
        * @param dirname Ŀ¼����
        */
        void command_mkdir(std::string& dirname);

        /**
         * @brief �л���ǰĿ¼����
         * @param dirname Ŀ¼����
         */
        void command_cd(std::string& dirname);
        /**
         * @brief �г���ǰĿ¼�µ������ļ�����Ŀ¼
         */
        void command_ls();
  
        /**
         * @brief ɾ��Ŀ¼����
         * @param dirname Ŀ¼����
         */
        void command_rm(std::string& dirname);

        /**
         * @brief ����������������
         * @param dirname Ŀ���ļ���Ŀ¼
         */
        void command_lkdir(std::string& source_path, std::string& target_path);

        /**
         * @brief ����Ӳ��������
         * @param dirname Ŀ���ļ���Ŀ¼
         */
        void command_lndir(std::string& source_path, std::string& target_path);

        /**
         * @brief �����ļ�����
         * @param filename �ļ���
         */
        void command_find(std::string& filename);

        // execute shell command for file only
        /**
         * @brief �����ļ�����
         * @param filename �ļ���
         */
        void command_touch(std::string& filename);

        /**
         * @brief ɾ���ļ�����
         * @param filename �ļ���
         */
        void command_delete(std::string& filename);

        /**
         * @brief ��ȡ�ļ���������
         * @param filename �ļ���
         */
        void command_cat(std::string& filename);

        // shell commmand for user
        void exit();

        void format();

        void help();

};