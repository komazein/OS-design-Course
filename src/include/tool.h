#pragma once
#include <string>
#include <vector>
#include "fs_types.h"

// Color 枚举定义保留在头文件�?
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

// �?�? printColor �? inline 函数，可以保留定义在头文�?
inline void printColor(const std::string &text, Color color = Color::Default, bool bold = false) {
    std::cout << "\033[" << (bold ? "1;" : "") << static_cast<int>(color) << "m"
              << text << "\033[0m" << "  ";
}

// 其余函数�?写声�?
void printLsResult(const std::string &name, TYPE type, int resultNum);
void printPath(std::vector<std::string> &path);
void showHelpMessage();
