#pragma once
#include "tool.h"
#include "dentry.h"
#include "ddq.h"
#include "filemanager.h"
#include "user.h"


struct FileItem {
    string name;
    int type; 
};
class user;
class dirTree;


// class file_system_manager{

//     private:
//         dirTree* dir_tree_;     
//         dentry* current_dir_;  
//         // file_manager * file_manager_;
//         blockScheduler* bs_; 
    
//     public:
//         file_system_manager() {

//             dir_tree_ = new dirTree;

//             bs_ = new blockScheduler(dir_tree_);

//             dir_tree_->set_bs(bs_);

//             bs_->load(dir_tree_);
//             current_dir_ = dir_tree_->get_root();

//             // file_manager_ = new file_manager(dir_tree_, dir_tree_->get_root());
//         }
//         // execute shell command for dir only

//         /** 
//         * @brief ����Ŀ¼����
//         * @param dirname Ŀ¼����
//         */
//         void command_mkdir(std::string& dirname);

//         /**
//          * @brief �л���ǰĿ¼����
//          * @param dirname Ŀ¼����
//          */
//         void command_cd(std::string& dirname);
//         /**
//          * @brief �г���ǰĿ¼�µ������ļ�����Ŀ¼
//          */
//         void command_ls();
  
//         /**
//          * @brief ɾ��Ŀ¼����
//          * @param dirname Ŀ¼����
//          */
//         void command_rm(std::string& dirname);

//         /**
//          * @brief ����������������
//          * @param dirname Ŀ���ļ���Ŀ¼
//          */
//         void command_lkdir(std::string& dirname);

//         /**
//          * @brief ����Ӳ��������
//          * @param dirname Ŀ���ļ���Ŀ¼
//          */
//         void command_lndir(std::string& dirname);

//         /**
//          * @brief �����ļ�����
//          * @param filename �ļ���
//          */
//         void command_find(std::string& filename, bool fuzzy);

//         // execute shell command for file only
//         /**
//          * @brief �����ļ�����
//          * @param filename �ļ���
//          */
//         void command_touch(std::string& filename);

//         /**
//          * @brief ɾ���ļ�����
//          * @param filename �ļ���
//          */
//         void command_delete(std::string& filename);

//         /**
//          * @brief ��ȡ�ļ���������
//          * @param filename �ļ���
//          */
//         void command_cat(std::string& filename);

//         // shell commmand for user
//         void Exit();

//         void format();

//         void help();

// };


class file_system_manager{

    private:
        dirTree* dir_tree_;     
        dentry* current_dir_;  
        file_manager * file_manager_;
        blockScheduler* bs_; 
        user* logInUser_;
    
    public:
        file_system_manager() {
            dir_tree_ = new dirTree;

            bs_ = new blockScheduler(dir_tree_);
            dir_tree_->set_bs(bs_);

            logInUser_= new user(dir_tree_);
            dir_tree_->set_user(logInUser_);
         
            bs_->load(dir_tree_);
            current_dir_ = dir_tree_->get_root();
            file_manager_ = new file_manager(dir_tree_, current_dir_);
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
        void command_find(std::string& filename, bool fuzzy);

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

        /**
         * @brief write content to file
         * @param filename file name
         * @param content content to write
         */
        void command_edit(std::string& filename, std::string& content);

        void command_athrz(std::string& filename, int uid);

        void command_rathrz(std::string& filename, int uid);




        dentry* getCurrentDir()  { return current_dir_; }

        blockScheduler* get_bs() { return bs_; }

        // shell commmand for user
        void Exit();

        void format();

        void help();

        bool CMPuser(string username_,string password_);

        int get_uid() { return logInUser_->getUid(); }





        /////////////////////////////////////////////
        /////////////NEW ADD/////////////////////////
        string get_cur_dir() {
            string cur_dir;
            if(current_dir_==dir_tree_->get_root()){
                return current_dir_->get_name();
            }
            dir_tree_->get_full_path(cur_dir, dir_tree_->get_root(), current_dir_);
            return cur_dir;
         }

        // 返回vector<string>给前端
        void get_ls(vector<FileItem>& ret){
            if (current_dir_ == nullptr) {
                Exit();
            }
            dir_tree_->has_child_test(current_dir_);                // 首先检查一下是否有子
            for (const auto& pair : current_dir_->get_subdir()) {
                std::string subdir_name = pair.first;
                dentry* subdir = pair.second;
                TYPE type = subdir->get_type();
                ret.push_back({subdir_name, type + 1});
            }
        }
        string get_root_name(){
            return dir_tree_->get_root()->get_name();
        }

};