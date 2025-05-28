#include <fs.h>

int main()
{

    dirTree* dir_tree_;     
    dentry* current_dir_;  

    blockScheduler* bs_; 


    dir_tree_ = new dirTree;

    bs_ = new blockScheduler(dir_tree_);

    dir_tree_->set_bs(bs_);

    bs_->new_disk();
    bs_->writesuperblock();
    bs_->ReWrinode(*dir_tree_->get_root()->get_inode(),false);
    dir_entry par;
    vector<dir_entry>temp;
    dir_tree_->get_root()->getDir_entry(par,temp);
    bs_->writechild(par,temp,*dir_tree_->get_root()->get_inode(),temp.size());
}