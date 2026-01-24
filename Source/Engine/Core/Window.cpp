#include <Engine/Core/Window.h>
#include <Engine/Core/Application.h>
#include <Engine/Core/Input.h>

#include <fstream>
#include <NlohmannJson/json.hpp>
using namespace nlohmann;

namespace Spikey
{
	static std::unordered_map<Key, float> s_InputState = {};
	static Input::DeviceType s_CurrentInputDevice = Input::KeyboardAndMouse;
	static SDL_Gamepad* s_CurrentGamepad = nullptr;
	static SDL_JoystickID s_CurrentGamepadID = 0;

	static std::unordered_map<std::string, Key> s_NameToKey =
	{
		{ "Tab", Key::Tab },
		{ "LeftArrow", Key::LeftArrow },
		{ "RightArrow", Key::RightArrow },
		{ "UpArrow", Key::UpArrow },
		{ "DownArrow", Key::DownArrow },
		{ "PageUp", Key::PageUp },
		{ "PageDown", Key::PageDown },
		{ "Home", Key::Home },
		{ "End", Key::End },
		{ "Insert", Key::Insert },
		{ "Delete", Key::Delete },
		{ "Backspace", Key::Backspace },
		{ "Space", Key::Space },
		{ "Enter", Key::Enter },
		{ "Escape", Key::Escape },
		{ "LeftCtrl", Key::LeftCtrl },
		{ "LeftShift ", Key::LeftShift  },
		{ "LeftAlt", Key::LeftAlt },
		{ "RightCtrl", Key::RightCtrl },
		{ "RightShift", Key::RightShift },
		{ "RightAlt", Key::RightAlt },
		{ "Menu", Key::Menu },
		{ "Zero", Key::Zero },
		{ "One", Key::One },
		{ "Two", Key::Two },
		{ "Three", Key::Three },
		{ "Four", Key::Four },
		{ "Five", Key::Five },
		{ "Six", Key::Six },
		{ "Seven", Key::Seven },
		{ "Eight", Key::Eight },
		{ "Nine", Key::Nine },
		{ "A", Key::A },
		{ "B", Key::B },
		{ "C", Key::C },
		{ "D ", Key::D  },
		{ "E", Key::E },
		{ "F", Key::F },
		{ "G", Key::G },
		{ "H", Key::H  },
		{ "I", Key::I },
		{ "J", Key::J },
		{ "K", Key::K },
		{ "L", Key::L },
		{ "M", Key::M },
		{ "N", Key::N },
		{ "O", Key::O },
		{ "P", Key::P },
		{ "Q", Key::Q },
		{ "R", Key::R  },
		{ "S", Key::S },
		{ "T", Key::T },
		{ "U", Key::U },
		{ "V", Key::V },
		{ "W", Key::W },
		{ "X", Key::X },
		{ "Y", Key::Y },
		{ "Z", Key::Z },
		{ "F1", Key::F1 },
		{ "F2", Key::F2 },
		{ "F3", Key::F3 },
		{ "F4", Key::F4 },
		{ "F5", Key::F5 },
		{ "F6", Key::F6 },
		{ "F7", Key::F7 },
		{ "F8", Key::F8 },
		{ "F9", Key::F9 },
		{ "F10", Key::F10 },
		{ "F11 ", Key::F11  },
		{ "F12", Key::F12 },
		{ "Apostrophe", Key::Apostrophe },
		{ "Comma", Key::Comma },
		{ "Minus", Key::Minus },
		{ "Period", Key::Period },
		{ "Slash", Key::Slash },
		{ "Semicolon", Key::Semicolon },
		{ "Equal", Key::Equal },
		{ "LeftBracket", Key::LeftBracket },
		{ "Backslash", Key::Backslash },
		{ "RightBracket", Key::RightBracket },
		{ "GraveAccent", Key::GraveAccent },
		{ "CapsLock", Key::CapsLock },
		{ "ScrollLock", Key::ScrollLock },
		{ "NumLock", Key::NumLock },
		{ "PrintScreen", Key::PrintScreen },
		{ "Pause", Key::Pause },
		{ "Keypad_Zero", Key::Keypad_Zero },
		{ "Keypad_One", Key::Keypad_One },
		{ "Keypad_Two", Key::Keypad_Two },
		{ "Keypad_Three", Key::Keypad_Three },
		{ "Keypad_Four", Key::Keypad_Four },
		{ "Keypad_Five", Key::Keypad_Five },
		{ "Keypad_Six", Key::Keypad_Six },
		{ "Keypad_Seven", Key::Keypad_Seven },
		{ "Keypad_Eight", Key::Keypad_Eight },
		{ "Keypad_Nine", Key::Keypad_Nine },
		{ "Keypad_Decimal", Key::Keypad_Decimal },
		{ "Keypad_Divide", Key::Keypad_Divide },
		{ "Keypad_Multiply", Key::Keypad_Multiply },
		{ "Keypad_Subtract", Key::Keypad_Subtract },
		{ "Keypad_Add", Key::Keypad_Add },
		{ "Keypad_Enter", Key::Keypad_Enter },
		{ "Keypad_Equal", Key::Keypad_Equal },
		{ "Gamepad_Start", Key::Gamepad_Start },
		{ "Gamepad_Back", Key::Gamepad_Back },
		{ "Gamepad_LeftX", Key::Gamepad_LeftX },
		{ "Gamepad_LeftY", Key::Gamepad_LeftY },
		{ "Gamepad_RightX", Key::Gamepad_RightX },
		{ "Gamepad_RightY", Key::Gamepad_RightY },
		{ "Gamepad_L1", Key::Gamepad_L1 },
		{ "Gamepad_R1", Key::Gamepad_R1 },
		{ "Gamepad_R2", Key::Gamepad_R2 },
		{ "Gamepad_L2", Key::Gamepad_L2 },
		{ "Gamepad_R3", Key::Gamepad_R3 },
		{ "Gamepad_L3", Key::Gamepad_L3 },
		{ "Gamepad_FaceBottom", Key::Gamepad_FaceBottom },
		{ "Gamepad_FaceRight", Key::Gamepad_FaceRight },
		{ "Gamepad_FaceLeft", Key::Gamepad_FaceLeft },
		{ "Gamepad_FaceTop", Key::Gamepad_FaceTop },
		{ "Gamepad_DPadRight", Key::Gamepad_DPadRight },
		{ "Gamepad_DPadUp", Key::Gamepad_DPadUp },
		{ "Gamepad_DPadLeft", Key::Gamepad_DPadLeft },
		{ "Gamepad_DPadDown", Key::Gamepad_DPadDown },
		{ "Mouse_X", Key::Mouse_X },
		{ "Mouse_Y", Key::Mouse_Y },
		{ "Mouse_LeftButton", Key::Mouse_LeftButton },
		{ "Mouse_RightButton", Key::Mouse_RightButton },
		{ "Mouse_MiddleButton", Key::Mouse_MiddleButton },
		{ "Mouse_X1Button", Key::Mouse_X1Button },
		{ "Mouse_X2Button", Key::Mouse_X2Button },
		{ "Mouse_Scroll", Key::Mouse_Scroll }
	};

