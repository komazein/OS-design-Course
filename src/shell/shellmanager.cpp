#include "../include/shellmanager.h"


/*
@brief ����Ŀ¼����


*/
void shell_manager::command_mkdir(const std::string& dirname) {
    if (current_dir_ == nullptr) {
        std::cout << "Error: Current directory is not set." << std::endl;
        return;
    }
    dir_tree_->mkdir(current_dir_, dirname);
}   