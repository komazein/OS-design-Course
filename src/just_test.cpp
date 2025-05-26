#include "test.h"
#include <spdlog/spdlog.h>

void func()
{
    LRUReplacer replacer;
    dcache dcache;

    dirTree dirtree(&replacer, &dcache);


    blockScheduler bs(&dirtree);

    dirtree.set_bs(&bs);
    
    bs.new_disk();



    string name="test";
    inode*p=(inode*)malloc(sizeof(inode));

    

    dirtree.alloc_dir(name, dirtree.get_root(),p);
    cout<<p->i_size<<endl;
    

    int N=30;
    char*u=(char*)malloc(N*512*sizeof(char)+10);
    char*z=(char*)malloc(N*512*sizeof(char)+10);
    for(int i=0;i<N;i++)
    {
        for(int j=0;j<512;j++)
        {
            if(i<26)
            {
                u[i*512+j]='A'+i;
            }
            else
            {
                u[i*512+j]='a'+j;
            }
        }
    }
    u[N*512-100]=0;
    bs.writeSIMfromBLOCK(*p,u);
    z=bs.readSIMfromBLOCK(*p);
    printf("%s",z);
}


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