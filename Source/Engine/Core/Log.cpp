#include <Engine/Core/Log.h>

namespace Spikey
{
	std::shared_ptr<spdlog::logger> Log::s_Logger;

	void Log::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");

		s_Logger = spdlog::stdout_color_mt("Spikey3D");
		s_Logger->set_level(spdlog::level::trace);
	}
}