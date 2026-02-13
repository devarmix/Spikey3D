#include <Engine/Core/Engine.h>
#include <Engine/Core/Application.h>
#include <Platform/Vulkan/VulkanBackend.h>

namespace Spikey
{
	uint64     Engine::FrameCount = 0;
	Window*    Engine::MainWindow = nullptr;
	GPUDevice* Engine::GraphicsDevice = nullptr;
	bool       Engine::IsRequestingExit = false;
	int32      Engine::ExitCode = 0;

	void ApplicationBase::OnKeyPressed(Key key, KeyboardKeyMod mod)
	{
	}

	void ApplicationBase::OnKeyReleased(Key key)
	{
	}

	void ApplicationBase::OnMouseButtonPressed(Key button)
	{
	}

	void ApplicationBase::OnMouseButtonReleased(Key button)
	{
	}

	void ApplicationBase::OnMouseMove(float dx, float dy)
	{
	}

	void ApplicationBase::OnMouseScroll(float value)
	{
	}


	int32 Engine::Main(int32 argc, char* argv[])
	{
		WindowCreateSettings windowSettings
		{
			.Title = "Spikey Editor",
			.MinimumSize = Vec2(100, 100)
		};

		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
		Log::Init();

		MainWindow = new Window(windowSettings);
		GraphicsDevice = new Vulkan::VulkanDevice();

		Vec2Int windowSize = MainWindow->GetSize();
		GPUSwapchain* swapchain = GraphicsDevice->CreateSwapchain(windowSize.x, windowSize.y, PixelFormat::BGRA8_UNORM, MainWindow->GetHandle());

		while (!ShouldExit())
		{
			Window::TickPlatform();

			if (!MainWindow->IsMinimized())
			{
				GraphicsDevice->BeginFrame();

				windowSize = MainWindow->GetSize();
				if (swapchain->GetWidth() != windowSize.x
					|| swapchain->GetHeight() != windowSize.y)
				{
					swapchain->Resize(windowSize.x, windowSize.y);
				}

				GPUTextureRef backBuffer = swapchain->GetBackBuffer();

				swapchain->Present(true, true);
				FrameCount++;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
			}
		}

		delete swapchain;
		delete GraphicsDevice;
		delete MainWindow;

		return 0;
	}

	int32 Engine::GetFramesPerSecond()
	{
		return 0;
	}

	void Engine::MessageBoxError(const char* title, const char* msg)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, 
			(MainWindow != nullptr) ? MainWindow->GetHandle() : NULL);
		Exit(-1);
	}

	void Engine::RequestExit(int32 exitCode)
	{
		ExitCode = exitCode;
		IsRequestingExit = true;
	}

	void Engine::Exit(int32 exitCode)
	{
		exit(exitCode);
	}

	bool Engine::ShouldExit()
	{
		return IsRequestingExit;
	}

	bool Engine::IsEditor()
	{
#if WITH_EDITOR
		return true;
#else
		return false;
#endif
	}

	void Engine::OnFixedUpdate()
	{

	}

	void Engine::OnUpdate()
	{

	}

	void Engine::OnDraw()
	{

	}

	void Engine::OnExit()
	{

	}
}