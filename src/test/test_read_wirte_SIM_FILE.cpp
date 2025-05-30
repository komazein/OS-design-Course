// #include "test.h"
// #include <spdlog/spdlog.h>
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


