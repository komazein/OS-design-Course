#include"frontier.h"
#include"log_global.h"
// 加载字体的改进方法
// 新增右键菜单相关变量
bool showResContextMenu = false;
SDL_Point resContextMenuPos;
int resContextMenuSelectedItem = -1; // -1表示未选中，0:在终端打开, 1:创建文本文件, 2:创建文件夹

// 右键菜单项定义
struct ContextMenuItem {
    string text;
    SDL_Texture* icon;
    SDL_Rect rect;
};
vector<ContextMenuItem> contextMenuItems;
AlertMessage alert;
bool showLogin = true;  // 新增状态变量
 std::string user_in_name = "";
 std::string password = "";
 string user_name="user_name";
 vector<string> user_history_cmd;
 int cmd_history_index=-1;
Uint32 lastCursorToggle = 0;     // 上次切换时间
bool is_username_active = false;  // 当前激活的输入框
Uint32 cursor_blink_time = 0;     // 光标闪烁计时
bool is_button_hovered = false; // 新增：按钮悬停状态
std::vector<Particle> particles(PARTICLE_COUNT);
// SDL全局图形变量
SDL_Texture* g_exitIcon;
SDL_Window* g_window;
SDL_Renderer* g_renderer;
TTF_Font* g_font;
SDL_Texture* g_upArrow;
SDL_Texture* g_cmdIcon;
SDL_Texture* g_background;
SDL_Texture* g_minimize_CMD;
SDL_Texture* g_maximize_CMD;
SDL_Texture* g_close_CMD;
SDL_Texture* g_restore_CMD;
SDL_Texture* g_minimize_RES;
SDL_Texture* g_maximize_RES;
SDL_Texture* g_close_RES;
SDL_Texture* g_restore_RES;
SDL_Texture* g_resourceIcon; // 新增资源管理器图标
SDL_Texture* g_folderIcon;
SDL_Texture* g_txtIcon;  
SDL_Texture* g_linkIcon;
TTF_Font* g_showFont = nullptr; 
TTF_Font* g_midFont = nullptr;    // 中主字体13pt
TTF_Font* g_smallFont = nullptr;   // 小字体8pt
vector<Command> history;
string currentCmd;
struct CMDWindow cmdWin;
struct ResWindow resWin;
// 新增窗口顺序列表和活动窗口记录
vector<WindowType> windowOrder = {CMD_WINDOW, RES_WINDOW};
WindowType activeWindow = CMD_WINDOW;
//初始化各按钮位置
SDL_Rect loginBox = {(SCREEN_WIDTH - LOGIN_BOX_WIDTH) / 2,(SCREEN_HEIGHT - LOGIN_BOX_HEIGHT) / 2,LOGIN_BOX_WIDTH,LOGIN_BOX_HEIGHT};
SDL_Rect head_sculpture_iconRect = {loginBox.x + (LOGIN_BOX_WIDTH - 80) / 2 - 55,  loginBox.y - 200,200, 200};
SDL_Rect inputRect = {loginBox.x + (LOGIN_BOX_WIDTH - INPUT_WIDTH) / 2,loginBox.y + 100,INPUT_WIDTH,INPUT_HEIGHT};
SDL_Rect buttonRect = {loginBox.x + (LOGIN_BOX_WIDTH - BUTTON_WIDTH) / 2,loginBox.y + LOGIN_BOX_HEIGHT - 100,BUTTON_WIDTH,BUTTON_HEIGHT};
// 全局变量声明
GlobalResources g_res;


template<typename T>
T cla(T value, T min_val, T max_val) {
    return (value < min_val) ? min_val :(value > max_val) ? max_val : value;  // [6,7](@ref)
}

// front.cpp
std::string TruncatePath(const std::string& path, int maxWidth) {
    const std::string ellipsis = "...";
    const int ellipsisWidth = GetTextWidth(g_midFont, ellipsis);
    
    if(GetTextWidth(g_midFont, path) <= maxWidth) return path;
    
    std::string truncated;
    int currentWidth = ellipsisWidth;
    
    // 从后往前逐个字符尝试添加
    for(auto rit = path.rbegin(); rit != path.rend(); ++rit) {
        char c = *rit;
        int charBytes = 1;
        if((c & 0xF0) == 0xE0) charBytes = 3;
        else if((c & 0xE0) == 0xC0) charBytes = 2;
        
        std::string charStr;
        for(int i=0; i<charBytes && rit != path.rend(); ++i, ++rit) {
            charStr.insert(0, 1, *rit);
        }
        --rit;
        
        int charWidth = GetTextWidth(g_midFont, charStr);
        if(currentWidth + charWidth > maxWidth) break;
        
        currentWidth += charWidth;
        truncated.insert(0, charStr);
    }
    
    return ellipsis + truncated;
}


TTF_Font* LoadSystemFont(int size) {
    for (const auto& path : FONT_PATHS) {
        if (TTF_Font* font = TTF_OpenFont(path.c_str(), size)) {
            cout << "成功加载字体: " << path << endl;
            return font;
        }
    }
    cerr << "错误:未找到系统字体,请尝试安装:sudo apt install fonts-freefont-ttf fonts-dejavu" << endl;
    return nullptr;
}

// 初始化SDL系统
bool InitSDL() {
     setlocale(LC_ALL, "zh_CN.UTF-8"); // 新增区域设置[3](@ref)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL初始化失败: " << SDL_GetError() << endl;
        return false;
    }
    if (TTF_Init() == -1) {
        cerr << "TTF初始化失败: " << TTF_GetError() << endl;
        return false;
    }
    // 创建渲染器
    //g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // 加载系统字体
    g_showFont = LoadSystemFont(22);
    g_midFont = LoadSystemFont(15);
    g_smallFont = LoadSystemFont(8);
    
    if (!g_showFont || !g_smallFont) {
        cerr << "字体加载失败，请检查系统字体路径" << endl;
        return false;
    }
    return true;
}



///TODO: 
string get_current_dir(file_system_manager *fileSystem){//后端接口  //返回当前cmd窗口的路径！！
     auto curdir = fileSystem->get_cur_dir();
     return curdir;
}

// 加载纹理资源[6](@ref)
SDL_Texture* LoadTexture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        cerr << "图片加载失败: " << IMG_GetError() << endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void LoadResources() {
    g_exitIcon = LoadTexture("../img/exit_icon.png");
    g_upArrow = LoadTexture("../img/up_arrow.png");
    g_folderIcon = LoadTexture("../img/folder_icon.png");
    g_txtIcon = LoadTexture("../img/txt_icon.png");
    g_linkIcon = LoadTexture("../img/link.png"); // 新增链接图标
    g_resourceIcon = LoadTexture("../img/resource.png");
    g_cmdIcon = LoadTexture("../img/cmd_icon.png");
    g_background = LoadTexture("../img/background.png");
    g_minimize_CMD = LoadTexture("../img/minimize.png");
    g_maximize_CMD = LoadTexture("../img/Maximize.png");
    g_close_CMD = LoadTexture("../img/close.png");
    g_restore_CMD = LoadTexture("../img/restore.png"); // 新增恢复按钮加载[2,6](@ref)
    g_minimize_RES = LoadTexture("../img/minimize_RES.png");
    g_maximize_RES = LoadTexture("../img/Maximize_RES.png");
    g_close_RES = LoadTexture("../img/close_RES.png");
    g_restore_RES = LoadTexture("../img/restore_RES.png"); // 新增恢复按钮加载[2,6](@ref)
}
// 新增文本宽度计算函数[1](@ref)
int GetTextWidth(TTF_Font* font, const string& text) {
    int width = 0;
    TTF_SizeUTF8(font, text.c_str(), &width, nullptr);
    return width;
}

vector<string> AutoWrapText(TTF_Font* font, const string& text, int maxWidth) {
    vector<string> lines;
    string currentLine;
    
    for (char c : text) {
        // 预判新行宽度
        string tempLine = currentLine + c;
        int tempWidth = GetTextWidth(font, tempLine);
        
        if (tempWidth > maxWidth) {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine.clear();
            }
            currentLine += c; // 强制换行后新行以当前字符开始
        } else {
            currentLine = tempLine;
        }
    }
    
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    return lines;
}

// 绘制带自动换行的文本[1,8](@ref)
void DrawText(int x, int y, const string& text, SDL_Color color ) {
    vector<string> lines;
    string currentLine;
    
    for (char c : text) {
        if (c == '\n' ) {
            lines.push_back(currentLine);
            currentLine.clear();
        }
        if (c != '\n') currentLine += c;
    }
    if (!currentLine.empty()) lines.push_back(currentLine);

    // 渲染多行文本
    int lineHeight = TTF_FontLineSkip(g_midFont);
    for (const auto& line : lines) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(g_midFont, line.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
            SDL_Rect dest = {x, y, surface->w, surface->h};
            SDL_RenderCopy(g_renderer, texture, NULL, &dest);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }
        y += lineHeight;
    }
}

