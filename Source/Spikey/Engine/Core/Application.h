#pragma once

#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/Threading/RenderThread.h>
#include <Engine/Core/Window.h>

namespace Spikey {

	struct ApplicationConfig 
	{
		WindowDesc WindowConfig;

		bool EnableGUI;
		bool EnableDocking;
	};

	class ApplicationBase
	{
	public:

		virtual void OnUpdate() = 0;

	private:
	};

	class Engine
	{
	public:
		static uint64 FrameCounter;
		static std::string Path1;
		static std::string Path2;
		static std::string Path3;

		static IWindow* Window;

		static int32 Main(int32 argc, char* argv[])
		{

		}

		void Exit();
		void RequestExit();
		void Crash();

		bool IsEditor();
		bool IsPlayMode();


		void OnUpdate();
		void OnPhysics();
	};

	extern ApplicationBase* CreateApplication(int argc, char* argv[])
	{
		Engine::FrameCounter;
	}
}