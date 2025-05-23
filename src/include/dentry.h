#include "fs.h"


class dentry
{
private:
    char* d_name;          // �ļ���
    inode* d_inode;         // ָ��Ŀ¼��� inode
    dentry* d_parent;         // ��(�������νṹ)
    std::list<dentry*> d_child;     // ��
    DFALG d_flag;             // ״̬��Ϣ
    time_t d_time;     // ���ɷ���ʱ��
    uint16_t d_ref;         // ���ü���
    //super_block d_sb;       // ָ���������ļ�ϵͳ
    //// ��ѡ�û����滻����(��lru)

    // �洢Ŀ¼�����ʹ�ù�ϣ��洢, ��߲���Ч��(Ҳ��˳���洢)
    std::unordered_map<std::string, dir_entry*> entries; // �洢Ŀ¼��

};