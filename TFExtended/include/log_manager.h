#pragma once
#include "log.h"
#include "spdlog/sinks/basic_file_sink.h"

class LogManager
{
public:
	LogManager() = default;

	~LogManager() = default;

	void Initialize();

	void Shutdown();
};

