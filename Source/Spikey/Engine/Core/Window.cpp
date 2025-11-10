#include <Engine/Core/Window.h>
#include <Engine/Core/Application.h>

#if BUILD_SDL_BACKEND
#include <Backend/SDL/SDLWindow.h>
#endif

namespace Spikey {

	IWindow* IWindow::Create(const WindowDesc& desc) {
#if BUILD_SDL_BACKEND
		return new SDLWindow(desc);
#else
#error No window backend selected!
#endif
	}
}

struct KeyState {
	bool Pressed = false;
	bool Down = false;
	bool Released = false;
};

struct InputData {
	KeyState Buttons[(uint8)Spikey::EMouseButton::BUTTON_COUNT];
	KeyState States[(uint8)Spikey::EKey::KEY_COUNT];

	std::vector<Spikey::EKey> Pressed;
	std::vector<Spikey::EKey> Released;

	float32 MouseX = 0;
	float32 MouseY = 0;
	float32 DeltaMouseX = 0;
	float32 DeltaMouseY = 0;
	float32 MouseScroll = 0;
};

static InputData s_Data = {};

void Spikey::Input::Init() {
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

void Spikey::Input::Tick() {

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

bool Spikey::Input::GetKeyDown(EKey key) {
	return s_Data.States[(uint8)key].Down;
}

bool Spikey::Input::GetKeyUp(EKey key) {
	return !GetKeyUp(key);
}

bool Spikey::Input::GetKeyPressed(EKey key) {
	return s_Data.States[(uint8)key].Pressed;
}

bool Spikey::Input::GetKeyReleased(EKey key) {
	return s_Data.States[(uint8)key].Released;
}

bool Spikey::Input::GetMouseButtonDown(EMouseButton button) {
	return s_Data.Buttons[(uint8)button].Down;
}

bool Spikey::Input::GetMouseButtonUp(EMouseButton button) {
	return !GetMouseButtonDown(button);
}

bool Spikey::Input::GetMouseButtonPressed(EMouseButton button) {
	return s_Data.Buttons[(uint8)button].Pressed;
}

bool Spikey::Input::GetMouseButtonReleased(EMouseButton button) {
	return s_Data.Buttons[(uint8)button].Released;
}

float32 Spikey::Input::GetMouseX() { return s_Data.MouseX; }
float32 Spikey::Input::GetMouseY() { return s_Data.MouseY; }
float32 Spikey::Input::GetMouseDeltaX() { return s_Data.DeltaMouseX; }
float32 Spikey::Input::GetMouseDeltaY() { return s_Data.DeltaMouseY; }
float32 Spikey::Input::GetMouseScroll() { return s_Data.MouseScroll; }