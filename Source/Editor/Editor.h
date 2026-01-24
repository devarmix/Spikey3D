#pragma once

#include <Engine/Core/Engine.h>

namespace Spikey
{
	class Editor : public ApplicationBase
	{
	public:
		static Window* CreateMainWindow();

		static bool OnInit();
		static void OnUpdate(float deltaTime);
		static void OnFixedUpdate();
		static void OnDraw();
		static void OnExit();

		static void OnKeyPressed(Key key, KeyboardKeyMod mod);
		static void OnKeyReleased(Key key);
		static void OnMouseButtonPressed(Key button);
		static void OnMouseButtonReleased(Key button);
		static void OnMouseMove(float dx, float dy);
		static void OnMouseScroll(float value);
	};
}