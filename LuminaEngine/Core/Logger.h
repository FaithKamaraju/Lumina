//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once

#include "spdlog/spdlog.h"


namespace LE {
    class Logger {

        using LoggerPtr = std::shared_ptr<spdlog::logger>;

    private:
        Logger();

        static LoggerPtr s_CoreLogger;
        static LoggerPtr s_ClientLogger;

        static std::mutex s_CoreMutex;
        static std::mutex s_ClientMutex;

    public:
        static void init();
        static LoggerPtr& GetCoreLogger();
        static LoggerPtr& GetClientLogger();


        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger&) = delete;
    };
}

#ifndef NDEBUG
    #define LE_ASSERT(...)             assert(__VA_ARGS__)
    #define LE_CORE_TRACE(...)      ::LE::Logger::GetCoreLogger()->trace(__VA_ARGS__)
    #define LE_CORE_INFO(...)       ::LE::Logger::GetCoreLogger()->info(__VA_ARGS__)
    #define LE_CORE_WARN(...)       ::LE::Logger::GetCoreLogger()->warn(__VA_ARGS__)
    #define LE_CORE_ERROR(...)      ::LE::Logger::GetCoreLogger()->error(__VA_ARGS__)
    #define LE_CORE_CRITICAL(...)   ::LE::Logger::GetCoreLogger()->critical(__VA_ARGS__)

    #define LE_TRACE(...)           ::LE::Logger::GetClientLogger()->trace(__VA_ARGS__)
    #define LE_INFO(...)            ::LE::Logger::GetClientLogger()->info(__VA_ARGS__)
    #define LE_WARN(...)            ::LE::Logger::GetClientLogger()->warn(__VA_ARGS__)
    #define LE_ERROR(...)           ::LE::Logger::GetClientLogger()->error(__VA_ARGS__)
    #define LE_CRITICAL(...)        ::LE::Logger::GetClientLogger()->critical(__VA_ARGS__)
#else
    #define LE_ASSERT(...)
    #define LE_CORE_TRACE(...)
    #define LE_CORE_INFO(...)
    #define LE_CORE_WARN(...)
    #define LE_CORE_ERROR(...)
    #define LE_CORE_CRITICAL(...)

    #define LE_TRACE(...)
    #define LE_INFO(...)
    #define LE_WARN(...)
    #define LE_ERROR(...)
    #define LE_CRITICAL(...)
#endif


