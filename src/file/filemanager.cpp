
#include "filemanager.h"
#include <spdlog/spdlog.h>
#include "log_global.h"
#include "tool.h"
void file_manager::openFile(std::string& filename, dentry* current_dir)
{
    
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot open file '{}'", filename);
        return;
    }

    // finding the file in the current directroy
    dir_tree_->has_child_test(current_dir);
    dentry* file_entry = current_dir->find_subdir(filename);
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        g_logger->warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }
    // check if the file is already open
    for (const auto& f : open_files_) {
        if (f.f_path == file_entry->get_name()) {
            spdlog::warn("File '{}' is already open", filename);
            return;
        }
    }
    
    // create a new file structure to represent the opened file 
    file new_file;
    if(file_entry->get_inode()==nullptr)
    {
        inode*tempinode=(inode*)malloc(sizeof(inode));
        tempinode->i_num=file_entry->get_inode_num();
        dir_tree_->get_bs()->ReWrinode(*tempinode,true);
        file_entry->set_inode(tempinode);
    }
    new_file.f_inode = file_entry->get_inode();
    new_file.f_type = SIM_FILE;  // type is file 
    new_file.f_path = file_entry->get_name(); // path is the name of the file
    new_file.f_ref = 1;  // open the file, so reference count is 1
    //fd starts from 0 and is incremented for each new file opened
    new_file.fd = open_files_.size();  // fd is the index in the open_files_ vector
    new_file.di_mode = file_entry->get_inode()->i_mode; // mode is the inode's mode
    
    open_files_.push_back(new_file);
    spdlog::info("File '{}' opened successfully", filename);
}

void file_manager::closeFile(std::string& filename, dentry* current_dir)
{
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot close file '{}'", filename);
        return;
    }

    // get the file in the current directroy
    dentry* file_entry = current_dir->find_subdir(filename);
    // close file which is not exist
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }

    // check the file in the open files list
    for (auto it = open_files_.begin(); it != open_files_.end(); ++it) {
        if (it->f_path == file_entry->get_name()) {
            // decrease reference count
            /*
            according to the file struct's definition,  
            close file will not immdeiatiely remove the file from the open files list,
            but just decrease the reference count, and will remove the file from the open files 
            untile the reference count reaches 0.
            */
            if (--it->f_ref == 0) {
                open_files_.erase(it);  // remove the file from the open files list
                spdlog::info("File '{}' closed successfully", filename);
            } else {
                spdlog::info("File '{}' reference count decreased to {}", filename, it->f_ref);
            }
            return;
        }
    }

    spdlog::info("File '{}' is not currently open", filename);
}

//TODO: implement readFile
void file_manager::readFile(std::string& filename, dentry* current_dir)
{
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot read file '{}'", filename);
        return;
    }

    // get the file in the current directroy
    dentry* file_entry = current_dir->find_subdir(filename);
    // file not exist
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }

    // check if the file is open
    for (auto& f : open_files_) {
        if (f.f_path == file_entry->get_name()) {
            // read the file content and print it to the console
            // here we just print a message, in real implementation, we would read the content from disk
            cout<<f.f_inode->i_size<<endl;
            if(f.di_mode.ownerMode[0] != 'r')
            {
                cout<< f.di_mode.ownerMode<<endl;
                spdlog::warn("File '{}' is not readable", filename);
                return;
            }
            auto bs = dir_tree_->get_bs();
            char* fileContents = bs->readSIMfromBLOCK(*f.f_inode);
            g_log_buffer.push_back(std::string(fileContents));
            printf("Reading file '%s': %s\n", filename.c_str(), fileContents);
            spdlog::info("Reading file '{}': [Content would be displayed here]", filename);
            return;
        }
    }

    spdlog::warn("File '{}' is not currently open", filename);
}

