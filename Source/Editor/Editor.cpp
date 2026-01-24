#include <Editor/Editor.h>

namespace Spikey
{
	Window* Editor::CreateMainWindow()
	{
		return nullptr;
	}

	bool Editor::OnInit()
	{
		return true;
	}

	void Editor::OnUpdate(float deltaTime)
	{ 
	}

	void Editor::OnFixedUpdate()
	{
	}

	void Editor::OnDraw()
	{
	}

	void Editor::OnExit()
	{
	}

	void Editor::OnKeyPressed(Key key, KeyboardKeyMod mod)
	{
		ENGINE_TRACE("key pressed!");
	}

	void Editor::OnKeyReleased(Key key)
	{
	}

	void Editor::OnMouseButtonPressed(Key button)
	{
	}

	void Editor::OnMouseButtonReleased(Key button)
	{
	}

	void Editor::OnMouseMove(float dx, float dy)
	{
	}

	void Editor::OnMouseScroll(float value)
	{
	}
}