#ifndef FRONTIER_H  // 建议使用文件名大写形式
#define FRONTIER_H
#pragma once
#include <algorithm> 
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <iostream>
#include<sstream>
#include <ctime>
#include <locale.h>
#include <random>
#include <chrono>
#include <memory>
#include <wordexp.h>  // 新增路径解析头文件
#include <fs.h>
#include <regex>

struct Command {
    string input;
    vector<pair<string,int>> output;
    vector<pair<string,int>>output_ans;
};
struct AlertMessage {
    AlertMessage() = default;  // 保留默认构造
    AlertMessage(std::string t, SDL_Color c, int d, Uint32 s, int y)
        : text(std::move(t)), color(c), duration(d), startTime(s), yPos(y) {}

    string text = "";
    SDL_Color color = { 255, 0, 0, 255 };
    int duration = 2000;
    Uint32 startTime = 0;
    int yPos = 0;
};
struct Particle {
    SDL_FPoint position;
    SDL_FPoint velocity;
    int size;
    float alpha;
};
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
const int TITLE_BAR_HEIGHT = 45;
const int BTN_SIZE_CMD = 45;
const int BTN_SIZE_RES = 15;
const int TASKBAR_HEIGHT = 40;
const SDL_Color TEXT_GREEN = {0, 255, 0, 255};    // 用户名颜色
const SDL_Color TEXT_BLUE = {0, 120, 215, 255};    // 目录颜色
const SDL_Color TEXT_WHITE = {255, 255, 255, 255}; // 默认颜色
const SDL_Color TASKBAR_COLOR = {240, 240, 240, 255}; // RGB(240,240,240)
const SDL_Rect iconRect = {10,10,60,50};
const SDL_Rect resIconRect = {10, iconRect.y + iconRect.h + 30, 60, 50}; // 新增资源图标位置
const Uint32 CURSOR_BLINK_INTERVAL = 500; // 闪烁间隔500ms
const SDL_Color BOX_COLOR = { 255, 255, 255, 128 }; // 半透明白色
const SDL_Color TEXT_COLOR = { 0, 0, 0, 255 };     // 黑色文字
const SDL_Color BUTTON_COLOR = { 240, 248, 255, 200 }; // 按钮颜色
const SDL_Color BG_TOP = { 18, 33, 43, 255 };     // 深蓝
const SDL_Color BG_BOTTOM = { 93, 173, 226, 255 }; // 浅蓝
const SDL_Color PARTICLE_COLOR = { 255, 255, 255, 80 }; // 半透明白色
// 粒子系统参数
const int PARTICLE_COUNT = 150;
const float PARTICLE_SPEED = 1.5f;
// 组件尺寸
const int LOGIN_BOX_WIDTH = 400;
const int LOGIN_BOX_HEIGHT = 375;
const int INPUT_WIDTH = 300;
const int INPUT_HEIGHT = 50;
const int BUTTON_WIDTH = 150;
const int BUTTON_HEIGHT = 50;
struct CMDWindow {
    int x = 100, y = 50;
    bool isMaximized = false;
    bool isDragging = false;
    bool cmdMinimized = false; // 新增：专门记录CMD窗口最小化状态
    string current_directory="/";
    int CMD_WIDTH = 910;
    int CMD_HEIGHT = 610;
    int origin_cmdx=0;
    int origin_cmdy=0;
    int lastHistorySize = 0;
    bool cmdOpen = false;
    string cursor=" ";
    bool showCursor = true;          // 当前是否显示光标
    SDL_Point dragStart;
    // 新增滚动相关字段
    int scrollOffset = 0;          // 滚动偏移量
    int contentHeight = 0;         // 内容总高度
    bool isScrolling = false;      // 是否正在滚动
    SDL_Point scrollStartPos;      // 滚动起始位置
} ;

// 从0开始
// enum TYPE
// {
//     SIM_FILE,
//     DIR,          // 那么这个inode中存储的就是其下的目录项(包含目录项名称, 以及他们对应的inode号)
//     LINK
// };
struct ResWindow {
    int x = 400, y = 150;
    int originX = 400, originY = 150; // 新增原始坐标记录
    bool isMaximized = false;
    bool isDragging = false;
    bool resMinimized = false;
    bool resOpen = false;
    int RES_WIDTH=800;
    int RES_HEIGHT=600;
    SDL_Point dragStart;
    int scrollOffset = 0;          // 新增滚动偏移
    int contentHeight = 0;         // 新增内容总高度
    bool isScrolling = false;      // 新增滚动状态
    string current_directory="/";
    string origin_directory="/";
    SDL_Point scrollStartPos;      // 新增滚动起始位置
    vector<FileItem> fileList;
    vector<pair<SDL_Rect,FileItem>> itemRects;
} ;
enum WindowType { CMD_WINDOW, RES_WINDOW };

struct SDL_Deleter {
    void operator()(SDL_Window* w) const { if (w) SDL_DestroyWindow(w); }
    void operator()(SDL_Renderer* r) const { if (r) SDL_DestroyRenderer(r); }
    void operator()(TTF_Font* f) const { if (f) TTF_CloseFont(f); }
    void operator()(SDL_Texture* t) const { if (t) SDL_DestroyTexture(t); }
};
// 全局资源结构体
struct GlobalResources {
    std::unique_ptr<SDL_Window, SDL_Deleter> window;
    std::unique_ptr<SDL_Renderer, SDL_Deleter> renderer;
    std::unique_ptr<TTF_Font, SDL_Deleter> font_label;
    std::unique_ptr<TTF_Font, SDL_Deleter> font_button;
    std::unique_ptr<TTF_Font, SDL_Deleter> font_title;
    std::unique_ptr<SDL_Texture, SDL_Deleter> offscreenTexture;
    std::unique_ptr<SDL_Texture, SDL_Deleter> loginTexture;
    std::unique_ptr<SDL_Texture, SDL_Deleter> userIconTexture;
    std::unique_ptr<TTF_Font, SDL_Deleter> font;
};


