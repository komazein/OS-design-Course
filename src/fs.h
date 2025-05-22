#include <string>
#include <vector>
using namespace std;
#include <cstdint>

// 文件模块
namespace file{

    const int N_BLOCK = 12;         // inode中物理块索引





    enum FILETYPE
    {
        //...
    };

    // 文件控制信息
    enum FILEMODE
    {

    };

    // 时间类型
    struct __timestamp{
        // 上次修改
        // 创建时间
        // 最后一次存取时间
    };

    // // 文件句柄
    // typedef struct file_handler;


    // inode

    struct dinode
    {
        FILETYPE di_type;        // 文件类型
        FILEMODE di_mode;        // file mode
        uint8_t di_uid;           // owner_id
        uint8_t di_gid;           // group_id
        size_t di_size;          // file size      (byte)   
        __timestamp	di_atime;	/* Access time */
        __timestamp	di_ctime;	/* Inode Change time */
        __timestamp	di_mtime;	/* Modification time */
        __timestamp	di_dtime;	/* Deletion Time */
        uint32_t di_flag;         // file flag
        // uint16_t di_link_count;  // file link count
        void* i_block[N_BLOCK];        // 一级间接块、二级间接块、三级间接块(可拓展的混合索引模式)
    };


    

    // 文件打开表表项(描述文件当前状态), 常驻内存
    // open后创建并加入文件打开表
    struct file
    {
        dinode* f_inode;        // 指向打开文件对应的 inode
        FILEMODE di_mode;       // mode(eg: read only, can write)   权限
        const struct file_operations *f_op;     // 对文件操作函数表
        // uint16_t f_flag;        // 文件打开的状态
        string /*struct path*/ f_path;     // 文件路径
        uint16_t /*file_ref_t*/ f_ref;      // 引用计数
        size_t fd;     // 文件句柄 (后续仅通过此标识访问文件而非文件名)
        
    };
    
}


// 超级块
struct super_block{
    size_t s_block_size;         //块大小
    string s_fs_name;           // 文件系统名称(类型)
    size_t s_block_num;                   // 块数
};

// 磁盘格式
struct hard_disk
{

};  

// 块
struct block
{
    uint16_t mode;          // 块是索引块还是数据块
};


