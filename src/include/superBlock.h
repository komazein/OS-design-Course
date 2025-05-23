#include "fs.h";
#include "dentry.h"
const int SUPERBLOCKSIZE = sizeof(class super_block);

class super_block{
private:
    size_t s_block_size;         //���С
    size_t s_block_num;                   // ����
    dentry* s_root;             // ָ����ڵ�inode
    int s_free_num[S_FREE_NUM]; // ����Block manange
    // size_t s_free_num;          // ���п�����
    /// �������ӷ�������ݽṹ

    
public:
    inode*    iget();             // ����
    void      idel();             // ���� 
    void      init();             // ��ʼ��

};