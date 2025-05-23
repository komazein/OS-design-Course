#include <string>
#include <vector>
using namespace std;
#include <cstdint>
#include <unordered_map>
#include <list>



const int INODE_SIZE = sizeof(struct inode);
const int N_BLOCK = 12;         // inode中物理块索引
const int BLOCKSIZE = 512;
const int USERNUM = 8;
const int DATANUM = 512;
const int INODENUM = 32;
const int S_FREE_NUM = 16;         // 空闲块数量
const int MAXNAMESIZE = 16;


// 文件类型
enum TYPE
{
    FILE,
    DIR,          // 那么这个inode中存储的就是其下的目录项(包含目录项名称, 以及他们对应的inode号)
    LINK
};


// 文件控制信息
enum FILEMODE
{

};


// // 文件句柄
// typedef struct file_handler;


// inode

struct dinode
{
    size_t di_num;          // inode号
    TYPE di_type;        // 文件类型
    FILEMODE di_mode;        // file mode
    uint8_t di_uid;           // owner_id
    uint8_t di_gid;           // group_id
    size_t di_size;          // file size      (byte, 可以便于定位几级索引)   
    time_t	di_atime;	/* Access time */
    time_t	di_ctime;	/* Inode Change time */
    time_t	di_mtime;	/* Modification time */
    time_t	di_dtime;	/* Deletion Time */
    uint32_t di_flag;         // file flag
    // uint16_t di_link_count;  // file link count
    size_t di_block[N_BLOCK];        // 存储逻辑块号, 直接,   一级间接块、二级间接块、三级间接块(可拓展的混合索引模式)
};




// 内存inode, 只有文件创建后才在内存中创建inode
struct inode
{
    size_t i_num;          // inode号
    TYPE i_type;        // 文件类型
    FILEMODE i_mode;        // file mode
    uint8_t i_uid;           // owner_id
    uint8_t i_gid;           // group_id
    size_t i_size;          // file size      (byte)   
    time_t	i_atime;	/* Access time */
    time_t	i_ctime;	/* Inode Change time */
    time_t	i_mtime;	/* Modification time */
    time_t	i_dtime;	/* Deletion Time */
    uint32_t i_flag;         // file flag
    uint16_t di_link_count;  // file link count(文件共享用, 硬链接)
    size_t i_block[N_BLOCK];        // 存储逻辑块号, 直接,   一级间接块、二级间接块、三级间接块(可拓展的混合索引模式)
};


// class inode
// {
//     private:
//         //...
//     public:
//         // 创建硬链接, 创建新目录
// };





// 文件打开表表项(描述文件当前状态), 常驻内存
// open后创建并加入文件打开表
struct file
{
    inode* f_inode;        // 指向打开文件对应的 inode
    FILEMODE di_mode;       // mode(eg: read only, can write)   权限
    TYPE f_type;            // 也许没啥用(都是文件类型)
    //const struct file_operations *f_op;     // 对文件操作函数表
    // uint16_t f_flag;        // 文件打开的状态
    string /*struct path*/ f_path;     // 文件路径
    uint16_t /*file_ref_t*/ f_ref;      // 引用计数
    size_t fd;     // 文件句柄 (后续仅通过此标识访问文件而非文件名)
};




// 超级块, 对应一个(具体的)文件系统
// struct super_block{
//     size_t s_block_size;         //块大小
//     string s_fs_name;           // 文件系统名称(类型)
//     size_t s_block_num;                   // 块数
// };




// // 磁盘格式
// struct hard_disk
// {

// };  



// 数据块类型
enum BTYPE
{
    INDEX,      // 索引块
    DATA        // 数据块
};

// 数据块
struct block
{
    BTYPE mode;          // 块是索引块还是数据块

};

// // 虚拟文件系统
// class VFS
// {
// private:
//     super_block sb;     // 超级快
//     inode* in[INODE_SIZE];       // inode
// };

// access control访问控制
// 保护单个文件和目录内文件
struct ACL
{
    // 规定从0-3位的访问类型: 如 0000-读, 0010: 写,  
    uint8_t owner;      
    uint8_t group;  
    uint8_t other;
};


/////////////////////////////////////////////
// 目录
/////////////////////////////////////////////

// 单个目录表项
struct dir_entry
{
    size_t inode_num;       // inode号
    char name[MAXNAMESIZE];            // 名称
    TYPE type;              // 类型(文件还是目录)
};


// 目录的本质也是文件, 存储文件名<--->inode的映射关系

// 如果一个目录的结构如下:
// src---
//      --- a.txt
//      --- src2
// | inode_num | name  | type   |
// |-----------|-------|--------|
// | 100       | a.txt | file   |
// | 200       | src2  | dir    |


// 目录项的状态
union DFALG
{
    //..可以是在缓存中的状态(如目录项已删除, 但保留在缓存中)
};


// (可选)多级目录(树形)结构
// 内存项缓存(缓存树形目录)
// 自选存储的数据结构(以查找速率为原则设计)
// 启动文件系统时, 从根('/')开始构建
// 缓存路径, 便于下面的查找
/*struct dentry
{
    string d_name;          // 文件名
    inode* d_inode;         // 指向目录项的 inode
    dentry* d_parent;         // 父(构建树形结构)
    std::list<dentry*> d_child;     // 子
    DFALG d_flag;             // 状态信息
    time_t d_time;     // 若干访问时间
    uint16_t d_ref;         // 引用计数
    super_block d_sb;       // 指向所属的文件系统
    //// 可选用缓存替换策略(如lru)

    // 存储目录项可以使用哈希表存储, 提高查找效率(也可顺序表存储)
    std::unordered_map<std::string, dir_entry*> entries; // 存储目录项

};
*/