	void Input::Init()
	{
		for (auto& [name, key] : s_NameToKey)
		{
			s_InputState[key] = {};
		}

		if (SDL_HasGamepad())
		{
			int32 count;
			SDL_JoystickID* ids = SDL_GetGamepads(&count);

			for (int32 i = 0; i < count; i++)
			{
				s_CurrentGamepad = SDL_OpenGamepad(ids[i]);
				if (s_CurrentGamepad != nullptr)
				{
					s_CurrentGamepadID = ids[0];
					break;
				}
			}
		}
	}

	Input::DeviceType Input::GetActiveDevice()
	{
		return s_CurrentInputDevice;
	}

	Key Input::GetKeyFromName(const std::string& name)
	{
		auto it = s_NameToKey.find(name);
		if (it != s_NameToKey.end())
		{
			return it->second;
		}

		return Key::Invalid;
	}

	static Key ToSpikeyKeyboardButton(SDL_Scancode scan, SDL_Keycode sym)
	{
		// A - Z
		if (sym >= SDLK_A && sym <= SDLK_Z)
		{
			return (Key)(sym - SDLK_A + (uint32)Key::A);
		}

		// 0 - 9
		if (sym >= SDLK_0 && sym <= SDLK_9)
		{
			return (Key)(sym - SDLK_0 + (uint32)Key::Zero);
		}

		// keypad 0 - 9
		if (sym >= SDLK_KP_0 && sym <= SDLK_KP_9)
		{
			return (Key)(sym - SDLK_KP_0 + (uint32)Key::Keypad_Zero);
		}

		// F1 - F12
		if (scan >= SDL_SCANCODE_F1 && scan <= SDL_SCANCODE_F12)
		{
			return (Key)(scan - SDL_SCANCODE_F1 + (uint32)Key::F1);
		}

		switch (scan)
		{
		case SDL_SCANCODE_RETURN: return Key::Enter;
		case SDL_SCANCODE_ESCAPE: return Key::Escape;
		case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
		case SDL_SCANCODE_TAB: return Key::Tab;
		case SDL_SCANCODE_SPACE: return Key::Space;
		case SDL_SCANCODE_MINUS: return Key::Minus;
		case SDL_SCANCODE_EQUALS: return Key::Equal;
		case SDL_SCANCODE_LEFTBRACKET: return Key::LeftBracket;
		case SDL_SCANCODE_RIGHTBRACKET: return Key::RightBracket;
		case SDL_SCANCODE_BACKSLASH: return Key::Backslash;
		case SDL_SCANCODE_SEMICOLON: return Key::Semicolon;
		case SDL_SCANCODE_APOSTROPHE: return Key::Apostrophe;
		case SDL_SCANCODE_GRAVE: return Key::GraveAccent;
		case SDL_SCANCODE_COMMA: return Key::Comma;
		case SDL_SCANCODE_PERIOD: return Key::Period;
		case SDL_SCANCODE_SLASH: return Key::Slash;
		case SDL_SCANCODE_CAPSLOCK: return Key::CapsLock;
		case SDL_SCANCODE_PRINTSCREEN: return Key::PrintScreen;
		case SDL_SCANCODE_SCROLLLOCK: return Key::ScrollLock;
		case SDL_SCANCODE_PAUSE: return Key::Pause;
		case SDL_SCANCODE_INSERT: return Key::Insert;
		case SDL_SCANCODE_HOME: return Key::Home;
		case SDL_SCANCODE_PAGEUP: return Key::PageUp;
		case SDL_SCANCODE_DELETE: return Key::Delete;
		case SDL_SCANCODE_END: return Key::End;
		case SDL_SCANCODE_PAGEDOWN: return Key::PageDown;
		case SDL_SCANCODE_RIGHT: return Key::RightArrow;
		case SDL_SCANCODE_LEFT: return Key::LeftArrow;
		case SDL_SCANCODE_DOWN: return Key::DownArrow;
		case SDL_SCANCODE_UP: return Key::UpArrow;
		case SDL_SCANCODE_NUMLOCKCLEAR: return Key::NumLock;
		case SDL_SCANCODE_KP_DIVIDE: return Key::Keypad_Divide;
		case SDL_SCANCODE_KP_MULTIPLY: return Key::Keypad_Multiply;
		case SDL_SCANCODE_KP_MINUS: return Key::Keypad_Subtract;
		case SDL_SCANCODE_KP_PLUS: return Key::Keypad_Add;
		case SDL_SCANCODE_KP_ENTER: return Key::Keypad_Enter;
		case SDL_SCANCODE_KP_EQUALS: return Key::Keypad_Equal;
		case SDL_SCANCODE_MENU: return Key::Menu;
		case SDL_SCANCODE_LCTRL: return Key::LeftCtrl;
		case SDL_SCANCODE_LSHIFT: return Key::LeftShift;
		case SDL_SCANCODE_LALT: return Key::LeftAlt;
		case SDL_SCANCODE_RCTRL: return Key::RightCtrl;
		case SDL_SCANCODE_RSHIFT: return Key::RightShift;
		case SDL_SCANCODE_RALT: return Key::RightAlt;

		default:
			return Key::Invalid;
		}
	}

