#pragma once

#include <Engine/Graphics/Texture2D.h>
#include <Engine/Graphics/Buffer.h>
#include <Engine/Graphics/Mesh.h>

#include <ImGui/imgui.h>

namespace Spikey {

	class RHICommandBuffer : public IRHIResource {
	public:
		RHICommandBuffer() : m_RHIData(0) {}
		virtual ~RHICommandBuffer() override = default;

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;

		RHIData GetRHIData() const { return m_RHIData; }

	private:
		RHIData m_RHIData;
	};

	struct TextureCopyRegion {
		uint32 BaseArrayLayer;
		uint32 MipLevel;
		uint32 LayerCount;
		Vec3Int Offset;
	};

	struct SubResourceCopyRegion {
		uint64 DataOffset;
		uint32 MipLevel;
		uint32 ArrayLayer;
	};

	struct TextureBarrierRegion {
		uint32 BaseMipLevel;
		uint32 MipCount;
		uint32 BaseArrayLayer;
		uint32 LayerCount;

		bool EntireTexture;
		EGPUAccess LastAccess;
		EGPUAccess NewAccess;
	};

	struct RenderInfo {
		std::vector<RHITextureView*> ColorTargets;
		RHITextureView* DepthTarget;

		Vec4* ColorClear;
		Vec2* DepthClear;

		Vec2Uint DrawSize;
	};

	struct ImGuiRTState {

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

	class IRHIDevice {
	public:
		virtual ~IRHIDevice() = default;

		virtual RHIData CreateTexture2DRHI(const Texture2DDesc& desc) = 0;
		virtual void DestroyTexture2DRHI(RHIData data) = 0;
		virtual void MipMapTexture2D(RHICommandBuffer* cmd, RHITexture2D* tex, EGPUAccess lastAccess, EGPUAccess newAccess, uint32 numMips) = 0;

		virtual RHIData CreateCubeTextureRHI(const TextureCubeDesc& desc) = 0;
		virtual void DestroyCubeTextureRHI(RHIData data) = 0;

		virtual void CopyTexture(RHICommandBuffer* cmd, IRHITexture* src, const TextureCopyRegion& srcRegion, IRHITexture* dst, const TextureCopyRegion& dstRegion, Vec2Uint copySize) = 0;
		virtual void ClearTexture(RHICommandBuffer* cmd, IRHITexture* tex, EGPUAccess access, const Vec4& color) = 0;

		virtual void CopyDataToTexture(void* src, uint64 srcOffset, IRHITexture* dst, EGPUAccess lastAccess, EGPUAccess newAccess,
			const std::vector<SubResourceCopyRegion>& regions, uint64 copySize) = 0;
		virtual void CopyFromTextureToCPU(RHICommandBuffer* cmd, IRHITexture* src, SubResourceCopyRegion region, RHIBuffer* dst) = 0;
		virtual void BarrierTexture(RHICommandBuffer* cmd, IRHITexture* texture, const TextureBarrierRegion* regions, uint32 numRegions) = 0;

		virtual RHIData CreateTextureViewRHI(const TextureViewDesc& desc) = 0;
		virtual void DestroyTextureViewRHI(RHIData data) = 0;

		virtual RHIData CreateBufferRHI(const BufferDesc& desc) = 0;
		virtual void DestroyBufferRHI(RHIData data) = 0;
		virtual void CopyBuffer(RHICommandBuffer* cmd, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint64 srcOffset, uint64 dstOffset, uint64 size) = 0;
		virtual void* MapBufferMem(RHIBuffer* buffer) = 0;
		virtual void BarrierBuffer(RHICommandBuffer* cmd, RHIBuffer* buffer, uint64 size, uint64 offset, EGPUAccess lastAccess, EGPUAccess newAccess) = 0;
		virtual void FillBuffer(RHICommandBuffer* cmd, RHIBuffer* buffer, uint64 size, uint64 offset, uint32 value) = 0;
		virtual uint64_t GetBufferGPUAddress(RHIBuffer* buffer) = 0;

		virtual RHIData CreateBindingSetLayoutRHI(const BindingSetLayoutDesc& desc) = 0;
		virtual void DestroyBindingSetLayoutRHI(RHIData data) = 0;

		virtual RHIData CreateBindingSetRHI(RHIBindingSetLayout* layout) = 0;
		virtual void DestroyBindingSetRHI(RHIData data) = 0;

		virtual RHIData CreateShaderRHI(const ShaderDesc& desc, const std::vector<uint8>* vertexData, const std::vector<uint8>* pixelData,
			const std::vector<uint8>* computeData, uint32 pushSize, const std::vector<RHIBindingSetLayout*>& layouts) = 0;
		virtual void DestroyShaderRHI(RHIData data) = 0;
		virtual void BindShader(RHICommandBuffer* cmd, RHIShader* shader, std::vector<RHIBindingSet*> shaderSets = {}, void* pushData = nullptr) = 0;

		virtual RHIData CreateSamplerRHI(const SamplerDesc& desc) = 0;
		virtual void DestroySamplerRHI(RHIData data) = 0;

		virtual RHIData CreateCommandBufferRHI() = 0;
		virtual void DestroyCommandBufferRHI(RHIData data) = 0;
		virtual void BeginFrameCommandBuffer(RHICommandBuffer* cmd) = 0;
		virtual void WaitForFrameCommandBuffer(RHICommandBuffer* cmd) = 0;
		virtual void ImmediateSubmit(std::function<void(RHICommandBuffer*)>&& func) = 0;
		virtual void DispatchCompute(RHICommandBuffer* cmd, uint32 groupCountX, uint32 groupCountY, uint32 groupCountZ) = 0;
		virtual void WaitGPUIdle() = 0;

		virtual void BeginRendering(RHICommandBuffer* cmd, const RenderInfo& info) = 0;
		virtual void EndRendering(RHICommandBuffer* cmd) = 0;
		virtual void DrawIndirectCount(RHICommandBuffer* cmd, RHIBuffer* commBuffer, uint64 offset, RHIBuffer* countBuffer,
			uint64 countBufferOffset, uint32 maxDrawCount, uint32 commStride) = 0;
		virtual void Draw(RHICommandBuffer* cmd, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance) = 0;
		virtual void DrawSwapchain(RHICommandBuffer* cmd, uint32 width, uint32 height, ImGuiRTState* guiState = nullptr, RHITexture2D* fillTexture = nullptr) = 0;
	};

	class FrameRenderer;

	namespace Graphics {

		void Init();
		void Shutdown();
		void Tick();

		IRHIDevice& GetRHI();
		FrameRenderer& GetFrameRenderer();
		RHISampler* GetCachedSampler(const SamplerDesc& desc);

		uint32 GetMaterialID();
		uint32 GetShaderTextureID(RHITextureView* view);
		uint32 GetShaderSamplerID(RHISampler* sampler);

		void FreeShaderTextureID(uint32 id);
		void FreeShaderSamplerID(uint32 id);
		void FreeMaterialID(uint32 id);
		void UpdateMaterial(uint32 id);

		struct alignas(16) MaterialData {

			float ScalarData[16];
			uint32 UintData[16];
			Vec2 Float2Data[16];
			Vec4 Float4Data[16];
			uint32 TextureData[16];
			uint32 SamplerData[16];
		};

		MaterialData& GetMaterialData(uint32 id);
		RHIBindingSet* GetMaterialSet();
		RHIBindingSetLayout* GetMeshDrawLayout();
		RHIBindingSetLayout* GetMaterialLayout();
	}
}