extern AlertMessage alert;
extern bool showLogin;
extern std::string user_in_name;
extern std::string password;
extern std::string user_name;
extern std::vector<std::string> user_history_cmd;
extern int cmd_history_index;
extern SDL_Texture* g_exitIcon;
extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
extern Uint32 lastCursorToggle ;  
extern Uint32 cursor_blink_time;    
extern bool is_button_hovered ; 
extern SDL_Texture* g_exitIcon;
extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
extern TTF_Font* g_font;
extern SDL_Texture* g_upArrow;
extern SDL_Texture* g_cmdIcon;
extern SDL_Texture* g_background;
extern SDL_Texture* g_minimize_CMD;
extern SDL_Texture* g_maximize_CMD;
extern SDL_Texture* g_close_CMD;
extern SDL_Texture* g_restore_CMD;
extern SDL_Texture* g_minimize_RES;
extern SDL_Texture* g_maximize_RES;
extern SDL_Texture* g_close_RES;
extern SDL_Texture* g_restore_RES;
extern SDL_Texture* g_resourceIcon; // 新增资源管理器图标
extern SDL_Texture* g_folderIcon;
extern SDL_Texture* g_txtIcon;  
extern SDL_Texture* g_linkIcon;
extern TTF_Font* g_midFont ;    // 中主字体13pt
extern TTF_Font* g_smallFont;   // 小字体8pt
extern vector<Command> history;
extern string currentCmd;
extern CMDWindow cmdWin;
extern ResWindow resWin;
extern std::vector<WindowType> windowOrder;
extern WindowType activeWindow;
extern SDL_Rect loginBox;
extern SDL_Rect head_sculpture_iconRect;
extern SDL_Rect inputRect;
extern SDL_Rect buttonRect;
extern GlobalResources g_res;
extern std::vector<Particle> particles;



// 系统字体路径（优先尝试Linux常见字体）
const vector<string> FONT_PATHS = {
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", // 优先加载中文字体
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",    // Ubuntu常见字体
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",   // DejaVu字体
    "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",        // 文泉驿微米黑
};

// 加载字体的改进方法
TTF_Font* LoadSystemFont(int size) ;
// 初始化SDL系统
bool InitSDL() ;



///TODO: 
string get_current_dir();


vector<string> ExecuteCommand(string cmd);
// 加载纹理资源[6](@ref)
SDL_Texture* LoadTexture(const char* path);

void LoadResources();
// 新增文本宽度计算函数[1](@ref)
int GetTextWidth(TTF_Font* font, const string& text) ;

vector<string> AutoWrapText(TTF_Font* font, const string& text, int maxWidth) ;

// 绘制带自动换行的文本[1,8](@ref)
void DrawText(int x, int y, const string& text, SDL_Color color = {255,255,255}) ;
// 新增文本渲染函数[6,7](@ref)
int RenderTextSegment(SDL_Renderer* renderer, TTF_Font* font, const string& text,SDL_Color color, int startX, int baseY) ;
void Gradient_display(SDL_Rect rect);
void DrawTaskbar();
// 绘制标题栏和按钮[6,8](@ref)
void DrawTitleBar() ;

string get_output_str(int inputMaxWidth,string str);
template<typename T> T cla(T value, T min_val, T max_val);
std::string TruncatePath(const std::string& path, int maxWidth); 
void DrawCMDWindow() ;

// 修改全局变量区的TruncateWithEllipsis函数 ↓
string TruncateWithEllipsis(TTF_Font* font, const string& text, int maxWidth) ;

vector<FileItem> return_filelist(file_system_manager *fileSystem);
// +++ 新增资源管理器窗口绘制函数 +++
void DrawResWindow() ;

// 处理窗口拖动和按钮点击[8,10](@ref)
void HandleTitleBarClick(int x, int y,file_system_manager *fileSystem) ;
void HandleResTitleBarClick(int x, int y) ;


// 主事件循环处理[6,8](@ref)
void HandleEvents(file_system_manager *fileSystem);
void display_text_mid(string text,SDL_Rect rec);
void run_system_front(file_system_manager *fileSystem);
bool InitializeGlobalResources() ;

void DrawCircle(SDL_Renderer* renderer, int x0, int y0, int radius, SDL_Color color) ;
void DrawRoundedRect(SDL_Renderer* renderer, int x, int y, int w, int h, int radius);
void DrawRoundedRect_Dynamic(SDL_Renderer* renderer, int x, int y, int w, int h, int radius, SDL_Color color) ;
void RenderText(SDL_Renderer* renderer, TTF_Font* font, const char* text,SDL_Color color, int x, int y) ;
void RenderAlert(SDL_Renderer* renderer, TTF_Font* font, AlertMessage& msg) ;
void mouse_handle(bool &running,SDL_Event &event);
void flush_new_figure();
void INIT_LOGIN_SYSYTM();
void run_lexer(const std::string& input);
#endif