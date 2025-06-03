#include "test.h"
#include <FlexLexer.h>
// #include "global_extern.h"

file_system_manager* fileSystem = new file_system_manager;
std::vector<std::string> g_log_buffer;  // 存储日志的容器
std::shared_ptr<spdlog::logger> g_logger;  // 全局 logger
//FlexLexer lex;
// MSGSTR* msg = new MSGSTR;
int main()
{
    yyFlexLexer lexer;
    init_global_logger();         // 初始化
  // g_logger->info("just test");
  // for(auto & log : g_log_buffer){
  //   cout << log <<endl;
  // }

    while (1)
    {   string usern,pas;
        cin>>usern>>pas;
        if(fileSystem->CMPuser(usern,pas))
            break;
    }
    
    while(1)
    {
        lexer.yylex();
    }
    return 0;
}
/*
init_global_logger();         // 初始化
  // g_logger->info("just test");
  // for(auto & log : g_log_buffer){
  //   cout << log <<endl;
  // }
  fileSystem->command_ls();
  string name = "user5";
  //fileSystem->command_find(name, 1);
  fileSystem->command_cd(name);
  name = "a.x";
  fileSystem->command_touch(name);
  fileSystem->command_ls();
  name = "/user5/a.x";
  string name2="/user7/m.x";
  fileSystem->command_lndir(name,name2);
  name="/user7";
  fileSystem->command_cd(name);
  fileSystem->command_ls();
  name="m.x";
  fileSystem->command_cat(name);
  string mssg="QWERTYUYTREW";
  fileSystem->command_edit(name,mssg);
  name="/user5";
  fileSystem->command_cd(name);
  name="a.x";
  fileSystem->command_cat(name);
  for(int i=0;i<g_log_buffer.size();i++)
    cout<<g_log_buffer[i]<<endl;
//  TESTWRITEBACK();
    return 0;
*/