#pragma once
#include <Engine/Core/Common.h>

namespace Spikey {

	enum class EWindowFlags {
		None = 0,

		Fullscreen = BIT(0),
		Borderless = BIT(1),
		Resizable  = BIT(2)
	};
	ENUM_FLAGS_OPERATORS(EWindowFlags);

	struct WindowDesc {
		const char* Name;

		uint32 Width;
		uint32 Height;

		EWindowFlags Flags;
	};

	enum class EKey : uint8 {
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

		KEY_COUNT = 66
	};

	enum class EMouseButton : uint8 {
		None = 0,

		Left,
		Middle,
		Right,
		X1, X2,

		BUTTON_COUNT = 6
	};

	class IWindow {
	public:
		static IWindow* Create(const WindowDesc& desc);
		virtual ~IWindow() = default;

		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;
		virtual const char* GetName() const = 0;

		virtual bool IsFocused() const = 0;
		virtual bool IsOpen() const = 0;
		virtual bool IsMinimized() const = 0;

		virtual void* GetNativeWindow() = 0;
		virtual void Tick(float32 deltaTime) = 0;

		using KeyDelegate = std::function<void(EKey, bool)>;
		using MouseDelegate = std::function<void(float32, float32, float32, float32)>;
		using ScrollDelegate = std::function<void(float32)>;
		using MouseButtonDelegate = std::function<void(EMouseButton, bool)>;

		virtual void BindKeyDelegate(KeyDelegate&& delegate) = 0;
		virtual void BindMouseDeletage(MouseDelegate&& delegate) = 0;
		virtual void BindScrollDelegate(ScrollDelegate&& delegate) = 0;
		virtual void BindMouseButtonDelegate(MouseButtonDelegate&& delegate) = 0;
	};

	class Input {
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

		static float32 GetMouseX();
		static float32 GetMouseY();
		static float32 GetMouseDeltaX();
		static float32 GetMouseDeltaY();
		static float32 GetMouseScroll();
	}
}