	static Key ToSpikeyGamepadButton(uint8 button)
	{
		switch (button)
		{
		case SDL_GAMEPAD_BUTTON_SOUTH: return Key::Gamepad_FaceBottom;
		case SDL_GAMEPAD_BUTTON_NORTH: return Key::Gamepad_FaceTop;
		case SDL_GAMEPAD_BUTTON_WEST: return Key::Gamepad_FaceRight;
		case SDL_GAMEPAD_BUTTON_EAST: return Key::Gamepad_FaceLeft;
		case SDL_GAMEPAD_BUTTON_BACK: return Key::Gamepad_Back;
		case SDL_GAMEPAD_BUTTON_START: return Key::Gamepad_Start;
		case SDL_GAMEPAD_BUTTON_LEFT_STICK: return Key::Gamepad_L3;
		case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return Key::Gamepad_R3;
		case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return Key::Gamepad_L1;
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return Key::Gamepad_R1;
		case SDL_GAMEPAD_BUTTON_DPAD_UP: return Key::Gamepad_DPadUp;
		case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return Key::Gamepad_DPadDown;
		case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return Key::Gamepad_DPadLeft;
		case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return Key::Gamepad_DPadRight;

		default:
			return Key::Invalid;
		}
	}

	static Key ToSpikeyMouseButton(uint8 button)
	{
		switch (button)
		{
		case SDL_BUTTON_LEFT: return Key::Mouse_LeftButton;
		case SDL_BUTTON_MIDDLE: return Key::Mouse_MiddleButton;
		case SDL_BUTTON_RIGHT: return Key::Mouse_RightButton;
		case SDL_BUTTON_X1: return Key::Mouse_X1Button;
		case SDL_BUTTON_X2: return Key::Mouse_X2Button;

		default:
			return Key::Invalid;
		}
	}

