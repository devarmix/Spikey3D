#pragma once

#include <Core/Common.h>

namespace Spikey
{
	class Engine
	{
	public:
		static uint64 FrameCounter;

		static int32 Main(int32 argc, char* argv[])
		{

		}

		void Exit();
		void RequestExit();
		void Crash();

		bool IsEditor();
		bool IsPlayMode();


		void OnUpdate();
		void OnPhysics();
	};
}