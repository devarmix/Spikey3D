#pragma once

#include <SDL3/SDL.h>
#include <Core/Window.h>

namespace Spikey {

	class SDLWindow : public IWindow {
	public:
		SDLWindow(const WindowDesc& desc);
		virtual ~SDLWindow() override;

		virtual bool IsFocused() const override { return m_Focused; }
		virtual bool IsOpen() const override { return m_Open; }
		virtual bool IsMinimized() const override { return m_Minimized; }

		virtual void* GetNativeWindow() override { return (void*)(&m_Native); }
		virtual void Tick(float32 deltaTime) override;

		virtual void BindKeyDelegate(KeyDelegate&& delegate) override;
		virtual void BindMouseDeletage(MouseDelegate&& delegate) override;
		virtual void BindScrollDelegate(ScrollDelegate&& delegate) override;
		virtual void BindMouseButtonDelegate(MouseButtonDelegate&& delegate) override;

	private:
		SDL_Window* m_Native;

		bool m_Focused;
		bool m_Open;
		bool m_Minimized;

		KeyDelegate m_OnKey;
		MouseDelegate m_OnMouseMove;
		ScrollDelegate m_OnScroll;
		MouseButtonDelegate m_OnMouseButton;
	};
}