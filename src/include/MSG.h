#pragma once
#include "fs_types.h"
#include "tool.h"
#include <vector>
#include <string>
#include <sstream>
#include <utility>

enum MSGTYPE {
    WARN,
    INFO,
    DEBUG,
    ERROR
};

class MSGSTR {
private:
    std::vector<std::pair<std::string, Color>> msgstr;

    template<typename T>
    std::string toString(T&& value) {
        std::ostringstream oss;
        oss << std::forward<T>(value);  // 修复：实际输出值
        return oss.str();
    }

public:
void trans(MSGTYPE type)
{
    if(type==ERROR)
        msgstr.push_back({"[ERROR]:",Color::Red});
    if(type==WARN)
        msgstr.push_back({"[WARN]:",Color::Yellow});
    if(type==DEBUG)
        msgstr.push_back({"[DEBUG]:",Color::Magenta});
    if(type==INFO)
        msgstr.push_back({"[INFO]:",Color::Magenta});
    return;
}
    vector<pair<string,Color>> RETstr()
{
    return msgstr;
}
void clean()
{
    msgstr.clear();
    return;
}
void msgdirentry(string filename,TYPE filetype)
{
    if(filetype==DIR)
        msgstr.push_back({filename,Color::Blue});
    else
        msgstr.push_back({filename,Color::Green});
    return;
}

    // 带日志级别的版本
    template<typename... Args>
    void msglog(MSGTYPE msg, const std::string& base, Args&&... args) {
        trans(msg);
        msglog_impl(base, std::forward<Args>(args)...);
    }

    // 不带日志级别的版本
    template<typename... Args>
    void msglog(const std::string& base, Args&&... args) {
        msglog_impl(base, std::forward<Args>(args)...);
    }

    // 终止条件
    void msglog(const std::string& base) {
        msgstr.emplace_back(base, Color::White);
    }
    void msglog(MSGTYPE msg,const std::string& base) {
        trans(msg);
        msgstr.emplace_back(base, Color::White);
    }

private:
    // 实现细节
    template<typename T, typename... Args>
    void msglog_impl(std::string base, T&& first, Args&&... args) {
        size_t pos = base.find("{}");
        if (pos != std::string::npos) {
            base.replace(pos, 2, toString(std::forward<T>(first)));
            msglog_impl(base, std::forward<Args>(args)...);
        } else {
            msglog(base);
        }
    }

    // 终止条件
    void msglog_impl(const std::string& base) {
        msglog(base);
    }
};

extern MSGSTR* msg;  // 全局变量声明