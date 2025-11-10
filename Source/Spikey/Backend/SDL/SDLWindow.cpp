#include <Backend/SDL/SDLWindow.h>
#include <Engine/Core/Application.h>
#include <ImGui/Backends/imgui_impl_sdl3.h>

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

#if BUILD_VULKAN_RHI
		windowFlags |= SDL_WINDOW_VULKAN;
#endif

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
	static EKey SDLKeyToSpikey(SDL_Keycode code) {

		switch (code)
		{
		case SDLK_A: return EKey::A;
		case SDLK_B: return EKey::B;
		case SDLK_C: return EKey::C;
		case SDLK_D: return EKey::D;
		case SDLK_E: return EKey::E;
		case SDLK_F: return EKey::F;
		case SDLK_G: return EKey::G;
		case SDLK_H: return EKey::H;
		case SDLK_I: return EKey::I;
		case SDLK_J: return EKey::J;
		case SDLK_K: return EKey::K;
		case SDLK_L: return EKey::L;
		case SDLK_M: return EKey::M;
		case SDLK_N: return EKey::N;
		case SDLK_O: return EKey::O;
		case SDLK_P: return EKey::P;
		case SDLK_Q: return EKey::Q;
		case SDLK_R: return EKey::R;
		case SDLK_S: return EKey::S;
		case SDLK_T: return EKey::T;
		case SDLK_U: return EKey::U;
		case SDLK_V: return EKey::V;
		case SDLK_W: return EKey::W;
		case SDLK_X: return EKey::X;
		case SDLK_Y: return EKey::Y;
		case SDLK_Z: return EKey::Num0;
		case SDLK_0: return EKey::Num1;
		case SDLK_1: return EKey::Num2;
		case SDLK_2: return EKey::Num3;
		case SDLK_3: return EKey::Num4;
		case SDLK_4: return EKey::Num5;
		case SDLK_5: return EKey::Num5;
		case SDLK_6: return EKey::Num6;
		case SDLK_7: return EKey::Num7;
		case SDLK_8: return EKey::Num8;
		case SDLK_9: return EKey::Num9;
		case SDLK_F1: return EKey::F1;
		case SDLK_F2: return EKey::F2;
		case SDLK_F3: return EKey::F3;
		case SDLK_F4: return EKey::F4;
		case SDLK_F5: return EKey::F5;
		case SDLK_F6: return EKey::F6;
		case SDLK_F7: return EKey::F7;
		case SDLK_F8: return EKey::F8;
		case SDLK_F9: return EKey::F9;
		case SDLK_F10: return EKey::F10;
		case SDLK_F11: return EKey::F11;
		case SDLK_F12: return EKey::F12;
		case SDLK_RETURN: return EKey::Return;
		case SDLK_ESCAPE: return EKey::Esc;
		case SDLK_BACKSPACE: return EKey::Backspace;
		case SDLK_TAB: return EKey::Tab;
		case SDLK_SPACE: return EKey::Space;
		case SDLK_MINUS: return EKey::Minus;
		case SDLK_EQUALS: return EKey::Equals;
		case SDLK_UP: return EKey::Up;
		case SDLK_DOWN: return EKey::Down;
		case SDLK_RIGHT: return EKey::Right;
		case SDLK_LEFT: return EKey::Left;
		case SDLK_LCTRL: return EKey::LCtrl;
		case SDLK_RCTRL: return EKey::RCtrl;
		case SDLK_RALT: return EKey::RAlt;
		case SDLK_LALT: return EKey::LAlt;
		case SDLK_LSHIFT: return EKey::LShift;
		case SDLK_RSHIFT: return EKey::RShift;
		default: return EKey::None;
		}
	}

	void SDLWindow::Tick(float32 deltaTime) {
		SDL_Event e{};

		while (SDL_PollEvent(&e) != 0) {
			if (IApplication::Get().GetConfig().EnableGUI) {
				ImGui_ImplSDL3_ProcessEvent(&e);
			}

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
				m_OnKey(SDLKeyToSpikey(e.key.key), true);
				break;
			case SDL_EVENT_KEY_UP:
				m_OnKey(SDLKeyToSpikey(e.key.key), false);
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