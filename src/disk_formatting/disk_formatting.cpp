#include <fs.h>

int main()
{

    dirTree* dir_tree_;     
    dentry* current_dir_;  

    blockScheduler* bs_; 


    dir_tree_ = new dirTree;

    bs_ = new blockScheduler(dir_tree_);

    dir_tree_->set_bs(bs_);

    // file_system_manager  fs;



    bs_->new_disk();
    bs_->freshUSER();
    bs_->writesuperblock();

    
    dir_entry par;
    vector<dir_entry>temp;
    dir_tree_->get_root()->getDir_entry(par,temp);
    cout<<temp.size()<<endl;
    bs_->writechild(par,temp,*dir_tree_->get_root()->get_inode(),temp.size());
}