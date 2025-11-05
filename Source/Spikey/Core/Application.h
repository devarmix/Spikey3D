#pragma once

#include <Graphics/GraphicsCore.h>
#include <Core/Window.h>

namespace Spikey {

	struct ApplicationDesc {

		bool UsingImGui;
		bool UsingDocking;

		ERenderAPI RenderAPI;
		WindowDesc WindowDesc;
	};

	class IApplication {
	public:
		IApplication();
		virtual ~IApplication() = default;

		void Tick();

		IWindow& GetWindow() const { return *m_Window; }
		static IApplication& Get() { CHECK(s_Instance); return *s_Instance; }

	private:
		static IApplication* s_Instance;

		IWindow* m_Window;
	};
}