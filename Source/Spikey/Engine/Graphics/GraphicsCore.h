#pragma once

#include <Engine/Graphics/Texture.h>
//#include <Engine/Graphics/TextureCube.h>
#include <Engine/Graphics/Buffer.h>
#include <Engine/Graphics/Mesh.h>

#include <ImGui/imgui.h>

namespace Spikey {

	enum class ERHIQueue : uint8 
	{
		Graphics,
		Compute,
		Transfer,

		Count
	};

	struct TextureCopyRegion
	{
		uint32 BaseArrayLayer;
		uint32 MipLevel;
		uint32 LayerCount;
		Vec3Int Offset;
	};

	struct SubresourceCopyRegion
	{
		uint64 DataOffset;
		uint32 MipLevel;
		uint32 BaseLayer;
		uint32 NumLayers;
	};

	struct TextureBarrierRegion
	{
		SubresourceRange Range;
		EGPUAccess LastAccess;
		EGPUAccess NewAccess;
		bool EntireTexture;
	};

	struct RenderInfo
	{
		std::vector<IRHITextureView*> ColorTargets;
		IRHITextureView* DepthTarget;

		Vec4* ColorClear;
		Vec2* DepthClear;

		Vec2Uint DrawSize;
	};

	struct ImGuiRTState
	{

		int TotalIdxCount;
		int TotalVtxCount;
		ImVec2 DisplayPos;
		ImVec2 DisplaySize;
		ImVec2 FramebufferScale;

		struct DrawList {
			std::vector<ImDrawCmd> CmdBuffer;
			ImVector<ImDrawIdx> IdxBuffer;
			ImVector<ImDrawVert> VtxBuffer;
		};

		std::vector<DrawList> CmdLists;
	};

	class RHICommandList
	{
	public:
		virtual ~RHICommandList() = default;
		virtual void* GetNative() const = 0;

		virtual void CopyTexture(RHITexture* src, const TextureSlice& srcSlice, RHITexture* dst, const TextureSlice& dstSlice) = 0;
		virtual void FlushBarriers() = 0;

		//virtual void MipMapTexture2D(RHITexture* tex, uint32 numMips) = 0;
		//virtual void CopyTexture(RHITexture* src, const TextureCopyRegion& srcRegion, RHITexture* dst, const TextureCopyRegion& dstRegion, Vec2Uint copySize) = 0;
		//virtual void ClearTexture(RHITexture* tex, const SubresourceRange& range, const Vec4& color) = 0;
		//virtual void CopyTextureData(RHITexture* src, const SubresourceCopyRegion& region, RHIBuffer* dst) = 0;
		//virtual void BarrierTexture(RHITexture* texture, const TextureBarrierRegion* regions, uint32 numRegions) = 0;
		//virtual void CopyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint64 srcOffset, uint64 dstOffset, uint64 size) = 0;
		//virtual void BarrierBuffer(RHIBuffer* buffer, uint64 size, uint64 offset, EGPUAccess lastAccess, EGPUAccess newAccess) = 0;
		//virtual void FillBuffer(RHIBuffer* buffer, uint64 size, uint64 offset, uint32 value) = 0;
		// virtual void BindShader(RHIShader* shader, std::vector<RHIBindingSet*> shaderSets = {}, void* pushData = nullptr) = 0;
		// virtual void DispatchCompute(uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ) = 0;


		// TODO
		//virtual void BindTextureSRV() = 0;
		//virtual void BindTextureUAV() = 0;
		//virtual void BindBuffer() = 0;
		//virtual void BindSamplerState() = 0;
	};

	class IRHIDevice {
	public:
		virtual ~IRHIDevice() = default;
		virtual void RunGarbageCollection() = 0;

		virtual void CopyDataToTexture(void* src, uint64 srcOffset, IRHITexture* dst, EGPUAccess lastAccess, EGPUAccess newAccess,
			const std::vector<SubResourceCopyRegion>& regions, uint64 copySize) = 0;

