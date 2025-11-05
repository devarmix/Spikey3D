#include <Backend/SDL/SDLWindow.h>

namespace Spikey {

	SDLWindow::SDLWindow(const WindowDesc& desc) {
		m_Width = desc.Width;
		m_Height = desc.Height;
		m_Name = desc.Name;

		m_Focused = true;
		m_Minimized = false;
		m_Open = true;

		SDL_Init(SDL_INIT_VIDEO);
		SDL_WindowFlags windowFlags{};

		if (EnumHasAllFlags(desc.Flags, EWindowFlags::Fullscreen)) windowFlags |= SDL_WINDOW_FULLSCREEN;
		else if (EnumHasAllFlags(desc.Flags, EWindowFlags::Borderless)) windowFlags |= SDL_WINDOW_BORDERLESS;

		if (EnumHasAllFlags(desc.Flags, EWindowFlags::Resizable)) windowFlags |= SDL_WINDOW_RESIZABLE;

		// VULKAN, D3D12
		!!!!!!!!!!!!!!!!!!!

		m_Native = SDL_CreateWindow(
			m_Name,
			m_Width,
			m_Height,
			windowFlags
		);
	}

	SDLWindow::~SDLWindow() {
		SDL_DestroyWindow(m_Native);
	}

	void SDLWindow::BindKeyDelegate(KeyDelegate&& delegate) {
		m_OnKey = std::forward<KeyDelegate>(delegate);
	}

	void SDLWindow::BindMouseDeletage(MouseDelegate&& delegate) {
		m_OnMouseMove = std::forward<MouseDelegate>(delegate);
	}

	void SDLWindow::BindScrollDelegate(ScrollDelegate&& delegate) {
		m_OnScroll = std::forward<ScrollDelegate>(delegate);
	}

	void SDLWindow::BindMouseButtonDelegate(MouseButtonDelegate&& delegate) {
		m_OnMouseButton = std::forward<MouseButtonDelegate>(delegate);
	}

	// helper
	EKey SDLKeyToSpikey(SDL_Scancode code) {
		int value = (int)code;

		if (value >= 4 && value <= 29) {
			return EKey(value - 3);
		} // A - Z

		if (value >= 30 && value <= 49) {
			return EKey(value - 3);
		} // 0 - 9

		if (value >= 58 && value <= 69) {
			return EKey(value - 14);
		} // F1 - F12

		switch (code)
		{
		case SDL_SCANCODE_RETURN:
			return EKey::Return;
		case SDL_SCANCODE_ESCAPE:
			return EKey::Esc;
		case SDL_SCANCODE_BACKSPACE:
			return EKey::Backspace;
		case SDL_SCANCODE_TAB:
			return EKey::Tab;
		case SDL_SCANCODE_SPACE:
			return EKey::Space;
		case SDL_SCANCODE_MINUS:
			return EKey::Minus;
		case SDL_SCANCODE_EQUALS:
			return EKey::Equals;
		case SDL_SCANCODE_UP:
			return EKey::Up;
		case SDL_SCANCODE_DOWN:
			return EKey::Down;
		case SDL_SCANCODE_RIGHT:
			return EKey::Right;
		case SDL_SCANCODE_LEFT:
			return EKey::Left;
		case SDL_SCANCODE_LCTRL:
			return EKey::LCtrl;
		case SDL_SCANCODE_RCTRL:
			return EKey::RCtrl;
		case SDL_SCANCODE_LSHIFT:
			return EKey::LShift;
		case SDL_SCANCODE_RSHIFT:
			return EKey::RShift;
		case SDL_SCANCODE_LALT:
			return EKey::LAlt;
		case SDL_SCANCODE_RALT:
			return EKey::RAlt;
		default:
			return EKey::None;
		}
	}

	void SDLWindow::Tick(float32 deltaTime) {
		SDL_Event e{};

		while (SDL_PollEvent(&e) != 0) {

			switch (e.type)
			{
			case SDL_EVENT_QUIT:
				m_Open = false;
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				int w, h;
				SDL_GetWindowSize(m_Native, &w, &h);

				m_Width = w;
				m_Height = h;
				break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:
				m_Focused = false;
				break;
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				m_Focused = true;
				break;
			case SDL_EVENT_WINDOW_MINIMIZED:
				m_Minimized = true;
				break;
			case SDL_EVENT_WINDOW_RESTORED:
				m_Minimized = false;
				break;
			case SDL_EVENT_KEY_DOWN:
				m_OnKey(SDLKeyToSpikey(e.key.scancode), true);
				break;
			case SDL_EVENT_KEY_UP:
				m_OnKey(SDLKeyToSpikey(e.key.scancode), false);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				m_OnMouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				m_OnMouseButton((EMouseButton)e.button.button, true);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				m_OnMouseButton((EMouseButton)e.button.button, false);
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				m_OnScroll(e.wheel.y);
				break;
			default:
				break;
			}
		}
	}
}