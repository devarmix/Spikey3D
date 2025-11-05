#pragma once

#include <Core/Common.h>

namespace Spikey {

	enum class ERenderAPI : uint8 {
		None = 0,
		Vulkan
	};

	class Graphics {
	public:
		static void Init();
		static void Shutdown();

		static RHIDevice* GetRHI() { return s_RHIDevice; }

	private:
		static RHIDevice* s_RHIDevice;
	};
}