#include <Core/Application.h>

#if BUILD_PLATFORM_WIN32
#include <Backend/SDL/SDLWindow.h>
#endif

namespace Spikey {

	IWindow* IWindow::Create(const WindowDesc& desc) {
#if BUILD_PLATFORM_WIN32
		return new SDLWindow(desc);
#else
#error Spikey3D does not support current platform!
#endif
	}

	Input::InputData Input::s_Data = {};

	void Input::Init() {
		IWindow& window = IApplication::Get().GetWindow();

		window.BindKeyDelegate([](EKey key, bool pressed) {
			if (key != EKey::None) {
				if (pressed) {
					s_Data.States[(uint8)key] = { true, true, false };
					s_Data.Pressed.push_back(key);
				}
				else {
					s_Data.States[(uint8)key] = { false, false, true };
					s_Data.Released.push_back(key);
				}
			}
			});

		window.BindMouseButtonDelegate([](EMouseButton button, bool pressed) {
			if (pressed) {
				s_Data.Buttons[(uint8)button] = { true, true, false };
			}
			else {
				s_Data.Buttons[(uint8)button] = { false, false, true };
			}
			});

		window.BindMouseDeletage([](float32 x, float32 y, float32 dx, float32 dy) {
			s_Data.MouseX = x;
			s_Data.MouseY = y;
			s_Data.DeltaMouseX = dx;
			s_Data.DeltaMouseY = dy;
			});

		window.BindScrollDelegate([](float32 value) {
			s_Data.MouseScroll = value;
			});
	}

	void Input::Tick() {

		for (auto key : s_Data.Pressed) {
			s_Data.States[(uint8)key].Pressed = false;
		}

		for (auto key : s_Data.Released) {
			s_Data.States[(uint8)key].Released = false;
		}

		for (int i = 1; i < 6; i++) {
			s_Data.Buttons[i].Pressed = false;
			s_Data.Buttons[i].Released = false;
		}

		s_Data.DeltaMouseX = 0;
		s_Data.DeltaMouseY = 0;
		s_Data.MouseScroll = 0;

		s_Data.Released.clear();
		s_Data.Pressed.clear();
	}

	bool Input::GetKeyDown(EKey key) {
		return s_Data.States[(uint8)key].Down;
	}

	bool Input::GetKeyUp(EKey key) {
		return !GetKeyUp(key);
	}

	bool Input::GetKeyPressed(EKey key) {
		return s_Data.States[(uint8)key].Pressed;
    }

	bool Input::GetKeyReleased(EKey key) {
		return s_Data.States[(uint8)key].Released;
	}

	bool Input::GetMouseButtonDown(EMouseButton button) {
		return s_Data.Buttons[(uint8)button].Down;
	}

	bool Input::GetMouseButtonUp(EMouseButton button) {
		return !GetMouseButtonDown(button);
	}

	bool Input::GetMouseButtonPressed(EMouseButton button) {
		return s_Data.Buttons[(uint8)button].Pressed;
	}

	bool Input::GetMouseButtonReleased(EMouseButton button) {
		return s_Data.Buttons[(uint8)button].Released;
	}

	float32 Input::GetMouseX() { return s_Data.MouseX; }
	float32 Input::GetMouseY() { return s_Data.MouseY; }
	float32 Input::GetMouseDeltaX() { return s_Data.DeltaMouseX; }
	float32 Input::GetMouseDeltaY() { return s_Data.DeltaMouseY; }
	float32 Input::GetMouseScroll() { return s_Data.MouseScroll; }
}