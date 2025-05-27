
#include "fs.h";

class shell_manager{

    private:
        fs* file_system_;  // �ļ�ϵͳָ��
        dentry* current_dir_;  // ��ǰĿ¼��
        
    
    public:
        shell_manager(fs* file_system) :file_system_(file_system), current_dir_(nullptr) {
            // ��ʼ����ǰĿ¼Ϊ��Ŀ¼
            current_dir_ = file_system_->get_root_dentry();
        }

        // execute shell command for dir only
        void command_mkdir(const std::string& dirname);

        void command_cd(const std::string& dirname);

        void command_ls();

        void command_rm(const std::string& dirname);

        void command_lkdir(const std::string& dirname);

        void command_lndir(const std::string& dirname);

        void command_find(const std::string& filename);

        // execute shell command for file only
        void command_touch(const std::string& filename);

        void command_delete(const std::string& filename);

        void command_cat(const std::string& filename);

        // shell commmand for user
        void exit();

        void format();

        void help();


        
};