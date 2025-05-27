#include "test.h"
#include <spdlog/spdlog.h>

void func()
{
    // LRUReplacer dcache_replacer;
    // LRUReplacer dentry_replacer;
    dcache dcache;

    dirTree dirtree;


    blockScheduler bs(&dirtree);

    dirtree.set_bs(&bs);
    
    bs.new_disk();



    string name="test";
    inode*p=(inode*)malloc(sizeof(inode)*40);
    vector<dir_entry>px;
    dir_entry par;
    int N=19;
    for(int i=0;i<N;i++)
    {
        string newname=name+to_string(i);
        dirtree.alloc_dir(newname, dirtree.get_root(),p+i, DIR);
        bs.creatFILE((size_t)i,*dirtree.get_root()->get_inode(),p[i]);
        char a[MAXNAMESIZE]={};
        newname.copy(a,newname.size());
    }
    dirtree.get_root()->getDir_entry(par,px);
    cout<<px.size()<<endl;
    bs.writechild(par,px,*dirtree.get_root()->get_inode(),px.size());
    vector<dir_entry>pz;
    bs.loadchild(pz,*dirtree.get_root()->get_inode());
    cout<<pz.size()<<endl;
    for(int i=0;i<pz.size();i++)
         cout<<pz[i].inode_num<<" "<<pz[i].name<<endl;
    vector<size_t>block_num;
    bs.getallBlockDIR(*dirtree.get_root()->get_inode(),pz.size(),block_num);
    cout<<"n"<<block_num.size()<<endl;
    for(int i=0;i<block_num.size();i++)
    {
        cout<<block_num[i]<<" ";
    }
    cout<<endl;
    string a="test0";
    vector<size_t>tempa;
    bs.getallBlockDIR(p[0],0,tempa);
    for(int i=0;i<tempa.size();i++)
        cout<<tempa[0]<<" ";
    cout<<endl;
    cout<<"item"<<dirtree.get_root()->get_subdir().size()<<endl;
    auto subnode = dirtree.name_travesal(a,dirtree.get_root());
    cout << subnode->get_name() << "\n";
    dirtree.free_dir(a,dirtree.get_root());
    cout<<"item"<<dirtree.get_root()->get_subdir().size()<<endl;

    vector<size_t>block_num_new;
    bs.getallBlockDIR(*dirtree.get_root()->get_inode(),dirtree.get_root()->get_subdir().size(),block_num_new);
    for(int i=0;i<block_num_new.size();i++)
        cout<<block_num_new[i]<<" ";
    cout<<endl;

    vector<dir_entry>py;
    dirtree.get_root()->getDir_entry(par,py);
    bs.writechild(par,py,*dirtree.get_root()->get_inode(),py.size());

    vector<dir_entry>pu;
    bs.loadchild(pu,*dirtree.get_root()->get_inode());
    cout<<pu.size()<<endl;
    for(int i=0;i<pu.size();i++)
         cout<<pu[i].inode_num<<" "<<pu[i].name<<endl;
}