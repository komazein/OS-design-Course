#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <list>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string.h>
#include<cmath>
#include<queue>
#include <optional>
using namespace std;

extern const int INODE_SIZE;
extern const int SUPERBLOCKSIZE;

// const int INODE_SIZE = sizeof(struct inode);
const int N_BLOCK = 12;         // inode中物理块索引,inode索引规则
const int BLOCKSIZE = 512;
const int USERNUM = 8;
const int DATANUM = 512;
const int INODENUM = 32;
const int S_FREE_NUM = 16;         // 空闲块数量
const int MAXNAMESIZE = 16;
const int MAXnumInBlock = 3;//限制每个磁盘最多目录项个数(包含父节点的目录项),必须大于等于3
const int ROOT_INODE_NUMBER = 0;        // 初始时根节点的inode号为0
const int ONLY_DIRECT_SIM = 9;
const int ONLY_ONE_SIM=2;
const int ONLY_MULTI_SIM=1;//不能变
const int ABLE_DIRECT_SIM = 12;
const int ABLE_ONE_SIM=3;
const int ABLE_MULTI_SIM=1;//不能变

const string FILE_MODE_INIT= "-rwxr--r--";
const string DIR_MODE_INIT = "drwxr--r--";

// 文件类型
enum TYPE
{
    SIM_FILE,
    DIR,          // 那么这个inode中存储的就是其下的目录项(包含目录项名称, 以及他们对应的inode号)
    LINK
};

// 文件控制信息
// symbolic notification of file mode
// ---------- 10 bit 
struct FILEMODE
{
    // - for nomal file, d for dir, l for soft link, 
    char file_type; // file type, 1 bit 
    char ownerMode[4]; // owner mode, 3bit
    char groupMode[4]; // group mode, 3bit
    char otherMode[4]; // other mode, 3bit
    //for all above, r for read, w for write, x for execute, - for no permission
};

// inode磁盘结构
struct dinode
{
    size_t di_num;          // inode号
    TYPE di_type;        // 文件类型
    FILEMODE di_mode;        // file mode
    uint8_t di_uid;           // owner_id
    uint8_t di_gid;           // group_id
    size_t di_size;          // file size (byte)
    time_t	di_atime;	/* Access time */
    time_t	di_ctime;	/* Inode Change time */
    time_t	di_mtime;	/* Modification time */
    uint32_t di_flag;         // file flag
    size_t di_block[N_BLOCK]; // 存储逻辑块号
};

// 内存inode
struct inode
{
    size_t i_num;          // inode号
    TYPE i_type;        // 文件类型
    FILEMODE i_mode;        // file mode
    uint8_t i_uid;           // owner_id
    uint8_t i_gid;           // group_id
    size_t i_size;          // file size
    time_t	i_atime;
    time_t	i_ctime;
    time_t	i_mtime;
    uint32_t i_flag;         // file flag
    uint16_t di_link_count { 0 };  // file link count(硬链接)
    size_t i_block[N_BLOCK]; // 块地址

};

// 文件句柄结构
struct file
{
    inode* f_inode;        // 指向打开文件对应的 inode
    FILEMODE di_mode;       // mode(权限)
    TYPE f_type;
    string f_path;          // 文件路径
    uint16_t f_ref;         // 引用计数
    size_t fd;              // 文件句柄
};

// 数据块类型
enum BTYPE
{
    INDEX,
    DATA
};

// 数据块
struct block
{
    BTYPE mode;          // 索引块或数据块
};

// 访问控制结构
struct ACL
{
    uint8_t owner;
    uint8_t group;
    uint8_t other;
};

// 目录项结构
struct dir_entry
{
    size_t inode_num;       // inode号
    char name[MAXNAMESIZE]; // 名称
    TYPE type;              // 类型(文件/目录)
};

// 目录项的状态
enum DFALG
{
    //..可以是在缓存中的状态(如目录项已删除, 但保留在缓存中)
    FIRST_LOAD_TO_MEMORY,       // 首次加载入内存, 
                //由于加载入内存中已经把他的子目录项中的内容初步分配并初始化为dentry节点链入树中, 所以保留了其子目录结构
    CUT_SUBDIRS,                // 此时它的子目录被剪枝(说明如果发现此时无子目录项, 需要再次遍历此dentry节点时, 还需要进行磁盘I/O)


};