//TODO: implement writeFile
void file_manager::writeFile(std::string& filename, std::string& content, dentry* current_dir)
{
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot write to file '{}'", filename);
        return;
    }

    // get the file in the current directroy
    dentry* file_entry = current_dir->find_subdir(filename);
    // file not exist
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }
    // check if the file is open
    for (auto& f : open_files_) {
        if (f.f_path == file_entry->get_name()) {
            // write the content to the file
            // here we just print a message, in real implementation, we would write the content to disk
            if(f.di_mode.ownerMode[1] != 'w')
            {
                spdlog::warn("File '{}' is not writable", filename);
                return;
            }
            auto bs = dir_tree_->get_bs();
            if(!bs->writeSIMfromBLOCK(*f.f_inode, strdup(content.c_str()))) {
                spdlog::error("Failed to write to file '{}': insufficient disk space or other error", filename);
                return;
            }
            file_entry->set_dirty(true);
            spdlog::info("Writing to file '{}': [Content: {}]", filename, content);
            return;
        }
    }

    spdlog::warn("File '{}' is not currently open", filename);
}

void file_manager::executeAssembly(std::string& code)
{
    unordered_map<char, int> registers = {{'A', 0}, {'B', 0}, {'C', 0}, {'D', 0}};
    vector<string> instructions;
    stringstream ss(code);
    string token;

    
    while (getline(ss, token, ';')) {
        if (!token.empty()) {
            instructions.push_back(token);
        }
    }

    // 执行每条指令
    for (const string& instr : instructions) {
        stringstream iss(instr);
        cout << instr << endl;
        string op;
        char dst;
        string src;
     
        iss >> op; // 读取操作码

        if (op == "ADD" || op == "SUB" || op == "MUT" || op == "DIV" || op == "ST") {
            iss >> dst >> src;
            cout << dst <<" "<<src<<endl;
            int num=0;
            if(src[0]>='A'&&src[0]<='D'&&src.size()==1)
                num=registers[src[0]];
            else
            {
                int start=(src[0]=='-');
                for(int i=start;i<src.size();i++)
                {
                    if(src[i]<='9'&&src[i]>='0')
                        num=10*num+src[i]-'0';
                    else
                    {
                        string a="ILLGEAL INPUT";
                        g_log_color_buffer.push_back(std::make_pair(a,Color::White));
                        return ;
                    }
                }
                if(start)
                    num*=-1;
            }
            if (op == "ADD") registers[dst] += num;
            else if (op == "SUB") registers[dst] -= num;
            else if (op == "MUT") registers[dst] *= num;
            else if (op == "DIV"){
                if(num == 0)
                {
                    string a="DIV_ZERO ";
                    g_log_color_buffer.push_back(std::make_pair(a,Color::White));
                    return ;
                }
                registers[dst] /= num;
            }
            else if (op == "ST" ) registers[dst] = num;
        } 
        else if (op == "RET") {
            iss >> dst; 
            cout << "The result of assembly code execution is: " << registers[dst] << endl;
            string a="The result of assembly code execution is: "+to_string(registers[dst]);
            g_log_color_buffer.push_back(std::make_pair(a,Color::White));
        }
        else
        {
            string a="ILLGEAL INPUT";
            g_log_color_buffer.push_back(std::make_pair(a,Color::White));
            return ;
        }
    }


}

