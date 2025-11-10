#pragma once

#include <memory>

#include <Spdlog/spdlog.h>
#include <Spdlog/sinks/stdout_color_sinks.h>

namespace Spikey {

	class Log {
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define ENGINE_ERROR(...)    ::Spikey::Log::GetCoreLogger()->error(__VA_ARGS__);
#define ENGINE_WARN(...)     ::Spikey::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define ENGINE_INFO(...)     ::Spikey::Log::GetCoreLogger()->info(__VA_ARGS__);
#define ENGINE_TRACE(...)    ::Spikey::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define ENGINE_FATAL(...)    ::Spikey::Log::GetCoreLogger()->critical(__VA_ARGS__);


// Client log macros
#define CLIENT_ERROR(...)    ::Spikey::Log::GetClientLogger()->error(__VA_ARGS__);
#define CLIENT_WARN(...)     ::Spikey::Log::GetClientLogger()->warn(__VA_ARGS__);
#define CLIENT_INFO(...)     ::Spikey::Log::GetClientLogger()->info(__VA_ARGS__);
#define CLIENT_TRACE(...)    ::Spikey::Log::GetClientLogger()->trace(__VA_ARGS__);
#define CLIENT_FATAL(...)    ::Spikey::Log::GetClientLogger()->critical(__VA_ARGS__);