// 新增文本渲染函数[6,7](@ref)
int RenderTextSegment(SDL_Renderer* renderer, TTF_Font* font, const string& text,SDL_Color color, int startX, int baseY) 
{
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return startX;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {startX, baseY, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    int newX = startX + surface->w;
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
    
    return newX;
}
void Gradient_display(SDL_Rect rect){
    SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
    // 渐变参数
    int rMin = 220, rMax = 250;  // R: 220~250
    int gMin = 220, gMax = 230;  // G: 220~230
    int bMin = 220, bMax = 250;  // B: 220~250

    // 逐行绘制渐变
    for (int y = rect.y; y < rect.y+rect.h; y++) {
    // 计算当前行的颜色（线性插值）
        float t = (float)(y - rect.y) / TASKBAR_HEIGHT;
        Uint8 r = (Uint8)(rMin + t * (rMax - rMin));
        Uint8 g = (Uint8)(gMin + t * (gMax - gMin));
        Uint8 b = (Uint8)(bMin + t * (bMax - bMin));

        // 设置当前行的颜色
        SDL_SetRenderDrawColor(g_renderer, r, g, b, 255);
        SDL_RenderDrawLine(g_renderer, rect.x, y, rect.x+rect.w, y);  // 绘制一整行
    }
}
void DrawTaskbar() {
    // 启用透明度混合模式[8,6](@ref)
    SDL_BlendMode originalMode;
    SDL_GetRenderDrawBlendMode(g_renderer, &originalMode); // 保存原始模式[7](@ref)

    // 绘制半透明任务栏背景（RGBA：240,240,240,200）
    int winW, winH;
    SDL_GetWindowSize(g_window, &winW, &winH);
    int taskbarTop = winH - TASKBAR_HEIGHT;
    SDL_Rect taskbarRect = {0, taskbarTop, winW, TASKBAR_HEIGHT};
    Gradient_display(taskbarRect);
    // 绘制任务栏图标及交互效果
    SDL_Rect taskbarIcon = {10, SCREEN_HEIGHT - TASKBAR_HEIGHT + 5, 30, 30};
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    // 图标状态管理
    if(cmdWin.cmdOpen) {
        // 最小化时显示半透明图标[7](@ref)
        if(cmdWin.cmdMinimized) {
            //SDL_SetTextureAlphaMod(g_cmdIcon, 128); // 50%透明度
        } else {
            SDL_SetTextureAlphaMod(g_cmdIcon, 255); // 恢复不透明
            
            // 绘制激活状态指示条[11](@ref)
            SDL_SetRenderDrawColor(g_renderer, 0, 120, 215, 255); // Windows蓝
            SDL_Rect highlight = {taskbarIcon.x, taskbarIcon.y + 28, 30, 2};
            SDL_RenderFillRect(g_renderer, &highlight);
        }
        
        // 绘制带阴影的图标[4](@ref)
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 30);
        SDL_Rect shadow = {taskbarIcon.x+2, taskbarIcon.y+2, 30, 30};
        SDL_RenderFillRect(g_renderer, &shadow);
        
        // 渲染主图标
        SDL_RenderCopy(g_renderer, g_cmdIcon, NULL, &taskbarIcon);

        // 鼠标悬停效果[3,11](@ref)

        SDL_Point mousePos = {mouseX, mouseY};  // 创建命名对象
        SDL_Rect iconArea = {
            taskbarIcon.x, 
            taskbarIcon.y,
            taskbarIcon.w,
            taskbarIcon.h
        };
        
        if (SDL_PointInRect(&mousePos, &iconArea)) {  // 传递已命名对象的地址
            // 鼠标悬停效果实现
            SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 50);
            SDL_RenderFillRect(g_renderer, &iconArea);
        }

    }
    SDL_Rect resTaskbarIcon = {60, SCREEN_HEIGHT - TASKBAR_HEIGHT + 5, 30, 30};
    if(resWin.resOpen) {
        // 最小化状态处理
        if(resWin.resMinimized) {
            //SDL_SetTextureAlphaMod(g_resourceIcon, 128);
        } else {
            SDL_SetTextureAlphaMod(g_resourceIcon, 255);
            // 激活状态指示
            if(activeWindow == RES_WINDOW) {
                SDL_SetRenderDrawColor(g_renderer, 0, 120, 215, 255);
                SDL_Rect highlight = {resTaskbarIcon.x, resTaskbarIcon.y + 28, 30, 2};
                SDL_RenderFillRect(g_renderer, &highlight);
            }
        }

        // 绘制图标阴影
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 30);
        SDL_Rect shadow = {resTaskbarIcon.x+2, resTaskbarIcon.y+2, 30, 30};
        SDL_RenderFillRect(g_renderer, &shadow);

        // 渲染主图标
        SDL_RenderCopy(g_renderer, g_resourceIcon, NULL, &resTaskbarIcon);

        // 鼠标悬停效果
        SDL_Point mousePos = {mouseX, mouseY};
        SDL_Rect iconArea = {resTaskbarIcon.x, resTaskbarIcon.y, 30, 30};
        if (SDL_PointInRect(&mousePos, &iconArea)) {
            SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 50);
            SDL_RenderFillRect(g_renderer, &iconArea);
        }
    }
     SDL_Rect exitIcon = {
        winW - 40, // 屏幕宽度减去40像素（右侧位置）
        SCREEN_HEIGHT - TASKBAR_HEIGHT + 5,
        30,
        30
    };
    SDL_Point mousePos = {mouseX, mouseY};
    // 绘制退出图标
    SDL_RenderCopy(g_renderer, g_exitIcon, NULL, &exitIcon);
    
    // 悬停效果
    if (SDL_PointInRect(&mousePos, &exitIcon)) {
        SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 50);
        SDL_RenderFillRect(g_renderer, &exitIcon);
    }
    // 恢复默认混合模式
     SDL_SetRenderDrawBlendMode(g_renderer, originalMode); // 恢复原始模式
}
// 绘制标题栏和按钮[6,8](@ref)
void DrawTitleBar() {
    // 标题栏背景
    SDL_Rect titleRect = {cmdWin.x, cmdWin.y, cmdWin.CMD_WIDTH, TITLE_BAR_HEIGHT};
    SDL_SetRenderDrawColor(g_renderer, 46, 46, 46, 255);
    SDL_RenderFillRect(g_renderer, &titleRect);

    // 绘制按钮
    int btnX = cmdWin.x + cmdWin.CMD_WIDTH - 3*(BTN_SIZE_CMD+5);
    SDL_Rect btnRect = {btnX, cmdWin.y, BTN_SIZE_CMD, BTN_SIZE_CMD};
    SDL_RenderCopy(g_renderer, g_minimize_CMD, NULL, &btnRect);
    btnRect.x += BTN_SIZE_CMD + 5;
    SDL_Texture* currentMaximizeTex = cmdWin.isMaximized ? g_restore_CMD : g_maximize_CMD;
    SDL_RenderCopy(g_renderer, currentMaximizeTex, NULL, &btnRect); // [2,6](@ref)
    btnRect.x += BTN_SIZE_CMD + 5;
    SDL_RenderCopy(g_renderer, g_close_CMD, NULL, &btnRect);
}

string get_output_str(int inputMaxWidth,string str){

    int availableWidth = inputMaxWidth; // 输入区域最大宽度
    string display;
    int currentWidth = 0;

    // 反向遍历字符，找到可显示的最大长度
    for (auto it = str.rbegin(); it != str.rend(); ++it) {
        char c = *it;
        int charWidth = GetTextWidth(g_showFont, string(1, c));
        
        if (currentWidth + charWidth > availableWidth) {
            break;
        }
        
        display.insert(0, 1, c); // 反向插入字符
        currentWidth += charWidth;
    }
    return display;
}