	static void from_json(const json& j, InputTrigger& trigger)
	{
		std::string buttonName = {};
		j.at("button").get_to(buttonName);

		trigger.TriggerKey = Input::GetKeyFromName(buttonName);
		if (trigger.TriggerKey == Key::Invalid)
			throw std::exception("Invalid button name!");

		auto it = j.find("event");
		if (it != j.end())
		{
			std::string event = {};
			it->get_to(event);

			if (event == "Pressed")
				trigger.Event = InputTrigger::Pressed;
			else if (event == "Released")
				trigger.Event = InputTrigger::Released;
			else if (event == "Down")
				trigger.Event = InputTrigger::Down;
			else
				throw std::exception("Invalid event args, expected Pressed, Released or Down");
		}

		it = j.find("deadzoneMin");
		if (it != j.end())
		{
			it->get_to(trigger.DeadzoneMin);
		}

		it = j.find("deadzoneMax");
		if (it != j.end())
		{
			it->get_to(trigger.DeadzoneMax);
		}

		it = j.find("valueScale");
		if (it != j.end())
		{
			it->get_to(trigger.ValueScale);
		}
	}

	static void from_json(const json& j, InputAction& action)
	{
		std::string type = {};

		j.at("name").get_to(action.Name);
		j.at("type").get_to(type);

		if (type == "Boolean")
			action.ActionType = InputAction::Boolean;
		else if (type == "Axis")
			action.ActionType = InputAction::Axis;
		else
			throw std::exception("Invalid type args, expected Boolean or Axis");

		auto it = j.find("minValue");
		if (it != j.end())
		{
			it->get_to(action.ClampMin);
		}

		it = j.find("maxValue");
		if (it != j.end())
		{
			it->get_to(action.ClampMax);
		}

		auto& triggerList = j.at("triggers");
		for (auto& trigger : triggerList)
		{
			InputTrigger& t = action.Triggers.emplace_back();
			t = trigger.get<InputTrigger>();
		}
	}

