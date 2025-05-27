#include "test.h"
#include <spdlog/spdlog.h>
void func()
{

    dirTree dirtree;


    blockScheduler bs(&dirtree);

    dirtree.set_bs(&bs);
    
    bs.new_disk();

    string name="test";
    inode*p=(inode*)malloc(sizeof(inode)*30);
    vector<dir_entry>px;
    dir_entry par;
    for(int i=0;i<30;i++)
    {
        string newname=name+to_string(i);
        dirtree.alloc_dir(newname, dirtree.get_root(),p+i, DIR);
        bs.creatFILE((size_t)i-1,*dirtree.get_root()->get_inode(),p[i]);
        char a[MAXNAMESIZE]={};
        newname.copy(a,newname.size());
    }
    dirtree.get_root()->getDir_entry(par,px);
    cout<<px.size();
    bs.writechild(par,px,*dirtree.get_root()->get_inode(),px.size());
    cout<<px.size();
}

// void func()
// {
//     // LRUReplacer dcache_replacer;
//     // LRUReplacer dentry_replacer;
//     dcache dcache;

//     dirTree dirtree;


//     blockScheduler bs(&dirtree);

//     dirtree.set_bs(&bs);
    
//     bs.new_disk();

//     string name="test";
//     inode*p=(inode*)malloc(sizeof(inode));

//     dirtree.alloc_dir(name, dirtree.get_root(),p, );
//     char*u=(char*)malloc(200*512*sizeof(char)+10);
//     char*z=(char*)malloc(200*512*sizeof(char)+10);
//     srand(time(NULL));
//     int KX=5;
//     for(int i=1;i<=513;i++)
//     {
//         for(int cont=0;cont<1;cont++)
//         {
//             for(int j=0;j<(i*KX)%73;j++)
//             {
//                 for(int m=0;m<512;m++)
//                 {
//                     u[j*512+m]='a'+rand()%26;
//                 }
//             }
//             int t=((i*KX)%73)*512-rand()%512;
//             u[t]=0;
//             bs.writeSIMfromBLOCK(*p,u);
//             z=bs.readSIMfromBLOCK(*p);
//             // cout<<(i*KX)%73<<" "<<strcmp(u,z)<<" "<<strlen(u)<<" "<<strlen(z)<<endl;
//             // cout<<511-bs.getfreeblocknum()<<" "<<bs.calSIMblockNUM((strlen(u)+511)/512)<<endl;
//             // cout<<"----------------"<<endl;
//             // cout<<u<<endl;
//             // cout<<"------------"<<endl;
//             // cout<<z<<endl;
//             // cout<<"------------"<<endl;
//         }
//     }
// }


/**
 * 全局目录项缓存键值对
 */
// struct dentryKey{
//     dentry* parent;
//     string name;

//     bool operator==(const dentryKey dk) const {
//         return parent == dk.parent && name == dk.name;
//     }
// };

// // 由于使用自定义的键, 所以得自定义一个hash函数
// struct dentryKeyHash{
//     size_t operator()(const dentryKey& key) const {
//         return hash<dentry*>()(key.parent) ^ hash<string>()(key.name);
//     }
// };

void func2()
{
    unordered_map<dentryKey, dentry*, dentryKeyHash> dentry_table_;
    string name = "test";
    inode* ino = nullptr;
    string name2 = "/";
    dentry* parent = new dentry(name2, ino,0, nullptr);
    auto it = dentry_table_.find({parent, name});
}