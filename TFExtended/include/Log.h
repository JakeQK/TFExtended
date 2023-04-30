#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/fmt.h"

#define TFE_DEFAULT_LOGGER_FILENAME "logs/TFExtended.log"
#define TFE_DEFAULT_LOGGER_NAME "TFExtendedLogger"

#ifndef TFE_CONFIG_RELEASE
#define TFE_TRACE(...)		if (spdlog::get(TFE_DEFAULT_LOGGER_NAME) != nullptr) { spdlog::get(TFE_DEFAULT_LOGGER_NAME)->trace(__VA_ARGS__); }
#define TFE_DEBUG(...)		if (spdlog::get(TFE_DEFAULT_LOGGER_NAME) != nullptr) { spdlog::get(TFE_DEFAULT_LOGGER_NAME)->debug(__VA_ARGS__); }
#define TFE_INFO(...)		if (spdlog::get(TFE_DEFAULT_LOGGER_NAME) != nullptr) { spdlog::get(TFE_DEFAULT_LOGGER_NAME)->info(__VA_ARGS__); }
#define TFE_WARN(...)		if (spdlog::get(TFE_DEFAULT_LOGGER_NAME) != nullptr) { spdlog::get(TFE_DEFAULT_LOGGER_NAME)->warn(__VA_ARGS__); }
#define TFE_ERROR(...)		if (spdlog::get(TFE_DEFAULT_LOGGER_NAME) != nullptr) { spdlog::get(TFE_DEFAULT_LOGGER_NAME)->error(__VA_ARGS__); }
#define TFE_CRITICAL(...)	if (spdlog::get(TFE_DEFAULT_LOGGER_NAME) != nullptr) { spdlog::get(TFE_DEFAULT_LOGGER_NAME)->critical(__VA_ARGS__); }
#define TFE_FLUSH			if (spdlog::get(TFE_DEFAULT_LOGGER_NAME) != nullptr) { spdlog::get(TFE_DEFAULT_LOGGER_NAME)->flush(); }
#else
// Disable logging for release builds 
#define TFE_TRACE(...)		(void)0
#define TFE_DEBUG(...)		(void)0
#define TFE_INFO(...)		(void)0
#define TFE_WARN(...)		(void)0
#define TFE_ERROR(...)		(void)0
#define TFE_CRITICAL(...)	(void)0
#endif