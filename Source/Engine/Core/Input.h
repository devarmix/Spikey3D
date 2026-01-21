#pragma once

#include <Core/Common.h>

namespace Spikey
{
	enum class Key : int32
	{
		Invalid = 0,

		Tab,
		LeftArrow,
		RightArrow,
		UpArrow,
		DownArrow,
		PageUp,
		PageDown,
		Home,
		End,
		Insert,
		Delete,
		Backspace,
		Space,
		Enter,
		Escape,
		LeftCtrl,
		LeftShift,
		LeftAlt,
		RightCtrl,
		RightShift,
		RightAlt,
		Menu,

		Zero, One, Two, Three, Four,
		Five, Six, Seven, Eight, Nine,

		A, B, C, D, E, F, G, H, I, J,
		K, L, M, N, O, P, Q, R, S, T,
		U, V, W, X, Y, Z,
		F1, F2, F3, F4, F5, F6,
		F7, F8, F9, F10, F11, F12,

		Apostrophe,
		Comma,
		Minus,
		Period,
		Slash,
		Semicolon,
		Equal,
		LeftBracket,
		Backslash,
		RightBracket,
		GraveAccent,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,

		Keypad_Zero,
		Keypad_One,
		Keypad_Two,
		Keypad_Three,
		Keypad_Four,
		Keypad_Five,
		Keypad_Six,
		Keypad_Seven,
		Keypad_Eight,
		Keypad_Nine,
		Keypad_Decimal,
		Keypad_Divide,
		Keypad_Multiply,
		Keypad_Subtract,
		Keypad_Add,
		Keypad_Enter,
		Keypad_Equal,

		Gamepad_Start,
		Gamepad_Back,
		Gamepad_LeftX,
		Gamepad_LeftY,
		Gamepad_RightX,
		Gamepad_RightY,
		Gamepad_L1,
		Gamepad_R1,
		Gamepad_R2,
		Gamepad_L2,
		Gamepad_R3,
		Gamepad_L3,
		Gamepad_FaceBottom,
		Gamepad_FaceRight,
		Gamepad_FaceLeft,
		Gamepad_FaceTop,
		Gamepad_DPadRight,
		Gamepad_DPadUp,
		Gamepad_DPadLeft,
		Gamepad_DPadDown,

		Mouse_X,
		Mouse_Y,
		Mouse_LeftButton,
		Mouse_RightButton,
		Mouse_MiddleButton,
		Mouse_X1Button,
		Mouse_X2Button,
		Mouse_Scroll
	};

	enum class KeyboardKeyMod : int32
	{
		None = 0,
		Ctrl = 1 << 0,
		Shift = 1 << 1,
		Alt = 1 << 2
	};
	ENUM_FLAGS_OPERATORS(KeyboardKeyMod);

	struct Input
	{
		static void Init();

		enum DeviceType
		{
			KeyboardAndMouse,
			PsController,
			XboxController
		};

		static DeviceType GetActiveDevice();
		static Key GetKeyFromName(const std::string& name);
	};

	struct InputTrigger
	{
		enum TriggerEvent
		{
			Default = 0,
			Pressed,
			Released,
			Down
		};

		Key TriggerKey;
		float DeadzoneMin = 0.f;
		float DeadzoneMax = 1.f;
		float ValueScale = 1.f;
		float LastValue = 0.f;
		TriggerEvent Event = Default;
	};

	struct InputAction
	{
		enum Type
		{
			Boolean,
			Axis
		};

		Type ActionType;
		float Value;
		float ClampMin;
		float ClampMax;

		std::string Name;
		std::vector<InputTrigger> Triggers;
	};

	class InputActionContext
	{
	public:
		InputActionContext();

		bool FromJson(const std::filesystem::path& path);
		void Tick();

	private:
		std::map<std::string, InputAction> m_ActionMap;
	};
}