#pragma once
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include "tool.h"



template<typename Mutex>
class vector_sink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit vector_sink(std::vector<std::string>& log_buffer) 
        : log_buffer_(log_buffer) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        // 格式化日志消息
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        // 存储到 vector
        log_buffer_.push_back(fmt::to_string(formatted));
    }

    void flush_() override {
        // 如果需要刷新（如写入文件），可以在这里实现
    }

private:
    std::vector<std::string>& log_buffer_; // 引用外部 vector
};

using vector_sink_st = vector_sink<spdlog::details::null_mutex>; // 单线程版本

extern std::vector<std::string> g_log_buffer;  // 存储日志的容器
extern std::shared_ptr<spdlog::logger> g_logger;  // 全局 logger
extern std::vector<std::pair<std::string, Color>> g_log_color_buffer; // 存储日志的容器，包含颜色信息

void init_global_logger(); 