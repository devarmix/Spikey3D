#pragma once

#include <Engine/Core/Common.h>
#include <SDL3/SDL.h>

namespace Spikey {

	enum class WindowFlags 
	{
		None = 0,

		Fullscreen = BIT(0),
		Borderless = BIT(1),
		Resizable  = BIT(2)
	};
	ENUM_FLAGS_OPERATORS(WindowFlags);

	struct WindowDesc 
	{
		const char* Name;

		uint32 Width;
		uint32 Height;

		WindowFlags Flags;
	};

	enum class Key : uint8 
	{
		None = 0,

		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P,
		Q, R, S, T, U, V, W, X, Y, Z,

		Num0, Num1, Num2, Num3, Num4, Num5,
		Num6, Num7, Num8, Num9,

		Return, Esc, Backspace, Tab, Space,
		Minus, Equals,

		F1, F2, F3, F4, F5, F6, F7, 
		F8, F9, F10, F11, F12,

		Up, Down, Left, Right,
		LCtrl, RCtrl, LShift, RShift, LAlt, RAlt,

		MAX = 66
	};

	enum class MouseButton : uint8 
	{
		None = 0,

		Left,
		Middle,
		Right,
		X1, X2,

		MAX = 6
	};

	class Window
	{
	public:
		void* GetNativeHandle();
		SDL_Window* GetHandle();
	};

	/*
	class Input
	{
	public:
		static void Init();
		static void Tick();

		static bool GetKeyDown(EKey key);
		static bool GetKeyUp(EKey key);
		static bool GetKeyPressed(EKey key);
		static bool GetKeyReleased(EKey key);

		static bool GetMouseButtonDown(EMouseButton button);
		static bool GetMouseButtonUp(EMouseButton button);
		static bool GetMouseButtonPressed(EMouseButton button);
		static bool GetMouseButtonReleased(EMouseButton button);

		static float GetMouseX();
		static float GetMouseY();
		static float GetMouseDeltaX();
		static float GetMouseDeltaY();
		static float GetMouseScroll();
	};
	*/
}