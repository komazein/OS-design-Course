#include "test.h"
#include <FlexLexer.h>
// #include "global_extern.h"

file_system_manager* fileSystem = new file_system_manager;
std::vector<std::string> g_log_buffer;  // 存储日志的容器
std::shared_ptr<spdlog::logger> g_logger;  // 全局 logger
std::vector<std::pair<std::string, Color>> g_log_color_buffer; 

//FlexLexer lex;
// MSGSTR* msg = new MSGSTR;
class MyLexer : public yyFlexLexer {
public:
    MyLexer(std::istream* in, std::ostream* out) {
        switch_streams(in, out);  // 设置输入输出流
    }
};
void run_lexer(const std::string& input) {
    std::istringstream iss(input);   // 输入流
    std::ostringstream oss;          // 输出流

    MyLexer lexer(&iss, &oss);       // 构建词法分析器
    lexer.yylex();
}
// int main()
// {
//     yyFlexLexer lexer;
//     init_global_logger();         // 初始化
//   // g_logger->info("just test");
//   // for(auto & log : g_log_buffer){
//   //   cout << log <<endl;
//   // }
//     while (1)
//     {   string usern,pas;
//         cin>>usern>>pas;
//         if(fileSystem->CMPuser(usern,pas))
//             break;
//     }
    
//     while(1)
//     {
//         lexer.yylex();
//     }
//     return 0;
// }

int main() {

    yyFlexLexer lexer;
    init_global_logger();  
    // std::string input = "mkdir test";
    
    // {run_lexer(input);}
    // //std::cout << "Lexed output: " << result << std::endl;
    // return 0;
      ////////////////////////
    INIT_LOGIN_SYSYTM();
 
    InitSDL();

    
    SDL_Event event;
    // 设置全局渲染器（关键修改）
    g_window = g_res.window.get();
    g_renderer = g_res.renderer.get();
    while (true) {
        if (showLogin) {
            // 登录界面循环
            bool loginRunning = true;
            while (loginRunning) {
                mouse_handle(loginRunning, event);
                flush_new_figure();
            }
            user_history_cmd.clear();
            showLogin = false;  // 登录成功后切换状态
            LoadResources();
            SDL_StartTextInput();
        } else {
            // 系统界面循环
            run_system_front(fileSystem);
        }
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