void DrawCMDWindow() {
    if (!cmdWin.cmdOpen || cmdWin.cmdMinimized) return; 
    SDL_Rect cmdRect = {cmdWin.x, cmdWin.y+TITLE_BAR_HEIGHT, cmdWin.CMD_WIDTH, cmdWin.CMD_HEIGHT };
    SDL_SetRenderDrawColor(g_renderer, 48,10,36,255);
    SDL_RenderFillRect(g_renderer, &cmdRect);

    const int lineHeight = TTF_FontLineSkip(g_showFont);
    SDL_Color textColor = {255,255,255,255};

    // 计算总内容高度
    cmdWin.contentHeight = 0;
    for (auto& cmd : history) {
        // 输入行高度
        cmdWin.contentHeight += (GetTextWidth(g_showFont,cmd.input) / (cmdWin.CMD_WIDTH - 40) + 1) * lineHeight;
        // 输出行高度
        for (auto& line : cmd.output) {
            cmdWin.contentHeight += (GetTextWidth(g_showFont,line.first) / (cmdWin.CMD_WIDTH - 40) + 1) * lineHeight;
        }
        string total="";
        for(auto &val :cmd.output_ans){
           total+=val.first+"  ";
        }
        if (cmd.output_ans.size()!=0)
        {        
            cmdWin.contentHeight += (GetTextWidth(g_showFont,total) / (cmdWin.CMD_WIDTH - 40) + 1) * lineHeight;
        }
    }
    // 限制滚动范围
    int maxScroll = max(0, cmdWin.contentHeight - cmdWin.CMD_HEIGHT  + TITLE_BAR_HEIGHT + 40);
    cmdWin.scrollOffset = cla(cmdWin.scrollOffset, 0, maxScroll);

    int yPos = cmdWin.y + TITLE_BAR_HEIGHT + 20 - cmdWin.scrollOffset;
    int visibleBottom = cmdWin.y + TITLE_BAR_HEIGHT + cmdWin.CMD_HEIGHT -100;

    // 渲染所有历史记录
    for (auto rit = history.rbegin(); rit != history.rend(); ++rit) {
        // 渲染输入行
        string input = rit->input;
        vector<string> inputLines = AutoWrapText(g_showFont, input, cmdWin.CMD_WIDTH - 40);
        
        for (auto& line : inputLines) {
            if (yPos + lineHeight < cmdWin.y + TITLE_BAR_HEIGHT || yPos > visibleBottom) {
                yPos += lineHeight;
                continue;
                if( yPos > visibleBottom){
                    break;
                }
            }

            int baseX = cmdWin.x + 20;
            int standardization_x = baseX;

            // 解析用户名部分
            size_t colon_pos = line.find(':');
            string user_part = line.substr(0, colon_pos + 1);
            standardization_x = RenderTextSegment(g_renderer, g_showFont, user_part, TEXT_GREEN, standardization_x, yPos);

            // 解析目录部分
            size_t dollar_pos = line.find('$', colon_pos + 1);
            string dir_part = line.substr(colon_pos + 1, dollar_pos - colon_pos);
            standardization_x = RenderTextSegment(g_renderer, g_showFont, dir_part, TEXT_BLUE, standardization_x, yPos);

            // 渲染命令部分
            string cmd_part = line.substr(dollar_pos + 1);
            RenderTextSegment(g_renderer, g_showFont, cmd_part.c_str(), TEXT_WHITE, standardization_x, yPos);
            
            yPos += lineHeight;
        }

        // 渲染输出行
        for (auto& output : rit->output) {
            vector<string> outputLines = AutoWrapText(g_showFont, output.first, cmdWin.CMD_WIDTH - 40);
            for (auto& line : outputLines) {
                if (yPos + lineHeight < cmdWin.y + TITLE_BAR_HEIGHT || yPos > visibleBottom) {
                    yPos += lineHeight;
                    continue;
                    if( yPos > visibleBottom){
                        break;
                    }
                }
                RenderTextSegment(g_renderer, g_showFont, line.c_str(), textColor, cmdWin.x + 20, yPos);
                yPos += lineHeight;
            }
        }
        // 渲染output_ans部分（在output之后）
        if (!rit->output_ans.empty()) {
            int baseX = cmdWin.x + 20;  // 起始X坐标
            int currentX = baseX;       // 当前行X坐标
            int lineY = yPos;           // 当前行Y坐标
            
            // 遍历所有输出项
            for (size_t i = 0; i < rit->output_ans.size(); i++) {
                const auto& item = rit->output_ans[i];
                const string& text = item.first;
                int colorCode = item.second;
                
                // 设置颜色
                SDL_Color textColor;
                switch (colorCode) {
                    case 31: textColor = {255, 0, 0, 255}; break;   // 红色
                    case 32: textColor = {38, 162, 105, 255}; break;   // 绿色
                    case 33: textColor = {255, 255, 0, 255}; break; // 黄色
                    case 34: textColor = {8, 69, 143, 255}; break;   // 蓝色
                    case 35: textColor = {255, 0, 255, 255}; break; // 品红
                    case 36: textColor = {0, 255, 255, 255}; break; // 青色
                    default: textColor = {255, 255, 255, 255};      // 白色
                }
                
                // 计算文本宽度（包括空格）
                int textWidth = GetTextWidth(g_showFont, text);
                int spaceWidth = GetTextWidth(g_showFont, "        ");
                int totalWidth = textWidth + (i < rit->output_ans.size() - 1 ? spaceWidth : 0);
                
                // 检查是否需要换行
                if (currentX + totalWidth > cmdWin.x + cmdWin.CMD_WIDTH - 20) {
                    // 换行处理
                    currentX = baseX;
                    lineY += lineHeight;
                }
                    // 检查是否超出可见区域
                if (lineY + lineHeight < cmdWin.y + TITLE_BAR_HEIGHT || lineY > visibleBottom) {
                    lineY += lineHeight;
                    if( lineY > visibleBottom){
                        break;
                    }
                    continue;
                }
                // 渲染文本
                currentX = RenderTextSegment(g_renderer, g_showFont, text.c_str(), textColor, currentX, lineY);
                
                // 渲染空格（非最后一项）
                if (i < rit->output_ans.size() - 1) {
                    currentX = RenderTextSegment(g_renderer, g_showFont, "        ", textColor, currentX, lineY);
                }
            }
            
            // 更新Y位置到下一行
            yPos = lineY + lineHeight;
        }
    }
        // +++ 新增滚动条绘制 +++
    if (cmdWin.contentHeight > (cmdWin.CMD_HEIGHT  - TITLE_BAR_HEIGHT-40)) {
        int scrollBarWidth = 8;
        int scrollTrackHeight = cmdWin.CMD_HEIGHT  - TITLE_BAR_HEIGHT-10;
        float scrollRatio = (float)scrollTrackHeight / cmdWin.contentHeight;
        int scrollThumbHeight = scrollTrackHeight * scrollRatio;
        int scrollThumbY = cmdWin.y + TITLE_BAR_HEIGHT + (cmdWin.scrollOffset * scrollRatio);

        // 绘制滚动轨道
        SDL_Rect trackRect = {
            cmdWin.x + cmdWin.CMD_WIDTH - scrollBarWidth - 5,
            cmdWin.y + TITLE_BAR_HEIGHT + 5,
            scrollBarWidth,
            scrollTrackHeight+20
        };
        SDL_SetRenderDrawColor(g_renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(g_renderer, &trackRect);

        // 绘制滚动滑块
        SDL_Rect thumbRect = {
            cmdWin.x + cmdWin.CMD_WIDTH - scrollBarWidth - 5,
            scrollThumbY,
            scrollBarWidth,
            scrollThumbHeight
        };
        SDL_SetRenderDrawColor(g_renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(g_renderer, &thumbRect);
    }
    if (activeWindow == CMD_WINDOW)
    {
        cmdWin.cursor = cmdWin.showCursor ? "_" : " "; // 使用空格保持字符宽度一致
    }
    // 绘制输入行（固定位置）
    int baseY = cmdWin.y + cmdWin.CMD_HEIGHT  - 20;
    int currentX = cmdWin.x + 20;
    
    currentX = RenderTextSegment(g_renderer, g_showFont, user_name + ":", TEXT_GREEN, currentX, baseY);
    string Standardization_dir=TruncatePath(cmdWin.current_directory,cmdWin.CMD_WIDTH/4);
    currentX = RenderTextSegment(g_renderer, g_showFont, Standardization_dir+ "$ ", TEXT_BLUE, currentX, baseY);
    
    int inputMaxWidth = cmdWin.CMD_WIDTH - GetTextWidth(g_showFont, user_name + ":") - GetTextWidth(g_showFont, Standardization_dir + "$ ") - 40;
    string trimmedCmd = get_output_str(inputMaxWidth, currentCmd);
    RenderTextSegment(g_renderer, g_showFont, trimmedCmd + cmdWin.cursor, TEXT_WHITE, currentX, baseY);

    DrawTitleBar();
}

// 修改全局变量区的TruncateWithEllipsis函数 ↓
string TruncateWithEllipsis(TTF_Font* font, const string& text, int maxWidth) {
    string result;
    int width = 0;
    const string ellipsis = "..."; 
    int ellipsisWidth = GetTextWidth(font, ellipsis);

    // UTF-8字符遍历
    size_t i = 0;
    while (i < text.length()) {
        int charLen = 1;
        // 判断UTF-8字符长度（汉字3字节）
        if ((text[i] & 0xF0) == 0xE0) charLen = 3;
        else if ((text[i] & 0xE0) == 0xC0) charLen = 2;
        
        // 检查是否超出剩余空间
        string currentChar = text.substr(i, charLen);
        int charWidth = GetTextWidth(font, currentChar);
        
        if (width + charWidth + ellipsisWidth > maxWidth) break;
            result += currentChar;
            width += charWidth;
            i += charLen; // 跳转到下一个字符起始位置
        }
        // 只有当实际截断时才添加省略号
        if (i < text.length()) {
            // 确保省略号不会单独出现在行首
            if (!result.empty()) result += ellipsis;
            else result = ellipsis;
        }
    return result;
}

vector<FileItem> return_filelist(file_system_manager *fileSystem){//后端接口
    vector<FileItem> fileList;
    fileSystem->get_ls(fileList);
    return fileList;

}
void reset_res(file_system_manager *fileSystem){
    string origin=fileSystem->get_cur_dir();
    fileSystem->command_cd(resWin.current_directory);
    resWin.fileList=return_filelist(fileSystem);
    fileSystem->command_cd(origin);
}
// +++ 新增资源管理器窗口绘制函数 +++
void DrawResWindow() {
    if (!resWin.resOpen || resWin.resMinimized) return;
    // 绘制窗口主体（白色背景）
    SDL_Rect winRect = {resWin.x, resWin.y+TITLE_BAR_HEIGHT, resWin.RES_WIDTH, resWin.RES_HEIGHT-TITLE_BAR_HEIGHT};
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255); // 纯白背景
    SDL_RenderFillRect(g_renderer, &winRect);

    // 文件显示参数
    const int ICON_SIZE = 64;       // 图标尺寸
    const int TEXT_HEIGHT = 20;     // 文字高度
    const int ITEM_SPACING = 30;    // 项目间距
    int row_show_length;
    if (resWin.isMaximized)
    {
        row_show_length=20;
    }else{
        row_show_length=8;
    }
    int rows = (resWin.fileList.size() + row_show_length-1) / row_show_length; // 每行6个文件
    resWin.contentHeight = rows * (ICON_SIZE + TEXT_HEIGHT + 20);
    
    // 限制滚动范围
    int maxScroll = max(0, resWin.contentHeight - (resWin.RES_HEIGHT - TITLE_BAR_HEIGHT - 30));
    resWin.scrollOffset = cla(resWin.scrollOffset, 0, maxScroll);


    const int MAX_TEXT_WIDTH = ICON_SIZE + 20; // 新增：文本最大宽度（图标宽度+边距）
    int contentBottom = winRect.y + winRect.h; // 窗口内容区域底部Y坐标

    int startX = resWin.x + 20;    // 起始X坐标
    int startY = resWin.y + TITLE_BAR_HEIGHT + 30- resWin.scrollOffset; // 起始Y坐标
    int currentX = startX;
    int currentY = startY;

    // 遍历文件列表
    resWin.itemRects.clear();
    for (const auto& file : resWin.fileList) {
        // 计算可见区域边界
        int windowTop = resWin.y + TITLE_BAR_HEIGHT+ICON_SIZE;
        int windowBottom = resWin.y + resWin.RES_HEIGHT-ICON_SIZE-TEXT_HEIGHT-10;
        
        // 计算当前项的边界
        int itemTop = currentY;
        int itemBottom = currentY + ICON_SIZE + TEXT_HEIGHT;

        // 跳过完全不可见的项
        if (itemBottom < windowTop || itemTop > windowBottom) {
            // 更新位置但不渲染
            currentX += ICON_SIZE + ITEM_SPACING;
            if(currentX + ICON_SIZE > resWin.x + resWin.RES_WIDTH) {
                currentX = startX;
                currentY += ICON_SIZE + TEXT_HEIGHT + 20;
            }
            continue;
        }
        // 获取对应图标
        SDL_Texture* mainIcon = nullptr;
        switch(file.type) {
            case 1: mainIcon = g_txtIcon; break;//g_txtIcon
            case 2: mainIcon = g_folderIcon; break;
            case 3: mainIcon = g_folderIcon; break;
        }

        // 绘制主图标
        SDL_Rect iconRect = {currentX, currentY, ICON_SIZE, ICON_SIZE};
        SDL_RenderCopy(g_renderer, mainIcon, NULL, &iconRect);

        // 绘制链接标识（类型3）
        if(file.type == 3) {
            SDL_Rect linkRect = {currentX+12, currentY+ICON_SIZE-20, 16, 16};
            SDL_RenderCopy(g_renderer, g_linkIcon, NULL, &linkRect);
        }

        // 绘制文件名
        SDL_Color textColor = {40, 40, 40, 255};
        string displayName = TruncateWithEllipsis(g_midFont, file.name, MAX_TEXT_WIDTH);
        int textWidth = GetTextWidth(g_midFont, displayName);
        int textX = currentX + (ICON_SIZE - textWidth)/2;
        DrawText(textX, currentY + ICON_SIZE + 5, displayName, textColor);
        resWin.itemRects.push_back({{currentX,currentY, ICON_SIZE, ICON_SIZE + 20},file});
        // 更新位置
        currentX += ICON_SIZE + ITEM_SPACING;
        if(currentX + ICON_SIZE > resWin.x + resWin.RES_WIDTH) {
            currentX = startX;
            currentY += ICON_SIZE + TEXT_HEIGHT + 20;
        }
    }
    // +++ 新增滚动条绘制 +++
        // +++ 新增滚动条绘制 +++
    if (resWin.contentHeight > (resWin.RES_HEIGHT - TITLE_BAR_HEIGHT)) {
        // 滚动条参数
        int scrollBarWidth = 8;
        int scrollTrackHeight = resWin.RES_HEIGHT - TITLE_BAR_HEIGHT - 20;
        float scrollRatio = (float)scrollTrackHeight / resWin.contentHeight;
        int scrollThumbHeight = scrollTrackHeight * scrollRatio;
        int scrollThumbY = resWin.y + TITLE_BAR_HEIGHT + 5 + 
                            (resWin.scrollOffset * scrollRatio);

        // 绘制滚动轨道
        SDL_Rect trackRect = {
            resWin.x + resWin.RES_WIDTH - scrollBarWidth - 5,
            resWin.y + TITLE_BAR_HEIGHT + 5,
            scrollBarWidth,
            scrollTrackHeight
        };
        SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(g_renderer, &trackRect);

        // 绘制滚动滑块
        SDL_Rect thumbRect = {
            resWin.x + resWin.RES_WIDTH - scrollBarWidth - 5,
            scrollThumbY,
            scrollBarWidth,
            scrollThumbHeight
        };
        SDL_SetRenderDrawColor(g_renderer, 150, 150, 150, 255);
        SDL_RenderFillRect(g_renderer, &thumbRect);
    }
    // 最后绘制渐变标题栏（与CMD窗口不同）
    SDL_Rect titleRect = {
        resWin.x, 
        resWin.y, 
        resWin.isMaximized ? SCREEN_WIDTH : resWin.RES_WIDTH, // 处理最大化状态
        TITLE_BAR_HEIGHT
    };
    Gradient_display(titleRect); // 对标题栏应用渐变
    // +++ 新增向上箭头绘制 +++
    SDL_Rect upArrowRect = {
        resWin.x + 10, 
        resWin.y + (TITLE_BAR_HEIGHT - 20)/2, 
        20, 
        20
    };
    // 替换为 ↓
    // 计算可用宽度（标题栏宽度 - 箭头区域 - 按钮区域 - 边距）
    const int ARROW_WIDTH = 40;        // 箭头区域宽度
    const int BUTTONS_WIDTH = 150;     // 右侧按钮区域
    int maxDisplayWidth = resWin.RES_WIDTH - ARROW_WIDTH - BUTTONS_WIDTH -50;
    SDL_Color pathColor = {40, 40, 40, 255};

    string displayPath = TruncatePath(resWin.current_directory, maxDisplayWidth);
    string dirDisplay = "Path: " + displayPath;
    DrawText(resWin.x + 40, resWin.y + (TITLE_BAR_HEIGHT - 20)/2, dirDisplay, pathColor);

    SDL_RenderCopy(g_renderer, g_upArrow, NULL, &upArrowRect);

    // 绘制按钮（关闭、最大化、最小化）
    const int BUTTON_SPACING = 30; // 按钮间距
    const int BUTTON_RIGHT_MARGIN = 5; // 右侧边距

    // 计算各按钮的X坐标
    int closeX = resWin.x + (resWin.isMaximized ? SCREEN_WIDTH : resWin.RES_WIDTH) - BTN_SIZE_RES - BUTTON_RIGHT_MARGIN;
    int maximizeX = closeX - BTN_SIZE_RES - BUTTON_SPACING;
    int minimizeX = maximizeX - BTN_SIZE_RES - BUTTON_SPACING;

    // 垂直居中
    int btnY = resWin.y + (TITLE_BAR_HEIGHT - BTN_SIZE_RES)/2;

    // 关闭按钮
    SDL_Rect closeRect = {closeX, btnY, BTN_SIZE_RES, BTN_SIZE_RES};
    SDL_RenderCopy(g_renderer, g_close_RES, NULL, &closeRect);

    // 最大化/恢复按钮
    SDL_Texture* maximizeTex = resWin.isMaximized ? g_restore_RES : g_maximize_RES;
    SDL_Rect maximizeRect = {maximizeX, btnY, BTN_SIZE_RES, BTN_SIZE_RES};
    SDL_RenderCopy(g_renderer, maximizeTex, NULL, &maximizeRect);

    // 最小化按钮
    SDL_Rect minimizeRect = {minimizeX, btnY, BTN_SIZE_RES, BTN_SIZE_RES};
    SDL_RenderCopy(g_renderer, g_minimize_RES, NULL, &minimizeRect);
    if (showResContextMenu) {
            for (int i = 0; i < contextMenuItems.size(); i++) {
                // 菜单项背景
                SDL_Color bgColor = {255, 255, 255, 255}; // 默认白色
                if (i == resContextMenuSelectedItem) {
                    bgColor = {222, 224, 234, 255}; // 悬停颜色
                }
                
                SDL_SetRenderDrawColor(g_renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
                SDL_RenderFillRect(g_renderer, &contextMenuItems[i].rect);
                
                // 菜单项边框
                SDL_SetRenderDrawColor(g_renderer, 200, 200, 200, 255);
                SDL_RenderDrawRect(g_renderer, &contextMenuItems[i].rect);
                
                // 绘制图标
                SDL_Rect iconRect = {
                    contextMenuItems[i].rect.x + 10,
                    contextMenuItems[i].rect.y + 10,
                    20,
                    20
                };
                SDL_RenderCopy(g_renderer, contextMenuItems[i].icon, NULL, &iconRect);
                
                // 绘制文本
                SDL_Color textColor = {0, 0, 0, 255};
                DrawText(contextMenuItems[i].rect.x + 40, 
                        contextMenuItems[i].rect.y + 12, 
                        contextMenuItems[i].text, 
                        textColor);
            }
        }

}
// 生成随机名称（3个字符）
string GenerateRandomName() {
    string name;
    for (int i = 0; i < 8; i++) {
        // 随机小写字母
        char c = 'a' + rand() % 26;
        name += c;
    }
    return name;
}
void OpenInTerminal(file_system_manager *fileSystem) {
    // 获取当前资源管理器路径
    string path = resWin.current_directory;
    
    // 打开CMD窗口并切换到该路径
    cmdWin.cmdOpen = true;
    cmdWin.cmdMinimized = false;
    cmdWin.current_directory = path;
    fileSystem->command_cd(path); // 后端接口
    g_log_buffer.clear();
    g_log_color_buffer.clear();
    history.clear();
    currentCmd.clear();
    // 将CMD窗口置顶
    windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), CMD_WINDOW));
    windowOrder.push_back(CMD_WINDOW);
    activeWindow = CMD_WINDOW;
}