void file_manager::executeFile(std::string& filename, dentry* current_dir)
{
    if (current_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot execute file '{}'", filename);
        return;
    }

    // get the file in the current directroy
    dentry* file_entry = current_dir->find_subdir(filename);
    // file not exist
    if (file_entry == nullptr) {
        spdlog::warn("File '{}' not found in current directory '{}'", filename, current_dir->get_name());
        return;
    }

    // check if the file is open
    for (auto& f : open_files_) {
        if (f.f_path == file_entry->get_name()) {
            // execute the file
            // here we just print a message, in real implementation, we would execute the file
            if(f.di_mode.ownerMode[2] != 'x')
            {
                spdlog::warn("File '{}' is not executable", filename);
                return;
            }
            auto bs = dir_tree_->get_bs();
            char* assemblyCode = bs->readSIMfromBLOCK(*f.f_inode);
            if (assemblyCode == nullptr) {
                spdlog::error("Failed to read assembly code from file '{}'", filename);
                return;
            }
            std::string code(assemblyCode);
            executeAssembly(code); // execute the assembly code
            spdlog::info("Executing file '{}': [Execution would be performed here]", filename);
            return;
        }
    }

    spdlog::warn("File '{}' is not currently open", filename);
}
void file_manager::fileHardLink(std::string& source_path, std::string& target_path, int cur_uid, dentry* work_dir)
{
    if (work_dir == nullptr) {
        spdlog::warn("Current directory is not set. Cannot create hard link for file path '{}'", source_path);
        return;
    }
    std::string filename_source;
    // std::string parent_dir_path_source = source_path.substr(0, source_path.find_last_of('/'));
    size_t last_slash_source = source_path.find_last_of('/');
    std::string parent_dir_path_source;
    dentry* old_SIM_FILE;
    dentry* parent_dir;
    if (last_slash_source != std::string::npos) {
        parent_dir_path_source = source_path.substr(0, last_slash_source);
        filename_source = source_path.substr(source_path.find_last_of('/') + 1);
        // get the file in the current directroy
        dentry* file_entry = dir_tree_->name_travesal(parent_dir_path_source, work_dir);
        // file not exist
        if (file_entry == nullptr) {
            spdlog::warn("File path '{}' not found in current directory '{}'", parent_dir_path_source, work_dir->get_name());
            g_logger->warn("File path '{}' not found in current directory '{}'", parent_dir_path_source, work_dir->get_name());
            return;
        }
        dir_tree_->has_child_test(file_entry);
        old_SIM_FILE=file_entry->get_subdir()[filename_source];
        if(old_SIM_FILE==nullptr)
        {
            spdlog::warn("File '{}' not found in current directory '{}'", filename_source, parent_dir_path_source);
            g_logger->warn("File '{}' not found in current directory '{}'", filename_source, parent_dir_path_source);
            return;
        }
        
    } else {
        filename_source = source_path;             // 直接是文件名

        dir_tree_->has_child_test(work_dir);
        old_SIM_FILE = work_dir->get_subdir()[source_path];

        if(old_SIM_FILE==nullptr)
        {
            spdlog::warn("File '{}' not found in current directory '{}'", filename_source, work_dir->get_name());
            g_logger->warn("File '{}' not found in current directory '{}'", filename_source, work_dir->get_name());
            return;
        }
    }
    if(old_SIM_FILE->get_inode()==nullptr)
    {
        inode*temp=(inode*)malloc(sizeof(inode));
        temp->i_num=old_SIM_FILE->get_inode_num();
        old_SIM_FILE->set_inode(temp);
        dir_tree_->get_bs()->ReWrinode(*old_SIM_FILE->get_inode(),true);
    }
    // create a hard link to the file
    // here we just print a message, in real implementation, we would create a hard link on disk
    // example:
    // ln ./xxxx/xxx/file.txt ./xxxx/xxx/file_link.txt
    // filename is file_link.txt and the parent directory path is ./xxxx/xxx/
    std::string filename = target_path.substr(target_path.find_last_of('/') + 1);
    size_t last_slash = target_path.find_last_of('/');
    std::string parent_dir_path;
    if (last_slash != std::string::npos) {
        parent_dir_path = target_path.substr(0, last_slash);
        parent_dir =dir_tree_->name_travesal(parent_dir_path, work_dir);
    } else {
        filename = target_path;             // 直接是文件名
        parent_dir = work_dir;
    }
    cout<<endl<<parent_dir->get_inode()->i_uid<<" id "<<cur_uid<<"   "<<parent_dir->get_name()<<" "<<parent_dir->get_inode()->i_num<<endl;
    if(parent_dir->get_inode()->i_uid != cur_uid){
        spdlog::error("Can't hard link one directory to others directory");
        g_logger-> error("Can't hard link one directory to others directory");
        return;
    }
    
    // use alloc_dir create the hard link using the file_entry's inode
    cout<<"owe"<<(int)(old_SIM_FILE->get_inode()->i_acl.owner)<<" "<<(1<<(cur_uid-1))<<endl;
    if((((old_SIM_FILE->get_inode()->i_acl.owner)>>(cur_uid-1))&1)==0)
    {
        spdlog::error("No authorization to link");
        g_logger->error("No authorization to link");
        return;
    }
    if(dir_tree_->alloc_dir_HARD(filename, parent_dir, old_SIM_FILE->get_inode()) == false) {
        spdlog::warn("Failed to create hard link for file path '{}': file already exists", source_path);
        g_logger->warn("Failed to create hard link for file path '{}': file already exists", source_path);
        return;
    }

    spdlog::info("Hard link created from '{}' to '{}'", source_path, target_path);
}
