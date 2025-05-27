#pragma once
#include <cstdio>
#include "fs_types.h"


// 支持的颜色枚举
enum class Color {
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37,
    Default = 0
};

// 输出带颜色的字符串
/**
 * * @brief 打印带颜色的文本
 * * @param text 要打印的文本
 * * @param color 颜色枚举值，默认为默认颜色
 * * @param bold 是否加粗，默认为false
 */
void printColor(const std::string &text, Color color = Color::Default, bool bold = false) {
    std::cout << "\033[" << (bold ? "1;" : "") << static_cast<int>(color) << "m"
              << text << "\033[0m" << "  ";
}

void printLsResult(const std::string &name, TYPE type, int resultNum) {
    if (type == DIR) {
        printColor(name, Color::Blue, true);
    } else if (type == SIM_FILE) {
        printColor(name, Color::Green, false);
    } else {
        printColor(name, Color::Default, false);
    }
    if(!(resultNum%10))  cout << endl;

}


void printPath(vector<string> &path)
{
    if (path.empty()) {
        spdlog::warn("Path is empty, no path to print.");
        return;
    }

    cout << "The Paths of the found file:" << endl;
    for(auto & p : path) {
        cout << p << endl;
    }
    
}

void showHelpMessage() {
    std::cout << "Available commands:\n";
    std::cout << "mkdir <dirname>      - Create a new directory\n";
    std::cout << "cd <dirname>         - Change current directory\n";
    std::cout << "ls                   - List contents of the current directory\n";
    std::cout << "rm <dirname>         - Remove a directory\n";
    std::cout << "lkdir <dirname>      - Create a symbolic link to a directory (not implemented)\n";
    std::cout << "lndir <dirname>      - Create a hard link to a directory (not implemented)\n";
    std::cout << "find <filename>      - Find a file in the current directory and its subdirectories\n";
    std::cout << "touch <filename>     - Create a new file (not implemented)\n";
    std::cout << "delete <filename>    - Delete a file (not implemented)\n";
    std::cout << "cat <filename>       - Read the contents of a file (not implemented)\n";
    std::cout << "exit                 - Exit the shell\n";
}