// 创建随机名称的文本文件
void CreateTextFile(file_system_manager *fileSystem) {
    // 生成随机文件名
    string filename = GenerateRandomName() + ".txt";
    string origin=fileSystem->get_cur_dir();
    fileSystem->command_cd(resWin.current_directory);    
    // 调用后端接口创建文件
    fileSystem->command_touch(filename);
    // 刷新资源管理器显示
    resWin.fileList=return_filelist(fileSystem);
    fileSystem->command_cd(origin);
    g_log_buffer.clear();
    g_log_color_buffer.clear();
}

// 创建随机名称的文件夹
void CreateFolder(file_system_manager *fileSystem) {
    // 生成随机文件夹名
    string foldername = GenerateRandomName();
        
    // 调用后端接口创建文件夹
    string origin=fileSystem->get_cur_dir();
    fileSystem->command_cd(resWin.current_directory);
    fileSystem->command_mkdir(foldername);
    // 刷新资源管理器显示
    resWin.fileList=return_filelist(fileSystem);
    fileSystem->command_cd(origin);
    g_log_buffer.clear();
    g_log_color_buffer.clear();
}

// 处理窗口拖动和按钮点击[8,10](@ref)
void HandleTitleBarClick(int x, int y,file_system_manager *fileSystem) {
    // 计算按钮区域
    if(activeWindow != CMD_WINDOW) return;
    int btnAreaStart = cmdWin.x + cmdWin.CMD_WIDTH - 3*(BTN_SIZE_CMD+5);
    if (x >= btnAreaStart && x <= cmdWin.x + cmdWin.CMD_WIDTH) {
        int btnIndex = (x - btnAreaStart) / (BTN_SIZE_CMD + 5);
        if (btnIndex == 0) {       // 最小化按钮
            cmdWin.cmdMinimized = true;
            windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), RES_WINDOW));
            windowOrder.push_back(RES_WINDOW);
            activeWindow = RES_WINDOW;
        }
        else if (btnIndex == 1) {  // 最大化按钮
        cmdWin.isMaximized = !cmdWin.isMaximized;
        if (cmdWin.isMaximized) {
            int winW, winH;
            SDL_GetWindowSize(g_window, &winW, &winH);
            cmdWin.CMD_HEIGHT =winH-2*TASKBAR_HEIGHT-6;
            cmdWin.CMD_WIDTH=winW;
            cmdWin.origin_cmdx=cmdWin.x;
            cmdWin.origin_cmdy=cmdWin.y;
            cmdWin.x = 0;
            cmdWin.y = 0; // 留出标题栏空间[2,5](@ref)
        } else {
            // 恢复原始尺寸
            cmdWin.CMD_HEIGHT =610;
            cmdWin.CMD_WIDTH=910;
            cmdWin.x = cmdWin.origin_cmdx; // 恢复默认坐标
            cmdWin.y = cmdWin.origin_cmdy;
        }
     }
        else if (btnIndex == 2) {  // 关闭按钮
            //fileSystem->Exit();
            cmdWin.cmdOpen = false;        // 关闭时才隐藏窗口
            cmdWin.cmdMinimized = false;   // 同时清除最小化状态
            cmdWin.isMaximized=false;
            windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), RES_WINDOW));
            windowOrder.push_back(RES_WINDOW);
            activeWindow = RES_WINDOW;
        }
        return;
    }

    // 开始拖动窗口
    cmdWin.isDragging = true;
    cmdWin.dragStart = {x - cmdWin.x, y - cmdWin.y};
}
void HandleResTitleBarClick(int x, int y) {
    if(activeWindow != RES_WINDOW) return;
// 计算各按钮的实际位置
    const int BUTTON_SPACING = 30;
    const int BUTTON_RIGHT_MARGIN = 5;
    const int HIT_AREA_PADDING = 15;  // 每个方向扩展5像素
    const int BUTTON_HIT_WIDTH = BTN_SIZE_RES + HIT_AREA_PADDING*2;
    const int BUTTON_HIT_HEIGHT = BTN_SIZE_RES + HIT_AREA_PADDING*2;
    int winWidth = resWin.isMaximized ? SCREEN_WIDTH : resWin.RES_WIDTH;
    SDL_Rect closeRect = {
        resWin.x + winWidth - BTN_SIZE_RES - BUTTON_RIGHT_MARGIN,
        resWin.y + (TITLE_BAR_HEIGHT - BTN_SIZE_RES) / 2,
        BTN_SIZE_RES,
        BTN_SIZE_RES
    };

    SDL_Rect maximizeRect = {
        closeRect.x - BTN_SIZE_RES - BUTTON_SPACING,
        closeRect.y,
        BTN_SIZE_RES,
        BTN_SIZE_RES
    };

    SDL_Rect minimizeRect = {
        maximizeRect.x - BTN_SIZE_RES - BUTTON_SPACING,
        maximizeRect.y,
        BTN_SIZE_RES,
        BTN_SIZE_RES
    };
    // 计算扩展后的按钮区域
    SDL_Rect closeHitRect = {
        closeRect.x - HIT_AREA_PADDING,
        closeRect.y - HIT_AREA_PADDING,
        BUTTON_HIT_WIDTH,
        BUTTON_HIT_HEIGHT
    };

    SDL_Rect maximizeHitRect = {
        maximizeRect.x - HIT_AREA_PADDING,
        maximizeRect.y - HIT_AREA_PADDING,
        BUTTON_HIT_WIDTH,
        BUTTON_HIT_HEIGHT
    };

    SDL_Rect minimizeHitRect = {
        minimizeRect.x - HIT_AREA_PADDING,
        minimizeRect.y - HIT_AREA_PADDING,
        BUTTON_HIT_WIDTH,
        BUTTON_HIT_HEIGHT
    };

    SDL_Point mousePos = {x, y};
    
    if(SDL_PointInRect(&mousePos, &closeHitRect)) {
        resWin.resOpen = false;
        resWin.resMinimized = false;
        resWin.isMaximized=false;
        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), CMD_WINDOW));
        windowOrder.push_back(CMD_WINDOW);
        activeWindow = CMD_WINDOW;
    } 
    else if(SDL_PointInRect(&mousePos, &maximizeHitRect)) {
        resWin.isMaximized = !resWin.isMaximized;
        // 处理窗口最大化逻辑
        if(resWin.isMaximized) {
            resWin.originX = resWin.x;
            resWin.originY = resWin.y;
            int winW, winH;
            SDL_GetWindowSize(g_window, &winW, &winH);
            resWin.RES_WIDTH = winW;
            resWin.RES_HEIGHT = winH - TASKBAR_HEIGHT;
            resWin.x = 0;
            resWin.y = 0;
        } else {
            resWin.RES_WIDTH = 800;
            resWin.RES_HEIGHT = 600;
            resWin.x = resWin.originX;
            resWin.y = resWin.originY;
        }
    } 
    else if(SDL_PointInRect(&mousePos, &minimizeHitRect)) {
        resWin.resMinimized = true;
        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), CMD_WINDOW));
        windowOrder.push_back(CMD_WINDOW);
        activeWindow = CMD_WINDOW;
    } 
    else {
        // 处理窗口拖动
        resWin.isDragging = true;
        resWin.dragStart = {x - resWin.x, y - resWin.y};
    }
}
std::string trim(const std::string& str) {
    if (str.empty()) return str;
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    if (first == std::string::npos) return "";
    return str.substr(first, (last - first + 1));
}