		virtual TextureRHIRef CreateTexture(const TextureDesc& desc) = 0;
		virtual TextureCubeRHIRef CreateTextureCube(uint32 size, uint32 numMips, ETextureFormat format, ETextureUsage usage) = 0;
		virtual TextureViewRHIRef CreateTextureView(uint32 baseMip, uint32 numMips, uint32 baseLayer, uint32 numLayers, IRHITexture* tex) = 0;
		virtual BufferRHIRef CreateBuffer(uint64 size, EBufferUsage usage) = 0;
		virtual SamplerStateRHIRef CreateSamplerState(const SamplerStateDesc& desc) = 0;

		virtual ShaderRHIRef CreateVertexShader(const std::span<uint8>& bytecode) = 0;
		virtual ShaderRHIRef CreatePixelShader(const std::span<uint8>& bytecode) = 0;
		virtual ShaderRHIRef CreateComputeShader(const std::span<uint8>& bytecode) = 0;
		virtual PipelineStateRHIRef CreatePipelineState(const PipelineStateDesc& desc) = 0;

		virtual RHICommandList* BeginCommandList() = 0;
		virtual void SubmitCommandList(RHICommandList* cmd) = 0;
		virtual void WaitCommandList(RHICommandList* cmd) = 0;

		/*
		virtual RHIData CreateShaderRHI(const ShaderDesc& desc, const ShaderData& data, const std::vector<RHIBindingSetLayout*>& layouts) = 0;
		virtual void DestroyShaderRHI(RHIData data) = 0;
		*/

		virtual RHIData CreateSamplerRHI(const SamplerDesc& desc) = 0;
		virtual void DestroySamplerRHI(RHIData data) = 0;

		virtual RHIData CreateCommandBufferRHI() = 0;
		virtual void DestroyCommandBufferRHI(RHIData data) = 0;
		virtual void BeginFrameCommandBuffer(RHICommandBuffer* cmd) = 0;
		virtual void WaitForFrameCommandBuffer(RHICommandBuffer* cmd) = 0;
		virtual void ImmediateSubmit(std::function<void(RHICommandBuffer*)>&& func) = 0;
		virtual void WaitGPUIdle() = 0;

		virtual void BeginRendering(RHICommandBuffer* cmd, const RenderInfo& info) = 0;
		virtual void EndRendering(RHICommandBuffer* cmd) = 0;
		virtual void DrawIndirectCount(RHICommandBuffer* cmd, RHIBuffer* commBuffer, uint64 offset, RHIBuffer* countBuffer,
			uint64 countBufferOffset, uint32 maxDrawCount, uint32 commStride) = 0;
		virtual void Draw(RHICommandBuffer* cmd, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;
		virtual void DrawSwapchain(RHICommandBuffer* cmd, uint32 width, uint32 height, ImGuiRTState* guiState = nullptr, RHITexture2D* fillTexture = nullptr) = 0;
	};

	class ResourcePool {
	public:
		static RHIBufferRef GetBuffer(const BufferDesc& desc);
		static RHITexture2DRef GetTex2D(const Texture2DDesc& desc);
		static RHITextureViewRef GetTexView(const TextureViewDesc& desc);

		static void ReleaseTex2D(IRHITexture2D* tex);
		static void ReleaseTexView(IRHITextureView* view);
		static void ReleaseBuffer(IRHIBuffer* buffer);
	};

	class Graphics {
	public:
		static void Init();
		static void Shutdown();
		static void Tick();
		//void SubmitCommand(std::function<void()>&& command);
		//void SubmitResourceChange(std::function<void()>&& command);

		static IRHIDevice& GetRHI();
		static RHIBindingSetLayout* GetMeshDrawLayout();
		static RHIBindingSetLayout* GetMaterialLayout();
		static class FrameRenderer& GetRenderer();
	};
}