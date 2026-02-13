#pragma once

#include <Engine/Graphics/GPUTexture.h>
#include <Engine/Graphics/GraphicsTypes.h>
#include <Engine/Graphics/GPUShader.h>
#include <Engine/Graphics/GPUBuffer.h>
#include <Engine/Core/Window.h>

#include <ThirdParty/ConcurrentQueue/concurrentqueue.h>

namespace Spikey 
{
	struct TextureCopyInfo
	{
		Vec3Int  SrcOffset = Vec3Int(0, 0, 0);
		Vec3Int  DstOffset = Vec3Int(0, 0, 0);
		uint32   SrcMipLevel;
		uint32   SrcSlice;
		uint32   DstMipLevel;
		uint32   DstSlice;
		//uint32   NumSlices;
		Vec3Uint CopySize;
	};

	constexpr uint32 FRAME_BUFFER_COUNT = 2;
	constexpr uint32 DESCRIPTOR_BINDER_CBV_COUNT = 8;
	constexpr uint32 DESCRIPTOR_BINDER_SRV_COUNT = 16;
	constexpr uint32 DESCRIPTOR_BINDER_UAV_COUNT = 8;
	constexpr uint32 DESCRIPTOR_BINDER_SAMPLER_COUNT = 8;

	// TODO: add multi queue rendering when its gonna be needed
	class GPUCommandContext
	{
	public:
		virtual ~GPUCommandContext() = default;

		virtual void UpdateTexture(GPUTexture* texture, uint32 arraySlice, uint32 mipIndex, const void* data, uint32 rowPitch, uint32 slicePitch) = 0;
		virtual void CopyTexture(GPUTexture* src, GPUTexture* dst, const TextureCopyInfo& info) = 0;
		virtual void CopyBuffer(GPUBuffer* src, uint64 srcOffset, GPUBuffer* dst, uint64 dstOffset, uint64 copySize) = 0;
		virtual void FlushBarriers() = 0;

		virtual void BindGraphicsState(GPUPipelineState* state) = 0;
		virtual void BindComputeState(GPUPipelineState* state) = 0;
		virtual void BindCB(GPUBuffer* buffer, uint32 slot) = 0;
		virtual void BindResource(GPUTextureView* view, uint32 slot) = 0;
		virtual void BindResource(GPUBuffer* buffer, uint32 slot) = 0;
		virtual void BindUAV(GPUTextureView* view, uint32 slot) = 0;
		virtual void BindUAV(GPUBuffer* buffer, uint32 slot) = 0;
		virtual void BindSamplerState(GPUSamplerState* sampler, uint32 slot) = 0;
	};

	class GPUSwapchain
	{
	protected:
		uint32 m_Width;
		uint32 m_Height;
		PixelFormat m_Format;

		GPUSwapchain()
			: m_Width(0)
			, m_Height(0)
			, m_Format(PixelFormat::Unknown)
		{
		}

	public:
		virtual ~GPUSwapchain() = default;

		uint32 GetWidth() const
		{
			return m_Width; 
		}

		uint32 GetHeight() const
		{
			return m_Height;
		}

		PixelFormat GetFormat() const
		{ 
			return m_Format;
		}

		virtual void Resize(uint32 width, uint32 height) = 0;
		virtual void Present(bool vsync, bool isPrimary) = 0;
		virtual GPUTexture* GetBackBuffer() = 0;
	};

	class GPUTask
	{
	public:
		enum class State
		{
			Created,
			Queued,
			Canceled,
			Failed,
			Finished
		};

	protected:
		State m_State = State::Created;

	public:
		virtual ~GPUTask() = default;

		void Enqueue();
		void Execute(GPUCommandContext* context);

		State GetState() const
		{
			return m_State;
		}

	protected:
		virtual bool OnExecute(GPUCommandContext* context) = 0;
	};

	class GPUTaskManager
	{
		friend GPUTask;

	private:
		struct QueueSettings : public moodycamel::ConcurrentQueueDefaultTraits
		{
			static const size_t BLOCK_SIZE = 256;
		};

		moodycamel::ConcurrentQueue<GPUTask*, QueueSettings> m_Queue;

	public:
		GPUTaskManager()
		{
		}

		void FrameBegin();
		void Dispose();
	};

	class GPUDevice
	{
	private:
		GPUTaskManager m_TaskManager;

	public:
		static GPUDevice* Instance;

	public:
		virtual ~GPUDevice() = default;

		virtual GPUTextureRef CreateTexture(const TextureDesc& desc) = 0;
		virtual GPUBufferRef CreateBuffer(const BufferDesc& desc) = 0;
		virtual GPUSamplerStateRef CreateSamplerState(const SamplerStateDesc& desc) = 0;
		virtual GPUShaderRef CreateShader(std::span<uint8> bytecode, ShaderStage stage) = 0;
		virtual GPUPipelineStateRef CreatePipelineState(const PipelineStateDesc& desc) = 0;
		virtual GPUSwapchain* CreateSwapchain(uint32 width, uint32 height, PixelFormat desiredFormat, SDL_Window* window) = 0;
		virtual GPUCommandContext* GetMainContext() = 0;

		virtual void BeginFrame() = 0;
		virtual void WaitGPUIdle() = 0;

		GPUTaskManager& GetTaskManager()
		{
			return m_TaskManager;
		}
	};
}