	bool InputActionContext::FromJson(const std::filesystem::path& path)
	{
		std::ifstream file(path);
		if (!file.is_open())
		{
			ENGINE_ERROR("Json action context: {} cannot be opened!", path.string());
			return false;
		}

		json jsonData = json::parse(file);

		try
		{
			auto& actionList = jsonData.at("actions");
			for (auto& action : actionList)
			{
				InputAction a = {};
				action.get_to(a);

				m_ActionMap.emplace(a.Name, a);
			}
		}
		catch (const std::exception& error)
		{
			ENGINE_ERROR("Invalid json action context: {0}, error: {1}", path.string(), error.what());
			m_ActionMap.clear();

			return false;
		}

		return true;
	}

	void InputActionContext::Tick()
	{
		for (auto& [name, action] : m_ActionMap)
		{
			action.Value = 0;
			for (auto& trigger : action.Triggers)
			{
				float triggerValue = 0;

				switch (trigger.Event)
				{
				case InputTrigger::Pressed:
					triggerValue = (!trigger.LastValue && s_InputState[trigger.TriggerKey]) ? 1.f : 0.f;
					break;
				case InputTrigger::Released:
					triggerValue = (trigger.LastValue && !s_InputState[trigger.TriggerKey]) ? 1.f : 0.f;
					break;
				case InputTrigger::Down:
					triggerValue = (s_InputState[trigger.TriggerKey] > 0) ? 1.f : 0.f;
					break;
				default:
					triggerValue = s_InputState[trigger.TriggerKey];
					break;
				}

				if (triggerValue < trigger.DeadzoneMin)
				{
					triggerValue = 0;
				}
				else if (triggerValue > trigger.DeadzoneMax)
				{
					triggerValue = 1;
				}

				triggerValue *= trigger.ValueScale;

				action.Value += triggerValue;
				trigger.LastValue = s_InputState[trigger.TriggerKey];
			}

			action.Value = std::max(action.Value, action.ClampMin);
			action.Value = std::min(action.Value, action.ClampMax);
		}
	}


	Window::Window(const WindowCreateSettings& settings)
		: m_Settings(settings)
		, m_Title(settings.Title)
		, m_NativeHandle(nullptr)
		, m_IsClosing(false)
	{
		SDL_WindowFlags windowFlags = {};
		windowFlags |= SDL_INIT_GAMEPAD;

		if (settings.Fullscreen)
			windowFlags |= SDL_WINDOW_FULLSCREEN;
		if (settings.AllowResize)
			windowFlags |= SDL_WINDOW_RESIZABLE;
		if (!settings.ShowInTaskbar)
			windowFlags |= SDL_WINDOW_UTILITY;
		if (!settings.HasBorder)
			windowFlags |= SDL_WINDOW_BORDERLESS;

#if SPIKEY_PLATFORM_WIN32
		windowFlags |= SDL_WINDOW_VULKAN;
#else
#error Platform unsupported!
#endif

		m_Handle = SDL_CreateWindow(
			m_Title.size() > 0 ? m_Title.c_str() : "Spikey3D",
			settings.Size.x,
			settings.Size.y,
			windowFlags
		);

		if (!m_Handle)
		{
			Engine::MessageBoxError("Fatal Error", "Failed to create window!");
		}

		SDL_SetWindowMinimumSize(m_Handle, settings.MinimumSize.x, settings.MinimumSize.y);
		if (settings.MaximumSize.x > 0 && settings.MaximumSize.y > 0)
		{
			SDL_SetWindowMaximumSize(m_Handle, settings.MaximumSize.x, settings.MaximumSize.y);
		}

		if (settings.StartPosition == WindowStartPosition::CenterScreen)
		{
			const SDL_DisplayMode* display = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());

			const int32 xPos = (display->w - settings.Size.x) / 2;
			const int32 yPos = (display->h - settings.Size.y) / 2;

			SDL_SetWindowPosition(m_Handle, xPos, yPos);
		}
		else
		{
			SDL_SetWindowPosition(m_Handle, settings.Position.x, settings.Position.y);
		}

#ifdef SPIKEY_PLATFORM_WINDOWS
		SDL_PropertiesID properties = SDL_GetWindowProperties(m_Handle);
		m_NativeHandle = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

