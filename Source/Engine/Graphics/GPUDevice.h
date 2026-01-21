#pragma once

#include <Graphics/Texture.h>
#include <Graphics/GraphicsTypes.h>
#include <Graphics/Shader.h>
#include <Graphics/Buffer.h>
#include <Graphics/Mesh.h>
#include <Core/Window.h>

#include <ImGui/imgui.h>

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

		virtual ~GPUSwapchain() = default;

	public:
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

	class GPUDevice
	{
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
	};
}