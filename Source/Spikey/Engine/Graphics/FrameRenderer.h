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

	namespace TransientPool {

		RHITexture2D* FindTexture(const Texture2DDesc& desc);
		RHIBuffer* FindBuffer(const BufferDesc& desc);
		RHITextureView* FindTextureView(const TextureViewDesc& desc);
		RHIBindingSet* FindBindingSet(RHIBindingSetLayout* layout);

		void ReleaseTexture(RHITexture2D* texture);
		void ReleaseBuffer(RHIBuffer* buffer);
		void ReleaseTextureView(RHITextureView* view);
		void ReleaseBindingSet(RHIBindingSet* set);
	}

	class FrameRenderer {
	public:


		void BeginFrame();

		using DeferredCallback = std::function<void()>;

		// calls the callback when frame finishes rendering on render thread
		// useful when need to safely destruct / change RHI resource
		void EnqueueDeferred(DeferredCallback&& callback);
		
	private:
		RHICommandBuffer* m_CommandBuffers[2];
		ImGuiRTState m_ImGuiStates[2];

		std::vector<DeferredCallback> m_DeferredQueues[2];
	};
}