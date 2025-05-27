#pragma once

#include "fs.h"

class file_manager {

    public:

    /**
     * @brief ���ļ�
     * @param filename �ļ���
     * @param current_dir ��ǰĿ¼��
     * open file and store the file descriptor in a structure
     * open func is used combined with close at any time
     */
    void openFile(const std::string& filename, dentry* current_dir);

    /**
     * @brief �ر��ļ�
     * @param filename �ļ���
     * @param current_dir ��ǰĿ¼��
     * close file and release the file descriptor
     */
    void closeFile(const std::string& filename, dentry* current_dir);
    
    /**
     * @brief ��ȡ�ļ�����
     * @param filename �ļ���
     * @param current_dir ��ǰĿ¼��
     * read file content and print it to the console
     * the main purpose of this function is to read the file content
     * and print it to the console, not to return the content
     */
    void readFile(const std::string& filename, dentry* current_dir);
    
    /** 
     * @brief д���ļ�����
     * @param filename �ļ���
     * @param content д�������
     * @param current_dir ��ǰĿ¼��
     * write file content to the file
     * if the file does not exist, create it
    */
    void writeFile(const std::string& filename, const std::string& content, dentry* current_dir);
    
    
    private:
        dirTree* dir_tree_;  // Ŀ¼��ָ��
        dentry* current_dir_;  // ��ǰĿ¼��

        

        /*struct file
        {
          inode* f_inode;        // ָ����ļ���Ӧ�� inode
          FILEMODE di_mode;       // mode(Ȩ��)
          TYPE f_type;
          string f_path;          // �ļ�·��
          uint16_t f_ref;         // ���ü���
          size_t fd;              // �ļ����
        };
        *
        */
        std::vector<file> open_files_;  // the list of open files

};