// 主事件循环处理[6,8](@ref)
void HandleEvents(file_system_manager *fileSystem) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        SDL_Rect temp_rect={resWin.x,resWin.y,resWin.RES_WIDTH,resWin.RES_WIDTH};
        SDL_Rect temp_cmd={cmdWin.x,cmdWin.y,cmdWin.CMD_WIDTH,cmdWin.CMD_WIDTH};
        SDL_Rect exitIcon = {SCREEN_WIDTH - 40,SCREEN_HEIGHT - TASKBAR_HEIGHT + 5,30,30};
        int x,y;
        SDL_GetMouseState(&x, &y); // 关键：实时获取坐标
        SDL_Point mouse=SDL_Point{x,y};
        switch (e.type) {
            case SDL_QUIT: 
                fileSystem->Exit();
                exit(1);
                break;
            case SDL_MOUSEWHEEL:  // 新增滚轮事件

                if (resWin.resOpen && !resWin.resMinimized&&SDL_PointInRect(&mouse, &temp_rect)&&activeWindow == RES_WINDOW) {
                    resWin.scrollOffset -= e.wheel.y * 30; // 调整滚动速度
                }else if (cmdWin.cmdOpen && !cmdWin.cmdMinimized &&SDL_PointInRect(&mouse, &temp_cmd)&&activeWindow == CMD_WINDOW) {
                    cmdWin.scrollOffset -= e.wheel.y * 30; // 调整滚动速度
                }
                break;    
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_Rect upArrowArea = {resWin.x + 10,resWin.y + (TITLE_BAR_HEIGHT - 20)/2,20,20};
                    if (resWin.resOpen && !resWin.resMinimized&&SDL_PointInRect(&mouse, &temp_rect)&&activeWindow == RES_WINDOW) {
                        int scrollBarX = resWin.x + resWin.RES_WIDTH - 13;
                        if (e.button.x >= scrollBarX && e.button.x <= scrollBarX + 8) {
                            resWin.isScrolling = true;
                            resWin.scrollStartPos.y = e.button.y;
                        }
                    }else if(cmdWin.cmdOpen && !cmdWin.cmdMinimized &&SDL_PointInRect(&mouse, &temp_cmd)&&activeWindow == CMD_WINDOW){
                        if (e.button.button == SDL_BUTTON_LEFT) {
                            // 检查滚动条点击
                            int scrollX = cmdWin.x + cmdWin.CMD_WIDTH - 13;
                            if (e.button.x >= scrollX && e.button.x <= scrollX + 8) {
                                cmdWin.isScrolling = true;
                                cmdWin.scrollStartPos.y = e.button.y;
                            }
                        }
                    }
                    // 检测CMD图标点击
                    if (showResContextMenu) {
                        for (int i = 0; i < contextMenuItems.size(); i++) {
                            if (SDL_PointInRect(&mouse, &contextMenuItems[i].rect)) {
                                switch (i) {
                                    case 0: // 在终端中打开
                                        OpenInTerminal(fileSystem);
                                        break;
                                    case 1: // 创建文本文件
                                        CreateTextFile(fileSystem);
                                        break;
                                    case 2: // 创建文件夹
                                        CreateFolder(fileSystem);
                                        break;
                                }
                                showResContextMenu = false;
                                break;
                            }
                        }
                        
                        // 点击菜单外区域关闭菜单
                        SDL_Rect menuArea = {
                            contextMenuItems[0].rect.x,
                            contextMenuItems[0].rect.y,
                            contextMenuItems[0].rect.w,
                            (int)contextMenuItems.size() * 40
                        };
                        if (!SDL_PointInRect(&mouse, &menuArea)) {
                            showResContextMenu = false;
                        }
                    }
                    if(SDL_PointInRect(&mouse, &temp_cmd)&& cmdWin.cmdOpen && !cmdWin.cmdMinimized&&!SDL_PointInRect(&mouse, &temp_rect)){
                        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), CMD_WINDOW));
                        windowOrder.push_back(CMD_WINDOW);
                        activeWindow = CMD_WINDOW;
                    }
                    if (SDL_PointInRect(&mouse, &temp_rect)&& resWin.resOpen && !resWin.resMinimized&&!SDL_PointInRect(&mouse, &temp_cmd)) {
                        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), RES_WINDOW));
                        windowOrder.push_back(RES_WINDOW);
                        activeWindow = RES_WINDOW;
                    }
                    if (y >= SCREEN_HEIGHT - TASKBAR_HEIGHT && x >= 10 && x <= 50) {
                        if (cmdWin.cmdMinimized) {    // 最小化状态下点击恢复
                            cmdWin.cmdMinimized = false;
                            cmdWin.cmdOpen = true;
                        }
                        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), CMD_WINDOW));
                        windowOrder.push_back(CMD_WINDOW);
                        activeWindow = CMD_WINDOW;
                    }
                    if (x >= 10 && x <= 70 && y >= 10 && y <= 60&&!(cmdWin.cmdOpen && !cmdWin.cmdMinimized)&&!(SDL_PointInRect(&mouse, &temp_cmd)&& cmdWin.cmdOpen && !cmdWin.cmdMinimized)&&!(SDL_PointInRect(&mouse, &temp_rect)&& resWin.resOpen && !resWin.resMinimized)) {
                        if (cmdWin.cmdOpen==true&&cmdWin.cmdMinimized==true)
                        {
                            cmdWin.cmdMinimized=false;
                        }
                        cmdWin = {100, 50, false, false};
                        cmdWin.cmdOpen = true;
                        history.clear();
                        currentCmd.clear();
                        string temp="";
                        fileSystem->command_cd(temp);
                        g_log_buffer.clear();
                        g_log_color_buffer.clear();
                        //后端接口 复位工作目录
                        //cmdWin.current_directory得到复位后的根目录
                        cmd_history_index=user_history_cmd.size();
                        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), CMD_WINDOW));
                        windowOrder.push_back(CMD_WINDOW);
                        activeWindow = CMD_WINDOW;
                    }
                    // 检测标题栏点击
                    else if (cmdWin.cmdOpen && x >= cmdWin.x && x <= cmdWin.x+cmdWin.CMD_WIDTH&& y >= cmdWin.y && y <= cmdWin.y+TITLE_BAR_HEIGHT) {
                        HandleTitleBarClick(x, y,fileSystem);
                    }
                    SDL_Rect resTaskbarIcon = {60, SCREEN_HEIGHT - TASKBAR_HEIGHT + 5, 30, 30};
                    if (SDL_PointInRect(&mouse, &resTaskbarIcon)) {
                        if (resWin.resMinimized) {
                            resWin.resMinimized = false;
                            resWin.resOpen = true;
                        }
                        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), RES_WINDOW));
                        windowOrder.push_back(RES_WINDOW);
                        activeWindow = RES_WINDOW;
                    }
                    if (SDL_PointInRect(&mouse, &resIconRect)&&!(resWin.resOpen && !resWin.resMinimized)&&!(SDL_PointInRect(&mouse, &temp_cmd)&& cmdWin.cmdOpen && !cmdWin.cmdMinimized)&&!(SDL_PointInRect(&mouse, &temp_rect)&& resWin.resOpen && !resWin.resMinimized)) {
                        if (resWin.resOpen && resWin.resMinimized) { // 新增状态判断
                            resWin.resMinimized = false;      // 恢复最小化窗口
                        } else {
                            resWin = {400, 150, false, false}; // 新增窗口坐标初始化[2,6](@ref)
                            
                            ///TODO: 将resWin赋值
                            // 1. 调用command_cd(current_user);
                            // 2. command_ls(current_dir);     //// 传递给resWin.fileList（文件名+类型）
                            
                            string temp="";
                            string origin=fileSystem->get_cur_dir();
                            fileSystem->command_cd(temp);  //初始化之后的目录
                            //resWin.current_directory=通过后端得到初始化目录
                            resWin.current_directory=fileSystem->get_cur_dir();
                            resWin.resOpen = true;
                            resWin.fileList = return_filelist(fileSystem);//后端接口
                            fileSystem->command_cd(origin);
                            g_log_buffer.clear();
                            g_log_color_buffer.clear();
                            ///TODO: 
                           // fs->chdir()


                        }
                        windowOrder.erase(remove(windowOrder.begin(), windowOrder.end(), RES_WINDOW));
                        windowOrder.push_back(RES_WINDOW);
                        activeWindow = RES_WINDOW;
                    }
                    
                    // 处理资源管理器标题栏点击
                    if (resWin.resOpen && !resWin.resMinimized&& x >= resWin.x && x <= resWin.x+resWin.RES_WIDTH &&y >= resWin.y && y <= resWin.y+TITLE_BAR_HEIGHT) {
                        HandleResTitleBarClick(x, y); // 新增处理函数
                    }
                    if (resWin.resOpen && !resWin.resMinimized) {
                        // 遍历所有文件项坐标
                        for (size_t i = 0; i < resWin.itemRects.size(); ++i) 
                        {
                            const SDL_Rect& rect = resWin.itemRects[i].first;
                            if (SDL_PointInRect(&mouse, &rect)) {
                                const FileItem& item =resWin.itemRects[i].second;
                                if (item.type == 2) { // 文件夹类型
                                    // 构造绝对路径
                                    string newPath = resWin.current_directory;
                                    if (newPath.back() != '/') newPath += "/";
                                    newPath += item.name;
                                    resWin.current_directory=newPath;
                                    string origin=fileSystem->get_cur_dir();
                                    fileSystem->command_cd(resWin.current_directory);
                                    resWin.fileList=return_filelist(fileSystem);//后端接口
                                    fileSystem->command_cd(origin);
                                    g_log_buffer.clear();
                                    g_log_color_buffer.clear();
                                    resWin.scrollOffset = 0;
                                    resWin.contentHeight = 0;

                                }else if(item.type==3){
                                    //目录软链接还得另写逻辑
                                    string origin=fileSystem->get_cur_dir();
                                    string newPath = resWin.current_directory;
                                    if (newPath.back() != '/') newPath += "/";
                                    newPath += item.name;
                                    fileSystem->command_cd(newPath);
                                    resWin.current_directory=fileSystem->get_cur_dir();
                                    resWin.fileList=return_filelist(fileSystem);
                                    fileSystem->command_cd(origin);
                                    g_log_buffer.clear();
                                    g_log_color_buffer.clear();
                                }
                                break;
                            }
                        }
                        if (SDL_PointInRect(&mouse, &upArrowArea)) {
                            if(resWin.current_directory!=fileSystem->get_root_name()){ //后端接口，所有将根目录判断为“/”的地方都得改
                                // 获取父目录路径
                                size_t lastSlash = resWin.current_directory.find_last_of('/');
                                string parentDir;
                                
                                if (lastSlash == 0) {  // 处理类似 "/home" -> "/" 的情况
                                    parentDir = "/";
                                } else if (lastSlash != string::npos) {
                                    parentDir = resWin.current_directory.substr(0, lastSlash);
                                }
                                
                                // 更新目录并获取文件列表
                                if (!parentDir.empty()) {
                                    string origin=fileSystem->get_cur_dir();
                                    resWin.current_directory = parentDir;
                                    fileSystem->command_cd(resWin.current_directory);
                                    resWin.fileList = return_filelist(fileSystem);
                                    fileSystem->command_cd(origin);

                                    // 重置滚动状态
                                    resWin.scrollOffset = 0;
                                    resWin.contentHeight = 0;
                                }

                            }
                        }
                    }
                    if (SDL_PointInRect(&mouse, &exitIcon)&&!(SDL_PointInRect(&mouse, &temp_cmd)&& cmdWin.cmdOpen && !cmdWin.cmdMinimized)&&!(SDL_PointInRect(&mouse, &temp_rect)&& resWin.resOpen && !resWin.resMinimized)) {
                            // 返回登录界面
                            fileSystem->Exit();
                            showLogin = true;
                            // 清除系统界面状态
                            cmdWin.cmdOpen = false;
                            resWin.resOpen = false;
                        }
                    }else  if (e.button.button == SDL_BUTTON_RIGHT) {
                        // 如果在资源管理器窗口内右键点击
                        if (resWin.resOpen && !resWin.resMinimized && SDL_PointInRect(&mouse, &temp_rect) && activeWindow == RES_WINDOW) {
                            showResContextMenu = true;
                            resContextMenuPos = mouse;
                            resContextMenuSelectedItem = -1;
                            
                            // 初始化右键菜单项
                            contextMenuItems.clear();
                            contextMenuItems.push_back({"在终端中打开", g_cmdIcon});
                            contextMenuItems.push_back({"创建文本文件", g_txtIcon});
                            contextMenuItems.push_back({"创建文件夹", g_folderIcon});
                            
                            // 计算菜单位置（确保在窗口内）
                            int menuX = resContextMenuPos.x;
                            int menuY = resContextMenuPos.y;
                            int menuWidth = 180;
                            int menuHeight = contextMenuItems.size() * 40;
                            
                            // 调整位置防止超出窗口
                            if (menuX + menuWidth > resWin.x + resWin.RES_WIDTH) {
                                menuX = resWin.x + resWin.RES_WIDTH - menuWidth;
                            }
                            if (menuY + menuHeight > resWin.y + resWin.RES_HEIGHT) {
                                menuY = resWin.y + resWin.RES_HEIGHT - menuHeight;
                            }
                            
                            // 设置菜单项位置
                            for (int i = 0; i < contextMenuItems.size(); i++) {
                                contextMenuItems[i].rect = {
                                    menuX, 
                                    menuY + i * 40,
                                    menuWidth,
                                    40
                                };
                            }
                        }
                    }
                break;
            case SDL_MOUSEBUTTONUP:
                cmdWin.isScrolling = false;
                resWin.isScrolling = false;
                cmdWin.isDragging = false;
                resWin.isDragging = false;
                break;
                
            case SDL_MOUSEMOTION:
                            // 更新右键菜单悬停状态
                if (showResContextMenu) {
                    resContextMenuSelectedItem = -1;
                    for (int i = 0; i < contextMenuItems.size(); i++) {
                        if (SDL_PointInRect(&mouse, &contextMenuItems[i].rect)) {
                            resContextMenuSelectedItem = i;
                            break;
                        }
                    }
                }
                if (resWin.isScrolling) {  // 处理滚动条拖拽
                    int deltaY = e.motion.y - resWin.scrollStartPos.y;
                    resWin.scrollOffset += deltaY * 2; // 调整滚动灵敏度
                    resWin.scrollStartPos.y = e.motion.y;
                }
                if (cmdWin.isScrolling) {
                    int deltaY = e.motion.y - cmdWin.scrollStartPos.y;
                    cmdWin.scrollOffset += deltaY * 2;
                    cmdWin.scrollStartPos.y = e.motion.y;
                }
                if (activeWindow == CMD_WINDOW&&cmdWin.isDragging) {
                    cmdWin.x = e.motion.x - cmdWin.dragStart.x;
                    cmdWin.y = e.motion.y - cmdWin.dragStart.y;
                }
                if (activeWindow == RES_WINDOW&&resWin.isDragging) {
                    resWin.x = e.motion.x - resWin.dragStart.x;
                    resWin.y = e.motion.y - resWin.dragStart.y;
                }
                break;
                
            case SDL_TEXTINPUT:
                if(activeWindow == CMD_WINDOW){
                    currentCmd += e.text.text;
                    lastCursorToggle = SDL_GetTicks();
                    cmdWin.showCursor = true;
                }
                break;
                
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_RETURN) {
                    if (!currentCmd.empty()) {
                        Command cmd;
                        string Standardization_dir=TruncatePath(cmdWin.current_directory,cmdWin.CMD_WIDTH/4);
                        std::string trimmedCmd = trim(currentCmd); // 去除两端空格
                        bool isClearCommand = (trimmedCmd == "clear");
                        cmd.input = user_name+":"+Standardization_dir+"$ "+currentCmd;
                        //auto result = ExecuteCommand(currentCmd);//创建管道执行命令，并返回执行的结果 //后端接口
                        if(trimmedCmd=="clear"){
                            history.clear();
                            user_history_cmd.push_back(currentCmd);
                            cmd_history_index=user_history_cmd.size();
                            currentCmd.clear();
                            g_log_buffer.clear();
                            g_log_color_buffer.clear();
                        }else{
                            run_lexer(trimmedCmd);
                            auto result= g_log_buffer;
                            for (auto& str : result) {
                                std::replace(str.begin(), str.end(), '\n', ' '); 
                                // 将 \n 替换为空格
                            }
                            string origin=fileSystem->get_cur_dir();
                            fileSystem->command_cdKC(resWin.current_directory);
                            reset_res(fileSystem);
                            resWin.current_directory=fileSystem->get_cur_dir();
                            fileSystem->command_cd(origin);
                            cmdWin.current_directory=get_current_dir(fileSystem);
                            //后端接口 
                            vector<pair<string,int>>ans;
                            for(auto val:result){
                                ans.push_back({val,37});
                            }
                            if(ans.size()!=0){
                                cmd.output = ans;  // 替换原始输出
                            }
                            std::vector<std::pair<std::string, int>> ans1;
                            for (const auto& item : g_log_color_buffer) {
                                ans1.push_back({item.first, static_cast<int>(item.second)});
                            }
                            if(ans1.size()!=0){
                                cmd.output_ans = ans1;  // 替换原始输出
                            }
                            history.push_back(cmd);
                            user_history_cmd.push_back(currentCmd);
                            cmd_history_index=user_history_cmd.size();
                            currentCmd.clear();
                            g_log_buffer.clear();
                            g_log_color_buffer.clear();
                        }
                    }
                }
                else if (e.key.keysym.sym == SDLK_BACKSPACE && !currentCmd.empty()) {
                    currentCmd.pop_back();
                }// 上键处理
                else if (e.key.keysym.sym == SDLK_UP) {
                    if (!user_history_cmd.empty() && cmd_history_index > 0) {
                        cmd_history_index--;
                        currentCmd = user_history_cmd[cmd_history_index];
                    }
                }
                // 下键处理
                else if (e.key.keysym.sym == SDLK_DOWN) {
                    if (!user_history_cmd.empty() && cmd_history_index < user_history_cmd.size() - 1) {
                        cmd_history_index++;
                        currentCmd = user_history_cmd[cmd_history_index];
                    }
                }
                break;
        }
    }
}
void display_text_mid(string text,SDL_Rect rec){
    int textWidth = GetTextWidth(g_midFont, text);
    int textX = rec.x + (rec.w - textWidth)/2; 
    int textY = rec.y + rec.h ; 
    DrawText(textX, textY, text);

}
void run_system_front(file_system_manager *fileSystem){
        HandleEvents(fileSystem);
        // 绘制背景
        SDL_RenderCopy(g_renderer, g_background, NULL, NULL);
        // 绘制CMD图标
        SDL_RenderCopy(g_renderer, g_cmdIcon, NULL, &iconRect);
        SDL_RenderCopy(g_renderer, g_resourceIcon, NULL, &resIconRect);
        display_text_mid("CMD",iconRect);
        display_text_mid(u8"资源管理器",resIconRect);
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastCursorToggle > CURSOR_BLINK_INTERVAL) {
            cmdWin.showCursor = !cmdWin.showCursor;
            lastCursorToggle = currentTime;
        }
        for (auto type : windowOrder) {
            if(type == CMD_WINDOW && cmdWin.cmdOpen) DrawCMDWindow();
            if(type == RES_WINDOW && resWin.resOpen) DrawResWindow(); 
        }
        DrawTaskbar(); 
        SDL_RenderPresent(g_renderer);
}
bool InitializeGlobalResources() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL初始化失败: %s", SDL_GetError());
        return false;
    }
    
    if (TTF_Init() < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_ttf初始化失败: %s", TTF_GetError());
        return false;
    }
    // exit(1);

    // 创建窗口
    g_res.window.reset(SDL_CreateWindow("File_system System", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI));
    if (!g_res.window) {return false;}

