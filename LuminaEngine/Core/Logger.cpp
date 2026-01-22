//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"

LE::Logger::LoggerPtr LE::Logger::s_CoreLogger = nullptr;
LE::Logger::LoggerPtr LE::Logger::s_ClientLogger = nullptr;

std::mutex LE::Logger::s_CoreMutex;
std::mutex LE::Logger::s_ClientMutex;

LE::Logger::Logger() {
    init();
}

void LE::Logger::init() {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("%^[%T] %n: %v%$");
}

LE::Logger::LoggerPtr & LE::Logger::GetCoreLogger() {
    std::lock_guard<std::mutex> lock(s_CoreMutex);
    if (s_CoreLogger == nullptr) {
        s_CoreLogger = spdlog::stdout_color_mt("Core");
    }
    return s_CoreLogger;
}

LE::Logger::LoggerPtr & LE::Logger::GetClientLogger() {
    std::lock_guard<std::mutex> lock(s_ClientMutex);
    if (s_ClientLogger == nullptr) {
        s_ClientLogger = spdlog::stdout_color_mt("Client");
    }
    return s_ClientLogger;
}
