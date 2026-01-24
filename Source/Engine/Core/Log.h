#pragma once

#include <memory>

#include <ThirdParty/Spdlog/spdlog.h>
#include <ThirdParty/Spdlog/sinks/stdout_color_sinks.h>

namespace Spikey
{
	class Log
	{
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

	private:
		static std::shared_ptr<spdlog::logger> s_Logger;
	};
}

#define ENGINE_ERROR(...)    ::Spikey::Log::GetLogger()->error(__VA_ARGS__);
#define ENGINE_WARN(...)     ::Spikey::Log::GetLogger()->warn(__VA_ARGS__);
#define ENGINE_INFO(...)     ::Spikey::Log::GetLogger()->info(__VA_ARGS__);
#define ENGINE_TRACE(...)    ::Spikey::Log::GetLogger()->trace(__VA_ARGS__);
#define ENGINE_FATAL(...)    ::Spikey::Log::GetLogger()->critical(__VA_ARGS__);