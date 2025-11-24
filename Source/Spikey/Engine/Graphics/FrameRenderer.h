#pragma once

#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/World/World.h>

namespace Spikey {

	struct DrawIndirectCommand {

		uint32 VertexCount;
		uint32 InstanceCount;
		uint32 FirstVertex;
		uint32 FirstInstance;
	};

	class FrameRenderer {
	public:


		void BeginFrame();

		using DeferredCallback = std::function<void()>;
		
	private:
		RHICommandBuffer* m_CommandBuffers[2];
		ImGuiRTState m_ImGuiStates[2];

		std::vector<DeferredCallback> m_DeferredQueues[2];
	};
}