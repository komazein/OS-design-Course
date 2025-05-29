#include "test.h"
#include <spdlog/spdlog.h>


/**
 * 
 * @brief mkdir testI  (I: 1~N)
 */
// PASS!
void CREATE_MULTIPLE_DIRS()
{

    file_system_manager fs;
    for(int i=0;i<100;i++)
    {
        string testdir_name="test"+to_string(i);
        fs.command_mkdir(testdir_name);
    }

}

/**
 * 
 * @brief   mkdir testI
 *          cd testI
 *          mkdir test(I+1)
 *          cd test(I+1)
 */
// 
void CREATE_MULTIPLE_DIRS_RECURSIVELY()
{
    file_system_manager fs;
    for(int i = 0; i < 513; i++){
        string test_name = "test" + to_string(i);
        fs.command_mkdir(test_name);
        fs.command_cd(test_name);
    }

}
void TESTWRITEBACK()
{
    file_system_manager fs;
    for(int i = 0; i < 10; i++){
        string test_name = "test" + to_string(i);
        fs.command_mkdir(test_name);
    }
    string next_root="test0";
    fs.command_cd(next_root);
    for(int i = 20; i < 30; i++){
        string test_name = "test" + to_string(i);
        fs.command_mkdir(test_name);
    }
    next_root="test25";
    fs.command_cd(next_root);
    for(int i = 20; i < 30; i++){
        string test_name = "test" + to_string(i);
        fs.command_mkdir(test_name);
    }
    fs.Exit();
}



void TESTWRITEBACK2()
{
    file_system_manager fs;
    string next_root="test0";
    fs.command_cd(next_root);
    next_root="test20";
    fs.command_cd(next_root);
    string test_name="test10086";
    fs.command_mkdir(test_name);
    fs.Exit();
}

void TESTWRITEBACK3()
{
    file_system_manager fs;
    string next_root="test0";
    fs.command_cd(next_root);
    string test_name="test10086";
    fs.command_mkdir(test_name);
    fs.command_cd(test_name);
    fs.command_mkdir(test_name);
    // string test_name="test10086";
    // //fs.command_mkdir(test_name);
    // next_root=test_name;
    // fs.command_cd(next_root);
    // fs.command_mkdir(test_name);
    fs.Exit();
}
void TESTLS()
{
    file_system_manager fs;
    string next_root="test0";
    fs.command_cd(next_root);
    fs.command_ls();
}

void TESTDELETE()
{
    TESTWRITEBACK();
    file_system_manager fs;
    string del_root="test0";
    fs.command_delete(del_root);
    fs.Exit();
}
/*
void func()
{
    // LRUReplacer dcache_replacer;
    // LRUReplacer dentry_replacer;
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
    int N;
    cin>>N;
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
    
    bs.writesuperblock();
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

// void func2()
// {
//     unordered_map<dentryKey, dentry*, dentryKeyHash> dentry_table_;
//     string name = "test";
//     inode* ino = nullptr;
//     string name2 = "/";
//     dentry* parent = new dentry(name2, ino,0, nullptr);
//     auto it = dentry_table_.find({parent, name});
// }

// int yylex();
// void testFuncMkdir()
// {
//     extend file_system_manager fs;

//     while (1)
//     {
//         printf("[");
        
//         printf("]$ ");
//         yylex();
//     }
    
// }