//////////////////////
    // 创建渲染器
      
    g_res.renderer.reset(SDL_CreateRenderer(g_res.window.get(), -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));

    if (!g_res.renderer) {

      return false;}



    // 创建离屏纹理
    g_res.offscreenTexture.reset(SDL_CreateTexture(g_res.renderer.get(),
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        SCREEN_WIDTH, SCREEN_HEIGHT));
    if (!g_res.offscreenTexture) return false;

    // exit(1);
    // 加载纹理资源
    g_res.loginTexture.reset(IMG_LoadTexture(g_res.renderer.get(), "../img/login_background.jpg"));
    g_res.userIconTexture.reset(IMG_LoadTexture(g_res.renderer.get(), "../img/Users_icon.png"));
    if (!g_res.loginTexture || !g_res.userIconTexture) return false;

    // 设置图标纹理混合模式
    SDL_SetTextureBlendMode(g_res.userIconTexture.get(), SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(g_res.userIconTexture.get(), 200);
    // 加载字体
    for (const auto& path : FONT_PATHS) {
        // 主标题字体
        g_res.font_title.reset(TTF_OpenFont(path.c_str(), 24));
        // 输入框标签字体
        g_res.font_label.reset(TTF_OpenFont(path.c_str(), 20));
        // 按钮字体
        g_res.font_button.reset(TTF_OpenFont(path.c_str(), 20));
        TTF_SetFontStyle(g_res.font_button.get(), TTF_STYLE_BOLD);
        
        if (g_res.font_title && g_res.font_label && g_res.font_button) {
            break;
        }
    }
    // 主字体（用于标题）
    for (const auto& path : FONT_PATHS) {
        g_res.font.reset(TTF_OpenFont(path.c_str(), 24));
        if (g_res.font) break;
    }

    return  g_res.renderer && g_res.font;
}