		if (!m_NativeHandle)
		{
			assert(false && "Failed to obtain native window handle!");
		}
#endif
	}

	Window::~Window()
	{
		SDL_DestroyWindow(m_Handle);
	}

	void Window::TickPlatform()
	{
		s_InputState[Key::Mouse_X] = 0.f;
		s_InputState[Key::Mouse_Y] = 0.f;
		s_InputState[Key::Mouse_Scroll] = 0.f;

		SDL_Event event = {};
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
			{
				Engine::RequestExit();
				break;
			}
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				SDL_WindowID mainWindowID
					= SDL_GetWindowID(Engine::MainWindow->GetHandle());

				if (event.window.windowID == mainWindowID)
				{
					Engine::RequestExit();
				}
				break;
			}
			case SDL_EVENT_GAMEPAD_ADDED:
			{
				if (s_CurrentGamepad == nullptr)
				{
					s_CurrentGamepad = SDL_OpenGamepad(event.gdevice.which);
					s_CurrentGamepadID = event.gdevice.which;
				}
				break;
			}
			case SDL_EVENT_GAMEPAD_REMOVED:
			{
				if (s_CurrentGamepadID == event.gdevice.which)
				{
					s_CurrentGamepad = nullptr;
					s_CurrentGamepadID = 0;
				}
				break;
			}
			case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			{
				Key button
					= ToSpikeyGamepadButton(event.gbutton.button);

				if (button != Key::Invalid)
				{
					s_InputState[button] = 1;
				}

				SDL_GamepadType gamepadType = SDL_GetGamepadType(s_CurrentGamepad);
				switch (gamepadType)
				{
				case SDL_GAMEPAD_TYPE_PS3:
				case SDL_GAMEPAD_TYPE_PS4:
				case SDL_GAMEPAD_TYPE_PS5:
					s_CurrentInputDevice = Input::PsController;
					break;
				default:
					s_CurrentInputDevice = Input::XboxController;
					break;
				}
				break;
			}
			case SDL_EVENT_GAMEPAD_BUTTON_UP:
			{
				Key button
					= ToSpikeyGamepadButton(event.gbutton.button);

				if (button != Key::Invalid)
				{
					s_InputState[button] = 0;
				}
				break;
			}
			case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			{
				float value = event.gaxis.value / 32767.0f;

				switch (event.gaxis.axis)
				{
				case SDL_GAMEPAD_AXIS_LEFTX:
					s_InputState[Key::Gamepad_LeftX] = value;
					break;
				case SDL_GAMEPAD_AXIS_LEFTY:
					s_InputState[Key::Gamepad_LeftY] = value;
					break;
				case SDL_GAMEPAD_AXIS_RIGHTX:
					s_InputState[Key::Gamepad_RightX] = value;
					break;
				case SDL_GAMEPAD_AXIS_RIGHTY:
					s_InputState[Key::Gamepad_RightY] = value;
					break;
				case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
					s_InputState[Key::Gamepad_L2] = value;
					break;
				case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER:
					s_InputState[Key::Gamepad_R2] = value;
					break;
				default:
					break;
				}
			}
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				Key button
					= ToSpikeyMouseButton(event.button.button);

				if (button != Key::Invalid)
				{
					s_InputState[button] = 1;
					Application::OnMouseButtonPressed(button);
				}

				s_CurrentInputDevice = Input::KeyboardAndMouse;
				break;
			}
			case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				Key button
					= ToSpikeyMouseButton(event.button.button);

				if (button != Key::Invalid)
				{
					s_InputState[button] = 0;
					Application::OnMouseButtonReleased(button);
				}
				break;
			}
			case SDL_EVENT_KEY_DOWN:
			{
				KeyboardKeyMod mod = {};

				if (event.key.mod & SDL_KMOD_SHIFT)
					mod |= KeyboardKeyMod::Shift;
				if (event.key.mod & SDL_KMOD_ALT)
					mod |= KeyboardKeyMod::Alt;
				if (event.key.mod & SDL_KMOD_CTRL)
					mod |= KeyboardKeyMod::Ctrl;

				Key key 
					= ToSpikeyKeyboardButton(event.key.scancode, event.key.key);

				if (key != Key::Invalid)
				{
					s_InputState[key] = 1;
					Application::OnKeyPressed(key, mod);
				}

				s_CurrentInputDevice = Input::KeyboardAndMouse;
				break;
			}
			case SDL_EVENT_KEY_UP:
			{
				Key key
					= ToSpikeyKeyboardButton(event.key.scancode, event.key.key);

				if (key != Key::Invalid)
				{
					s_InputState[key] = 0;
					Application::OnKeyReleased(key);
				}

				break;
			}
			case SDL_EVENT_MOUSE_MOTION:
			{
				s_InputState[Key::Mouse_X] = event.motion.xrel;
				s_InputState[Key::Mouse_Y] = event.motion.yrel;

				Application::OnMouseMove(event.motion.xrel, event.motion.yrel);
				break;
			}
			case SDL_EVENT_MOUSE_WHEEL:
			{
				s_InputState[Key::Mouse_Scroll] = event.wheel.x;
				Application::OnMouseScroll(event.wheel.x);
				break;
			}
			default:
				break;
			};
		}
	}

	void Window::SetIsFullscreen(bool isFullscreen)
	{
		SDL_SetWindowFullscreen(m_Handle, isFullscreen);
	}

	bool Window::IsMain() const
	{
		return Engine::MainWindow == this;
	}

	bool Window::IsFullscreen() const
	{
		return SDL_GetWindowFlags(m_Handle) & SDL_WINDOW_FULLSCREEN;
	}

	bool Window::IsMinimized() const
	{
		return SDL_GetWindowFlags(m_Handle) & SDL_WINDOW_MINIMIZED;
	}

	bool Window::IsMaximized() const
	{
		return SDL_GetWindowFlags(m_Handle) & SDL_WINDOW_MAXIMIZED;
	}

	bool Window::IsFocused() const
	{
		return SDL_GetWindowFlags(m_Handle) & SDL_WINDOW_INPUT_FOCUS;
	}

	void Window::Restore()
	{
		SDL_RestoreWindow(m_Handle);
	}

	void Window::Focus()
	{
		if (IsFocused())
		{
			SDL_RaiseWindow(m_Handle);
		}
	}

	void Window::SetBorderless(bool isBorderless, bool maximized)
	{
		if (IsFullscreen())
			SetIsFullscreen(false);

		if (IsMaximized())
			Restore();

		SDL_SetWindowBordered(m_Handle, isBorderless);

		if (maximized)
		{
			Maximize();
		}
	}

	void Window::Minimize()
	{
		if (!m_Settings.AllowResize)
			return;

		if (!IsMinimized())
		{
			SDL_MinimizeWindow(m_Handle);
		}
	}

	void Window::Maximize()
	{
		if (!m_Settings.AllowResize)
			return;

		if (!IsMaximized())
		{
			SDL_MaximizeWindow(m_Handle);
		}
	}

	Vec2Int Window::GetSize() const
	{
		int32 w, h;
		SDL_GetWindowSize(m_Handle, &w, &h);

		return Vec2Int(w, h);
	}

	Vec2Int Window::GetPosition() const
	{
		int32 x, y;
		SDL_GetWindowPosition(m_Handle, &x, &y);

		return Vec2Int(x, y);
	}

	void Window::SetPosition(const Vec2Int& position)
	{
		SDL_SetWindowPosition(m_Handle, position.x, position.y);
	}

	void Window::SetTitle(const std::string_view& title)
	{
		m_Title = title;
		SDL_SetWindowTitle(m_Handle, m_Title.size() > 0 ? m_Title.c_str() : "Spikey3D");
	}
}