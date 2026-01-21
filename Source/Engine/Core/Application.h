#pragma once

#include <Graphics/GPUDevice.h>
#include <Core/Input.h>
#include <Core/Window.h>

namespace Spikey
{
	class ApplicationBase
	{
	public:
		static Window* CreateMainWindow() = delete;

		static bool OnInit() = delete;
		static void OnUpdate(float deltaTime) = delete;
		static void OnFixedUpdate() = delete;
		static void OnDraw() = delete;
		static void OnExit() = delete;

		static void OnKeyPressed(Key key, KeyboardKeyMod mod);
		static void OnKeyReleased(Key key);
		static void OnMouseButtonPressed(Key button);
		static void OnMouseButtonReleased(Key button);
		static void OnMouseMove(float dx, float dy);
	};

	class Engine
	{
	public:
		static uint64 FrameCount;
		static Window* MainWindow;
		static GPUDevice* GraphicsDevice;
		static bool IsRequestingExit;
		static int32 ExitCode;

	public:
		static int32 Main(int32 argc, char* argv[]);
		static int32 GetFramesPerSecond();

		static void RequestExit(int32 exitCode = 0);
		static void Exit(int32 exitCode = -1);
		static bool ShouldExit();
		static bool IsEditor();
		static void OnFixedUpdate();
		static void OnUpdate();
		static void OnDraw();
		static void OnExit();
	};
}