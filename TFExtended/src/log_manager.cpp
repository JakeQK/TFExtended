#include "pch.h"
#include "log_manager.h"

void LogManager::Initialize()
{
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(TFE_DEFAULT_LOGGER_FILENAME);
	fileSink->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l] %v%$");

	std::vector<spdlog::sink_ptr> sinks{ fileSink };
	auto logger = std::make_shared<spdlog::logger>(TFE_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::err);
	spdlog::register_logger(logger);
}

void LogManager::Shutdown()
{
	spdlog::shutdown();
}
