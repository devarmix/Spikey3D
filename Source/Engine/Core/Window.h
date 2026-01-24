#pragma once

#include <Engine/Core/Common.h>
#include <SDL3/SDL.h>

namespace Spikey
{
	enum class WindowStartPosition : uint8
	{
		CenterScreen,
		Manual
	};

	struct WindowCreateSettings
	{
		std::string Title;
		Vec2Int Position = Vec2Int(100, 400);
		Vec2Int Size = Vec2Int(640, 480);
		Vec2Int MinimumSize = Vec2Int(1, 1);
		Vec2Int MaximumSize = Vec2Int(0, 0);

		WindowStartPosition StartPosition;
		bool Fullscreen = false;
		bool HasBorder = true;
		bool ShowInTaskbar = true;
		bool AllowDragAndDrop = false;
		bool AllowResize = true;
	};

	class Window
	{
	protected:
		WindowCreateSettings m_Settings;
		bool m_IsClosing;
		std::string m_Title;

		SDL_Window* m_Handle;
		void* m_NativeHandle;

	public:
		explicit Window(const WindowCreateSettings& settings);
		~Window();

		void* GetNativeHandle() const
		{
			return m_NativeHandle;
		}

		SDL_Window* GetHandle() const
		{
			return m_Handle;
		}

		bool IsClosing() const
		{
			return m_IsClosing;
		}

		bool IsMain() const;
		bool IsFullscreen() const;
		bool IsMinimized() const;
		bool IsMaximized() const;
		bool IsFocused() const;
		void Minimize();
		void Maximize();
		void SetIsFullscreen(bool isFullscreen);
		void Focus();
		void SetBorderless(bool isBorderless, bool maximized = false);
		void Restore();
		Vec2Int GetSize() const;
		Vec2Int GetPosition() const;
		void SetPosition(const Vec2Int& position);

		std::string GetTitle() const
		{
			return m_Title;
		}

		void SetTitle(const std::string_view& title);
		static void TickPlatform();
	};
}