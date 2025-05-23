# 文件系统相关结构体定义

## inode

记录文件的相关信息, 与文件一一对应.

### 磁盘inode

```cpp
struct dinode
{
    size_t di_num;          // inode号
    TYPE di_type;        // 文件类型
    FILEMODE di_mode;        // file mode
    uint8_t di_uid;           // owner_id
    uint8_t di_gid;           // group_id
    size_t di_size;          // file size      (byte, 可以便于定位几级索引)   
    __timestamp	di_atime;	/* Access time */
    __timestamp	di_ctime;	/* Inode Change time */
    __timestamp	di_mtime;	/* Modification time */
    __timestamp	di_dtime;	/* Deletion Time */
    uint32_t di_flag;         // file flag
    // uint16_t di_link_count;  // file link count
    size_t di_block[N_BLOCK];        // 存储逻辑块号, 直接,   一级间接块、二级间接块、三级间接块(可拓展的混合索引模式)
};
```

- 文件类型:

  以下三种: 普通文件, 目录, 软连接(指定的目标文件的路径)

  ```cpp
  // 文件类型
  enum TYPE
  {
      FILE,
      DIR,          // 那么这个inode中存储的就是其下的目录项(包含目录项名称, 以及他们对应的inode号)
      LINK
  };
  ```

- 时间:

  格式自定, 为日志时间的形式

- `diblock`:

  存储逻辑块号(用户视角看来是连续分配), 需要有地址映射机制将逻辑块号映射到物理块号从而取出数据.

  大小定义为12, 前9个是直接索引, 后面一次为若干级索引

- `filemode`:

  记录文件中的控制信息(自定义)

### 内存inode

较磁盘inode添加了引用计数, 方便实现硬链接

```cpp
struct inode
{
    size_t i_num;          // inode号
    TYPE i_type;        // 文件类型
    FILEMODE i_mode;        // file mode
    uint8_t i_uid;           // owner_id
    uint8_t i_gid;           // group_id
    size_t i_size;          // file size      (byte)   
    __timestamp	i_atime;	/* Access time */
    __timestamp	i_ctime;	/* Inode Change time */
    __timestamp	i_mtime;	/* Modification time */
    __timestamp	i_dtime;	/* Deletion Time */
    uint32_t i_flag;         // file flag
    uint16_t di_link_count;  // file link count(文件共享用, 硬链接)
    size_t i_block[N_BLOCK];        // 存储逻辑块号, 直接,   一级间接块、二级间接块、三级间接块(可拓展的混合索引模式)
};
```

## 文件打开表

文件打开表表项(描述文件当前状态), 常驻内存, open后创建并加入文件打开表

```cpp
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
```

用户对文件进行操作时记录相关信息, 动态的实例.

可以返回一个文件句柄便于用户操作, 打开, 关闭文件仅需要此句柄即可

## 超级块

```cpp 
class super_block{
private:
    size_t s_block_size;         //块大小
    string s_fs_name;           // 文件系统名称(类型)
    size_t s_block_num;                   // 块数
    inode* s_root;             // 指向根节点inode
    // size_t s_free_num;          // 空闲块数量
    /// 成组连接法相关数据结构

    
public:
    inode*    iget();             // 分配
    void            idel();             // 销毁 



};
```

记录文件系统的元信息, 一个文件系统对应一个超级块

## 目录表项

```cpp
// 单个目录表项
struct dir_entry
{
    size_t inode_num;       // inode号
    string name;            // 名称
    TYPE type;              // 类型(文件还是目录)
};
```

仅仅是维护inode号与文件名称的映射关系, 减少体量查找时便于减少I/O.

- `type`: 见上面的描述

## 目录

```cpp
struct dentry
{
    string d_name;          // 文件名
    inode* d_inode;         // 指向目录项的 inode
    dentry* d_parent;         // 父(构建树形结构)
    std::list<dentry*> d_child;     // 子
    DFALG d_flag;             // 状态信息
    __timestamp d_time;     // 若干访问时间
    uint16_t d_ref;         // 引用计数
    super_block d_sb;       // 指向所属的文件系统
    //// 可选用缓存替换策略(如lru)

    // 存储目录项可以使用哈希表存储, 提高查找效率(也可顺序表存储)
    std::unordered_map<std::string, dir_entry*> entries; // 存储目录项

};
```

- 记录树形结构, 通过路径查找相应的目录项, 找到inode号, 从而定位到inode, 找到文件
  - 其中的树形结构通过父子节点进行构建

- 为了减少I/O频率, 可以设置缓存, 并适配相应的替换策略方式内存占用过多和查找费力



  