#pragma once

#ifndef DISABLE_LOGGING

#include <spdlog/spdlog.h>

#define LOG_TRACE(...)		spdlog::trace(__VA_ARGS__)
#define LOG_INFO(...)		spdlog::info(__VA_ARGS__)
#define LOG_DEBUG(...)		spdlog::debug(__VA_ARGS__)
#define LOG_WARN(...)		spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...)		spdlog::error(__VA_ARGS__)
#define LOG_CRITICAL(...)	spdlog::critical(__VA_ARGS__)

#define LOG_SET_LEVEL(level)spdlog::set_level(level)
#define LOG_LEVEL_TRACE		spdlog::level::trace
#define LOG_LEVEL_DEBUG		spdlog::level::debug
#define LOG_LEVEL_INFO		spdlog::level::info
#define LOG_LEVEL_WARN		spdlog::level::warn
#define LOG_LEVEL_CRITICAL	spdlog::level::critical

#else

#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_DEBUG(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_CRITICAL(...)

#define LOG_SET_LEVEL(level)
#define LOG_LEVEL_TRACE
#define LOG_LEVEL_DEBUG
#define LOG_LEVEL_INFO
#define LOG_LEVEL_WARN
#define LOG_LEVEL_CRITICAL

#endif
