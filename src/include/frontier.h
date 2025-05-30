#include <algorithm> 
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <locale.h>
#include <memory>
#include <cstdlib>
#include <wordexp.h>  // 新增路径解析头文件
#include <string>
#include <vector>
using namespace std;

// 窗口参数
string user_name="user_name";
string current_directory="/";
const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
 int CMD_WIDTH = 910;
const int ORIGIN_WIDTH=910;
const int ORIGIN_HEIGHT=610;
int origin_cmdx=0;
int origin_cmdy=0;
 int CMD_HEIGHT = 610;
const int TITLE_BAR_HEIGHT = 45;
const int MAX_VISIBLE_LINES = 18;
const int MAX_INPUT_DISPLAY = 75;
const int BTN_SIZE = 45;
// 在窗口参数区添加 ↓
const int TASKBAR_HEIGHT = 40;
// 在窗口参数区添加 ↓
const SDL_Color TEXT_GREEN = {0, 255, 0, 255};    // 用户名颜色
const SDL_Color TEXT_BLUE = {0, 120, 215, 255};    // 目录颜色
const SDL_Color TEXT_WHITE = {255, 255, 255, 255}; // 默认颜色
const SDL_Color TASKBAR_COLOR = {240, 240, 240, 255}; // RGB(240,240,240)
bool isWindowMinimized = false; // 新增窗口状态跟踪

// 新增光标闪烁控制变量[6](@ref)
bool showCursor = true;          // 当前是否显示光标
Uint32 lastCursorToggle = 0;     // 上次切换时间
const Uint32 CURSOR_BLINK_INTERVAL = 500; // 闪烁间隔500ms

// 在窗口参数区添加 ↓
int scrollOffset = 0;          // 新增滚动偏移量
const int LINE_HEIGHT = 24;    // 行高需与字体大小匹配
bool isScrolling = false;      // 新增滚动状态跟踪
SDL_Point scrollStartPos;      // 新增滚动起始坐标

// 在全局变量区新增CMD窗口最小化状态
bool cmdMinimized = false; // 新增：专门记录CMD窗口最小化状态
// SDL全局对象
SDL_Window* g_window;
SDL_Renderer* g_renderer;
TTF_Font* g_font;
SDL_Texture* g_cmdIcon;
SDL_Texture* g_background;
SDL_Texture* g_minimize;
SDL_Texture* g_maximize;
SDL_Texture* g_close;
SDL_Texture* g_restore;
TTF_Font* g_mainFont = nullptr;    // 主字体18pt
TTF_Font* g_smallFont = nullptr;   // 小字体8pt
vector<Command> history;
string currentCmd;
bool cmdOpen = false;

// 命令窗口结构体
struct CMDWindow {
    int x = 100, y = 50;
    bool isMaximized = false;
    bool isDragging = false;
    SDL_Point dragStart;
} cmdWin;

struct Command {
    string input;
    vector<string> output;
};

// 系统字体路径（优先尝试Linux常见字体）
const vector<string> FONT_PATHS = {
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", // 优先加载中文字体
    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",    // Ubuntu常见字体
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",   // DejaVu字体
    "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",        // 文泉驿微米黑
};


// 系统字体路径（兼容波浪号路径）
const vector<string> FONT_PATHS = {
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",   // DejaVu字体
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc" // 备用字体
};

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;
// 颜色定义
// 全局输入变量（添加在using namespace std;之后）
std::string user = "";
std::string password = "";
bool is_username_active = false;  // 当前激活的输入框
Uint32 cursor_blink_time = 0;     // 光标闪烁计时
bool is_button_hovered = false; // 新增：按钮悬停状态
const SDL_Color BOX_COLOR = {255, 255, 255, 128}; // 半透明白色
const SDL_Color TEXT_COLOR = {0, 0, 0, 255};     // 黑色文字
const SDL_Color BUTTON_COLOR = {240, 248, 255, 200}; // 按钮颜色

// 组件尺寸
const int LOGIN_BOX_WIDTH = 400;
const int LOGIN_BOX_HEIGHT = 375;
const int INPUT_WIDTH = 300;
const int INPUT_HEIGHT = 50;
const int BUTTON_WIDTH = 150;
const int BUTTON_HEIGHT = 50;

struct SDL_Deleter {
    void operator()(SDL_Window* w) const { if(w) SDL_DestroyWindow(w); }
    void operator()(SDL_Renderer* r) const { if(r) SDL_DestroyRenderer(r); }
    void operator()(TTF_Font* f) const { if(f) TTF_CloseFont(f); }
    void operator()(SDL_Texture* t) const { if(t) SDL_DestroyTexture(t); }
};
// 颜色定义
const SDL_Color BG_TOP = {18, 33, 43, 255};     // 深蓝
const SDL_Color BG_BOTTOM = {93, 173, 226, 255}; // 浅蓝
const SDL_Color PARTICLE_COLOR = {255, 255, 255, 80}; // 半透明白色

// 粒子系统参数
const int PARTICLE_COUNT = 150;
const float PARTICLE_SPEED = 1.5f;

struct AlertMessage {
    AlertMessage() = default;  // 保留默认构造
    AlertMessage(std::string t, SDL_Color c, int d, Uint32 s, int y) 
        : text(std::move(t)), color(c), duration(d), startTime(s), yPos(y) {}
    
    string text = "";
    SDL_Color color = {255, 0, 0, 255};
    int duration = 2000;
    Uint32 startTime = 0;
    int yPos = 0;
};
AlertMessage alert; // 全局提示对象

struct Particle {
    SDL_FPoint position;
    SDL_FPoint velocity;
    int size;
    float alpha;
};