void DrawCircle(SDL_Renderer* renderer, int x0, int y0, int radius, SDL_Color color) {
    const int n = 100; // number of triangles used to draw the circle
    SDL_Vertex vertices[n + 1];
    for (int i = 0; i <= n; i++) {
        float angle = 2 * M_PI * i / n;
        vertices[i].position.x = x0 + radius * cos(angle);
        vertices[i].position.y = y0 + radius * sin(angle);
        vertices[i].color = color;
    }
    int indices[3 * n];
    for (int i = 0; i < n; i++) {
        indices[3 * i] = 0;
        indices[3 * i + 1] = i + 1;
        indices[3 * i + 2] = (i + 1) % n + 1;
    }
    SDL_RenderGeometry(renderer, NULL, vertices, n + 1, indices, 3 * n);
}

void DrawRoundedRect(SDL_Renderer* renderer, int x, int y, int w, int h, int radius) {
    SDL_Color color = { 227, 237, 249, 255 };
    DrawCircle(renderer, x, y + h / 2, h / 2, color);
    DrawCircle(renderer, x + w, y + h / 2, h / 2, color);
    SDL_Rect areas[] = {
        {x, y, w, h}
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (auto& rect : areas) {
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // 恢复默认模式
}
void DrawRoundedRect_Dynamic(SDL_Renderer* renderer, int x, int y, int w, int h, int radius, SDL_Color color) {
    DrawCircle(renderer, x, y + h / 2, h / 2, color);
    DrawCircle(renderer, x + w, y + h / 2, h / 2, color);
    SDL_Rect areas[] = {
        {x, y, w, h}
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (auto& rect : areas) {
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // 恢复默认模式
}
void RenderText(SDL_Renderer* renderer, TTF_Font* font, const char* text,
    SDL_Color color, int x, int y) {
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>
        surface(TTF_RenderUTF8_Blended(font, text, color), SDL_FreeSurface);

    if (surface) {
        std::unique_ptr<SDL_Texture, SDL_Deleter>
            texture(SDL_CreateTextureFromSurface(renderer, surface.get()), SDL_Deleter());

        SDL_Rect dest = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture.get(), NULL, &dest);
    }
}
void RenderAlert(SDL_Renderer* renderer, TTF_Font* font, AlertMessage& msg) {
    if (SDL_GetTicks() - msg.startTime > msg.duration) return;

    // 计算渐变透明度
    float progress = (SDL_GetTicks() - msg.startTime) / (float)msg.duration;
    SDL_Color fadeColor = {
        msg.color.r,
        msg.color.g,
        msg.color.b,
        static_cast<Uint8>(255 * (1 - progress)) // 透明度渐变
    };

    // 居中显示
    int textWidth = 0;
    TTF_SizeText(font, msg.text.c_str(), &textWidth, nullptr);
    int xPos = (SCREEN_WIDTH - textWidth) / 2;

    RenderText(renderer, font, msg.text.c_str(), fadeColor, xPos, msg.yPos);
}
void mouse_handle(bool &running,SDL_Event &event){
            while (SDL_PollEvent(&event)) {
            SDL_Rect passwordRect = { inputRect.x, inputRect.y + 80, inputRect.w, inputRect.h };
            if (event.type == SDL_QUIT) {
                exit(1);
                break;  // 立即退出循环
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point mousePos = { event.button.x, event.button.y };

                // 检测用户名输入框点击
                if (SDL_PointInRect(&mousePos, &inputRect)) {
                    is_username_active = true;
                    SDL_StartTextInput();
                }
                // 检测密码输入框点击
                else if (SDL_PointInRect(&mousePos, &passwordRect)) {
                    is_username_active = false;
                    SDL_StartTextInput();
                }
                else if (SDL_PointInRect(&mousePos, &buttonRect)) {
                    running=!fileSystem->CMPuser(user_in_name,password);//后端接口
                    if(running){
                        alert = { "Wrong username or password", SDL_Color{255,0,0,255}, 2000, SDL_GetTicks(), inputRect.y - 30 };
                    }else{
                        user_name=user_in_name;//得同步修改
                    }
                    //点击登入的实现
                    SDL_Log("Attempt login: User=%s, Pass=%s", user_in_name.c_str(), password.c_str());
                    user_in_name.clear();
                    password.clear();
                }
                else {
                    SDL_StopTextInput();
                }
                cursor_blink_time = SDL_GetTicks(); // 重置光标计时
            }
            // 文本输入事件
            else if (event.type == SDL_TEXTINPUT) {
                const char* input = event.text.text;
                bool overLimit = false;
                if (is_username_active) {
                    int remaining = 16 - user_in_name.length();
                    if (remaining > 0) {
                        user_in_name.append(input, std::min(remaining, (int)strlen(input)));
                    }
                    else {
                        overLimit = true;
                        alert = { "User name length exceeds the limit!", SDL_Color{255,0,0,255}, 2000, SDL_GetTicks(), inputRect.y - 30 };
                    }
                }
                else {
                    int remaining = 32 - password.length();
                    if (remaining > 0) {
                        password.append(input, std::min(remaining, (int)strlen(input)));
                    }
                    else {
                        overLimit = true;
                        alert = { "Password length exceeds limit", SDL_Color{255,0,0,255}, 2000, SDL_GetTicks(), inputRect.y + 130 };
                    }
                }
                if (!overLimit) {
                    cursor_blink_time = SDL_GetTicks();
                }
            }
            // 特殊按键处理
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    if (is_username_active && !user_in_name.empty()) {
                        user_in_name.pop_back();
                    }
                    else if (!is_username_active && !password.empty()) {
                        password.pop_back();
                    }
                    cursor_blink_time = SDL_GetTicks();
                }
                else if (event.key.keysym.sym == SDLK_RETURN) {
                    // 回车键提交登录
                    running=!fileSystem->CMPuser(user_in_name,password);//后端接口
                    if(running){
                        alert = { "Wrong username or password", SDL_Color{255,0,0,255}, 2000, SDL_GetTicks(), inputRect.y - 30 };
                    }else{
                        user_name=user_in_name;//得同步修改
                    }
                    SDL_Log("Attempt login: User=%s, Pass=%s", user_in_name.c_str(), password.c_str());
                    user_in_name.clear();
                    password.clear();
                }
            }
            if (event.type == SDL_MOUSEMOTION) {
                SDL_Point mousePos = { event.motion.x, event.motion.y };
                is_button_hovered = SDL_PointInRect(&mousePos, &buttonRect) ? true : false;
            }
        }
}
void flush_new_figure(){
for (auto& p : particles) {
            p.position.x += p.velocity.x;
            p.position.y += p.velocity.y;

            // 边界反弹
            if (p.position.x < 0 || p.position.x > SCREEN_WIDTH)
                p.velocity.x *= -1;
            if (p.position.y < 0 || p.position.y > SCREEN_HEIGHT)
                p.velocity.y *= -1;
        }

        SDL_SetRenderTarget(g_res.renderer.get(), g_res.offscreenTexture.get());

        // 清空离屏缓冲区
        SDL_SetRenderDrawColor(g_res.renderer.get(), 0, 0, 0, 255);
        SDL_RenderClear(g_res.renderer.get());

        // 绘制渐变背景
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            float ratio = y / static_cast<float>(SCREEN_HEIGHT);
            SDL_SetRenderDrawColor(g_res.renderer.get(),
                BG_TOP.r + (BG_BOTTOM.r - BG_TOP.r) * ratio,
                BG_TOP.g + (BG_BOTTOM.g - BG_TOP.g) * ratio,
                BG_TOP.b + (BG_BOTTOM.b - BG_TOP.b) * ratio,
                255);
            SDL_RenderDrawLine(g_res.renderer.get(), 0, y, SCREEN_WIDTH, y);
        }

        // 批量绘制粒子
        SDL_SetRenderDrawBlendMode(g_res.renderer.get(), SDL_BLENDMODE_ADD);
        std::vector<SDL_Point> particlePoints;
        for (const auto& p : particles) {
            for (int i = 0; i < p.size; ++i) {
                for (int j = 0; j < p.size; ++j) {
                    particlePoints.push_back({
                    static_cast<int>(p.position.x) + i,
                    static_cast<int>(p.position.y) + j
                        });
                }
            }
        }
        SDL_RenderDrawPoints(g_res.renderer.get(), particlePoints.data(), particlePoints.size());
        SDL_SetRenderDrawBlendMode(g_res.renderer.get(), SDL_BLENDMODE_NONE);
         // 半透明背景（启用混合模式）
        SDL_SetRenderDrawBlendMode(g_res.renderer.get(), SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(g_res.loginTexture.get(), SDL_BLENDMODE_BLEND); // 新增
        SDL_SetTextureAlphaMod(g_res.loginTexture.get(), 128); // 新增
        SDL_RenderCopy(g_res.renderer.get(), g_res.loginTexture.get(), NULL, &loginBox);
        SDL_SetRenderDrawBlendMode(g_res.renderer.get(), SDL_BLENDMODE_NONE); // 恢复默认模式
        // 绘制标题
        RenderText(g_res.renderer.get(), g_res.font.get(), "Login System",TEXT_COLOR,loginBox.x + 120,loginBox.y + 30);

        SDL_RenderCopy(g_res.renderer.get(), g_res.userIconTexture.get(), NULL, &head_sculpture_iconRect);

        SDL_SetRenderDrawBlendMode(g_res.renderer.get(), SDL_BLENDMODE_BLEND);
        // 用户名输入框
        SDL_SetRenderDrawColor(g_res.renderer.get(), 255, 255, 255, 200);
        DrawRoundedRect(g_res.renderer.get(), inputRect.x, inputRect.y, inputRect.w, inputRect.h, 10);
        // 显示输入文本或提示
        const std::string& user_display = user_in_name.empty() ? "user name" : user_in_name;
        RenderText(g_res.renderer.get(), g_res.font_label.get(), user_display.c_str(), TEXT_COLOR, inputRect.x + 10, inputRect.y + 10);

        // 密码输入框
        SDL_SetRenderDrawColor(g_res.renderer.get(), BOX_COLOR.r, BOX_COLOR.g, BOX_COLOR.b, BOX_COLOR.a);
        DrawRoundedRect(g_res.renderer.get(), inputRect.x, inputRect.y + 80, inputRect.w, inputRect.h, 10);
        // 密码显示为星号
        std::string pass_display(password.size(), '*');
        if (password.empty()) pass_display = "password";
        RenderText(g_res.renderer.get(), g_res.font_label.get(), pass_display.c_str(), TEXT_COLOR, inputRect.x + 10, inputRect.y + 10 + 80);

        // 光标绘制逻辑（添加在渲染文本后）
        Uint32 current_time = SDL_GetTicks();
        if ((current_time - cursor_blink_time) % 1000 < 500) {
            if (is_username_active && !user_in_name.empty()) {
                int text_width = 0;
                TTF_SizeText(g_res.font_label.get(), user_in_name.c_str(), &text_width, nullptr);
                SDL_Rect cursor = { inputRect.x + 10 + text_width, inputRect.y + 15, 2, 20 };
                SDL_SetRenderDrawColor(g_res.renderer.get(), 0, 0, 0, 255);
                SDL_RenderFillRect(g_res.renderer.get(), &cursor);
            }
            else if (!is_username_active && !password.empty()) {
                int text_width = 0;
                TTF_SizeText(g_res.font_label.get(), pass_display.c_str(), &text_width, nullptr);
                SDL_Rect cursor = { inputRect.x + 10 + text_width, inputRect.y + 95, 2, 20 };
                SDL_SetRenderDrawColor(g_res.renderer.get(), 0, 0, 0, 255);
                SDL_RenderFillRect(g_res.renderer.get(), &cursor);
            }
        }
        SDL_Color login_color = { 255,255,255,255 };
        if (is_button_hovered)
        {
            login_color = { 218,232,252,255 };
        }
        DrawRoundedRect_Dynamic(g_res.renderer.get(), buttonRect.x, buttonRect.y, buttonRect.w, buttonRect.h, 25, login_color);
        RenderText(g_res.renderer.get(), g_res.font_button.get(), "Login", TEXT_COLOR, buttonRect.x + 40, buttonRect.y + 10);
        SDL_SetRenderTarget(g_res.renderer.get(), nullptr);
        SDL_RenderCopy(g_res.renderer.get(), g_res.offscreenTexture.get(), NULL, NULL);
        RenderAlert(g_res.renderer.get(), g_res.font_label.get(), alert);
        SDL_RenderPresent(g_res.renderer.get());
        SDL_Delay(10);
}
void INIT_LOGIN_SYSYTM(){
    std::srand(std::time(nullptr));
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init(); 

    if (!InitializeGlobalResources()) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "全局资源初始化失败");
        exit(0);
    }
    SDL_SetTextureBlendMode(g_res.userIconTexture.get(), SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(g_res.userIconTexture.get(), 200);
    for (auto& p : particles) {
        p.position = {
            static_cast<float>(std::rand() % SCREEN_WIDTH),
            static_cast<float>(std::rand() % SCREEN_HEIGHT)
        };
        p.velocity = {
            (std::rand() % 100 - 50) / 50.0f * PARTICLE_SPEED,
            (std::rand() % 100 - 50) / 50.0f * PARTICLE_SPEED
        };
        p.size = 2 + std::rand() % 5;
        p.alpha = 50 + std::rand() % 155;
    }
}