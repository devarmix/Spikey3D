#pragma once

#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/Threading/RenderThread.h>
#include <Engine/Core/Window.h>

namespace Spikey {

	struct ApplicationConfig {
		WindowDesc WindowConfig;

		bool EnableGUI;
		bool EnableDocking;
	};

	class IApplication {
	public:
		IApplication(const ApplicationConfig& config);
		virtual ~IApplication() = default;

		void Tick();
		void Destroy();

		IWindow& GetWindow() const { return *m_Window; }
		RenderThread& GetRenderThread() { return m_RenderThread; }
		const ApplicationConfig& GetConfig() const { return m_Config; }

		static IApplication& Get() { CHECK(s_Instance); return *s_Instance; }

	protected:
		virtual void OnTick(float32 deltaTime) = 0;

	private:
		static IApplication* s_Instance;

		IWindow* m_Window;
		RenderThread m_RenderThread;
		ApplicationConfig m_Config;
	};

	extern IApplication* CreateApplication(int argc, char* argv[]);
}