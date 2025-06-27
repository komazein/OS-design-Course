#include "log_global.h"

void init_global_logger() {
    // 创建自定义 sink
    auto sink = std::make_shared<vector_sink_st>(g_log_buffer);
    sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    // 初始化全局 logger
    g_logger = std::make_shared<spdlog::logger>("global_logger", sink);
    spdlog::register_logger(g_logger);
    g_logger->set_level(spdlog::level::info);  // 设置日志级别
}