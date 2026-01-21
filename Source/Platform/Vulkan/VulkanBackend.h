#pragma once

#include <Vulkan/VulkanCommon.h>
#include <Graphics/GPUDevice.h>

#if (VK_HEADER_VERSION < 318)
#error "Vulkan SDK version 1.4.318 or later is required to compile Spikey3D"
#endif

namespace Spikey::Vulkan 
{
	class  VulkanCommandContext;
	class  VulkanDevice;
	class  VulkanTexture;
	class  VulkanBuffer;
	class  VulkanSamplerState;
	class  VulkanPipelineState;
	struct VulkanPSOLayout;
	struct VulkanPSOLayoutHash;

	class VulkanDeletionQueue
	{
	public:
		enum class Type
		{
			Buffer,
			Image,
			ImageView,
			Pipeline,
			Sampler,
			DescriptorPool
		};

		VulkanDeletionQueue()
		{
		}

		~VulkanDeletionQueue()
		{
			ReleaseResources(true);
		}

		template<typename T>
		void Enqueue(Type type, T handle)
		{
			static_assert(sizeof(T) <= sizeof(uint64), "Invalid handle size.");
			EnqueueGeneric(type, (uint64)handle, VK_NULL_HANDLE);
		}
		
		template<typename T>
		void Enqueue(Type type, T handle, VmaAllocation allocation)
		{
			static_assert(sizeof(T) <= sizeof(uint64), "Invalid handle size.");
			EnqueueGeneric(type, (uint64)handle, allocation);
		}

		void ReleaseResources(bool immediate = false, VulkanDevice* device);

	private:
		struct Entry
		{
			Type          StructureType;
			uint64        FrameNumber;
			VmaAllocation AllocationHandle;
			uint64        Handle;
		};

		std::mutex         m_Mutex;
		std::deque<Entry>  m_Entries;

	private:
		void EnqueueGeneric(Type type, uint64 handle, VmaAllocation allocation);
	};

	class VulkanDevice : public GPUDevice
	{
	public:
		VkSampler     StaticSamplers[7];
		VkBuffer      NullBuffer;
		VmaAllocation NullBufferAllocation;
		VkImage       NullImage2D;
		VkImage       NullImage3D;
		VmaAllocation NullImageAllocation2D;
		VmaAllocation NullImageAllocation3D;
		VkImageView   NullImageView2D;
		VkImageView   NullImageView2DArray;
		VkImageView   NullImageView3D;
		VkImageView   NullImageViewCube;

		VkFence FrameFences[FRAME_BUFFER_COUNT];
		VulkanCommandContext* MainContext;

	public:
		VulkanDevice();
		virtual ~VulkanDevice() override;

		VkInstance       GetInstanceHandle() const { return m_Instance; }
		VkDevice         GetDeviceHandle() const { return m_Device; }
		VkPhysicalDevice GetPhysicalHandle() const { return m_PhysicalDevice; }
		VmaAllocator     GetAllocatorHandle() const { return m_Allocator; }

		VulkanPSOLayout  CreateCachedPSOLayout(const VulkanPSOLayoutHash& hash);
		VkPipelineCache  GetPipelineCacheHandle() const { return m_PipelineStateCache; }

		const VkPhysicalDeviceLimits& GetLimits() const { return m_Properties.limits; }
		const VkPhysicalDeviceProperties& GetProperties() const { return m_Properties; }

		VulkanDeletionQueue& GetDeletionQueue()
		{
			return m_DeletionQueue;
		}

		virtual GPUTextureRef CreateTexture(const TextureDesc& desc) override;
		virtual GPUBufferRef CreateBuffer(const BufferDesc& desc) override;
		virtual GPUSamplerStateRef CreateSamplerState(const SamplerStateDesc& desc) override;
		virtual GPUShaderRef CreateShader(std::span<uint8> bytecode, ShaderStage stage) override;
		virtual GPUPipelineStateRef CreatePipelineState(const PipelineStateDesc& desc) override;
		virtual GPUSwapchain* CreateSwapchain(uint32 width, uint32 height, PixelFormat desiredFormat, SDL_Window* window) override;

		virtual GPUCommandContext* GetMainContext() override
		{
			return MainContext;
		}

		virtual void WaitGPUIdle() override
		{
			VK_CHECK(vkDeviceWaitIdle(m_Device));
		}

		virtual void BeginFrame() override;

	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_PhysicalDevice;
		VkPhysicalDeviceProperties m_Properties;
		VkDevice m_Device;
		VmaAllocator m_Allocator;
		VulkanDeletionQueue m_DeletionQueue;
		VkPipelineCache m_PipelineStateCache;

		std::mutex m_LayoutCacheLock;
		std::unordered_map<VulkanPSOLayoutHash, VulkanPSOLayout> m_LayoutCache;
		std::mutex m_SamplerCacheLock;
		std::unordered_map<SamplerStateDesc, GPUSamplerStateRef> m_SamplerCache;

		VkQueue m_GraphicsQueue;
		uint32 m_GraphicsQueueFamily;
	};

	class VulkanDescriptorResource
	{
	public:
		virtual ~VulkanDescriptorResource() = default;

	public:
		virtual void DescriptorAsSampler(VkSampler& sampler)
		{
			assert(false && "Invalid descriptor usage!");
		}

		virtual void DescriptorAsImage(VulkanCommandContext* context, VkImageView& view, VkImageLayout& layout)
		{
			assert(false && "Invalid descriptor usage!");
		}

		virtual void DescriptorAsStorageImage(VulkanCommandContext* context, VkImageView& view, VkImageLayout& layout)
		{
			assert(false && "Invalid descriptor usage!");
		}

		virtual void DescriptorAsStorageBuffer(VulkanCommandContext* context, VkBuffer& buffer, VkDeviceSize& offset, VkDeviceSize& range, bool isSRV = false)
		{
			assert(false && "Invalid descriptor usage!");
		}

		virtual void DescriptorAsUniformBuffer(VulkanCommandContext* context, VkBuffer& buffer, VkDeviceSize& offset, VkDeviceSize& range)
		{
			assert(false && "Invalid descriptor usage!");
		}
	};

	class VulkanDescriptorBinder
	{
	public:
		VulkanDescriptorBinder(VulkanDevice* device);

		~VulkanDescriptorBinder()
		{
			for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
			{
				Destroy(Pools[i]);
			}
		}

		struct DescriptorPool
		{
			VkDescriptorPool Handle = VK_NULL_HANDLE;
			uint32           Capacity = 256;
		};

		void InitPool(DescriptorPool& pool);
		void Destroy(DescriptorPool& pool);
		void Reset();
		void FlushDescriptors(VulkanPipelineState* pipeline, bool graphics, VulkanCommandContext* context);

	public:
		DescriptorPool Pools[FRAME_BUFFER_COUNT] = {};
		bool           Dirty = false;

		struct
		{
			VulkanDescriptorResource* CBV[DESCRIPTOR_BINDER_CBV_COUNT];
			VulkanDescriptorResource* SRV[DESCRIPTOR_BINDER_SRV_COUNT];
			VulkanDescriptorResource* UAV[DESCRIPTOR_BINDER_UAV_COUNT];
			VulkanDescriptorResource* Sampler[DESCRIPTOR_BINDER_SAMPLER_COUNT];
		} DescriptorTable;

		std::vector<VkDescriptorImageInfo> ImageInfos;
		std::vector<VkDescriptorBufferInfo> BufferInfos;
		std::vector<VkWriteDescriptorSet> Writes;

	private:
		VulkanDevice* m_Device;
	};

	class VulkanCommandBuffer
	{
	public:
		uint64 SubmitID = 0;
		std::vector<VkSemaphoreSubmitInfo> WaitSemaphores;
		std::vector<VkSemaphoreSubmitInfo> SignalSemaphores;

	public:
		VulkanCommandBuffer(VkCommandPool pool, VulkanDevice* device);
		~VulkanCommandBuffer();

	public:
		void AddWaitSemaphore(VkSemaphore semaphore, uint64 value = 1, VkPipelineStageFlags2 stageFlags = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT);
		void AddSignalSemaphore(VkSemaphore semaphore, uint64 value = 1, VkPipelineStageFlags2 stageFlags = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT);

		VkCommandBuffer GetCmdHandle() const
		{
			return m_CmdBuffer;
		}

	private:
		VulkanDevice* m_Device;
		VkCommandPool m_CmdPool;
		VkCommandBuffer m_CmdBuffer;
	};

	class VulkanCmdBufferManager
	{
	public:
		VulkanCmdBufferManager(VkQueue queue, uint32 queueFamilyIndex, VulkanDevice* device);

		~VulkanCmdBufferManager()
		{
			vkDestroyCommandPool(m_Device->GetDeviceHandle(), m_Pool, nullptr);
			vkDestroySemaphore(m_Device->GetDeviceHandle(), m_TrackingSemaphore, nullptr);
		}

		// FIXME: when submiting, set bound pipeline states in the comamnd context as dirty
		void                 SubmitActiveCmd(VkFence fence = VK_NULL_HANDLE);
		VulkanCommandBuffer* GetActive();

	private:
		VulkanDevice* m_Device;
		VkCommandPool m_Pool;
		VkSemaphore m_TrackingSemaphore;
		VkQueue m_Queue;

		uint64 m_SubmitCounter = 0;
		uint64 m_LastFinishedID = 0;
		TSharedPtr<VulkanCommandBuffer> m_ActiveCmdBuffer;
		std::deque<TSharedPtr<VulkanCommandBuffer>> m_CmdInFlight;
		std::vector<TSharedPtr<VulkanCommandBuffer>> m_FreeCmdBuffers;
	};

	class VulkanCommandContext : public GPUCommandContext
	{
	public:
		VulkanCommandContext(VkQueue queue, uint32 queueIndex, VulkanDevice* device)
			: m_Device(device)
			, m_DescriptorBinder(device)
			, m_CmdBufferManager(queue, queueIndex, device)
		{
		}

		virtual ~VulkanCommandContext() override
		{
		}

		virtual void CopyTexture(GPUTexture* src, GPUTexture* dst, const TextureCopyInfo& info) override;
		virtual void CopyBuffer(GPUBuffer* src, uint64 srcOffset, GPUBuffer* dst, uint64 dstOffset, uint64 copySize) override;
		virtual void FlushBarriers() override;

		virtual void BindGraphicsState(GPUPipelineState* state) override;
		virtual void BindComputeState(GPUPipelineState* state) override;
		virtual void BindCB(GPUBuffer* buffer, uint32 slot) override;
		virtual void BindResource(GPUTextureView* view, uint32 slot) override;
		virtual void BindResource(GPUBuffer* buffer, uint32 slot) override;
		virtual void BindUAV(GPUTextureView* view, uint32 slot) override;
		virtual void BindUAV(GPUBuffer* buffer, uint32 slot) override;
		virtual void BindSamplerState(GPUSamplerState* sampler, uint32 slot) override;

		void AddImageBarrier(VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout, const VkImageSubresourceRange& subresourceRange);
		void AddImageBarrier(VulkanTextureView* view, VkImageLayout dstLayout);
		void AddImageBarrier(VulkanTexture* texture, uint32 mipSlice, uint32 arraySlice, VkImageLayout dstLayout);
		void AddImageBarrier(VulkanTexture* texture, VkImageLayout dstLayout);
		void AddBufferBarrier(VulkanBuffer* buffer, VkAccessFlags2 dstAccess);
		void AddBufferBarrier(VulkanBuffer* buffer, uint64 offset, uint64 size, VkAccessFlags2 dstAccess);

		void BeginFrame()
		{
			m_GraphicsState = nullptr;
			m_ComputeState = nullptr;
			m_DescriptorBinder.Reset();
		}

		VulkanCmdBufferManager& GetCmdManager()
		{
			return m_CmdBufferManager;
		}

	private:
		VulkanDevice* m_Device;

		VulkanCmdBufferManager m_CmdBufferManager;
		VulkanDescriptorBinder m_DescriptorBinder;
		std::vector<VkImageMemoryBarrier2>  m_ImageBarriers;
		std::vector<VkBufferMemoryBarrier2> m_BufferBarriers;

		VulkanPipelineState* m_GraphicsState = nullptr;
		VulkanPipelineState* m_ComputeState = nullptr;
	};

	class VulkanBuffer : public GPUBuffer, public VulkanDescriptorResource
	{
	public:
		VkAccessFlags2 Access;

		VulkanBuffer(const BufferDesc& desc, VulkanDevice& device);
		virtual ~VulkanBuffer() override;

		virtual void* GetMappedData() const override { return m_MappedData; }
		virtual void* GetNative() const override { return (void*)m_Buffer; }
		VkBuffer      GetBufferHandle() const { return m_Buffer; }

		virtual void DescriptorAsStorageBuffer(VulkanCommandContext* context, VkBuffer& buffer, VkDeviceSize& offset, VkDeviceSize& range, bool isSRV = false) override;
		virtual void DescriptorAsUniformBuffer(VulkanCommandContext* context, VkBuffer& buffer, VkDeviceSize& offset, VkDeviceSize& range) override;

	private:
		VulkanDevice& m_Device;
		VkBuffer      m_Buffer;
		VmaAllocation m_Allocation;
		void*         m_MappedData;
	};

	struct VulkanTextureState
	{
		VkImageLayout              State;
		bool                       AllSubresourcesSame;
		std::vector<VkImageLayout> SubresourceStates;

		void Initialize(uint32 subresourceCount, VkImageLayout initialState, bool usePerSubresourceTracking)
		{
			assert(SubresourceStates.empty() && subresourceCount > 0);
			AllSubresourcesSame = true;
			State = initialState;

			if (usePerSubresourceTracking && subresourceCount > 1)
				SubresourceStates.resize(subresourceCount);
		}

		VkImageLayout GetSubresourceState(uint32 index)
		{
			if (AllSubresourcesSame)
				return State;
			return SubresourceStates[index];
		}

		void SetState(VkImageLayout state)
		{
			AllSubresourcesSame = true;
			State = state;
		}

		void SetSubresourceState(uint32 index, VkImageLayout state)
		{
			if (index == -1 || SubresourceStates.size() <= 1)
			{
				SetState(state);
			}
			else
			{
				assert(index < SubresourceStates.size());

				// transition for all subresources
				if (AllSubresourcesSame)
				{
					for (int32 i = 0; i < SubresourceStates.size(); i++)
						SubresourceStates[i] = State;
					AllSubresourcesSame = false;
				}

				SubresourceStates[index] = state;
			}
		}
	};

	class VulkanTextureView : public GPUTextureView, public VulkanDescriptorResource
	{
	public:
		VkImageView View;
		VkImageView PartialView;
		VkImageLayout LayoutSRV;
		VkImageLayout LayoutRTV;
		VkImageSubresourceRange Subresources;
		VkImageViewType ViewType;
		VulkanDevice* Device;

	public:
		VulkanTextureView()
			: View(nullptr)
			, PartialView(nullptr)
			, LayoutSRV(VK_IMAGE_LAYOUT_UNDEFINED)
			, LayoutRTV(VK_IMAGE_LAYOUT_UNDEFINED)
			, ViewType(VK_IMAGE_VIEW_TYPE_MAX_ENUM)
			, Device(nullptr)
		{
		}

		virtual ~VulkanTextureView() override
		{
#if _DEBUG
			assert(View == nullptr);
#endif
		}

		void Init(VulkanDevice* device, VulkanTexture* owner, VkImageViewType viewType, PixelFormat format,
			uint32 baseMip, uint32 mipLevels, uint32 baseLayer, uint32 numLayers, bool readOnlyDepth = false, bool stencilView = false);
		void Release();

		virtual void DescriptorAsImage(VulkanCommandContext* context, VkImageView& view, VkImageLayout& layout) override;
		virtual void DescriptorAsStorageImage(VulkanCommandContext* context, VkImageView& view, VkImageLayout& layout) override;
	};

	class VulkanTexture : public GPUTexture
	{
	public:
		VulkanTextureState State;
		VkImageAspectFlags AspectFlags;

		VulkanTexture(const TextureDesc& desc, VkImage image, VulkanDevice* device);
		VulkanTexture(const TextureDesc& desc, VulkanDevice* device);
		virtual ~VulkanTexture() override;

		VkImage GetImageHandle() const { return m_Image; }
		virtual void* GetNative() const override { return (void*)m_Image; }

	private:
		void InitViews();

	public:
		virtual GPUTextureView* View(int32 arrayIndex) const override
		{ 
			return (GPUTextureView*)&m_ViewsPerSlice[arrayIndex]; 
		}

		virtual GPUTextureView* View(int32 arrayIndex, int32 mipLevel) const override
		{ 
			return (GPUTextureView*)&m_ViewsPerMip[arrayIndex][mipLevel]; 
		}

		virtual GPUTextureView* ViewArray() const override
		{
			assert(m_Desc.ArraySize > 1);
			return (GPUTextureView*)&m_ViewArray;
		}

		virtual GPUTextureView* ViewVolume() const override
		{
			assert(m_Desc.Dimension == TextureDimension::Texture3D);
			return (GPUTextureView*)&m_ViewVolume;
		}

		virtual GPUTextureView* ViewReadOnlyDepth() const override
		{
			assert(EnumHasAllFlags(m_Desc.Flags, TextureFlags::DepthStencil));
			return (GPUTextureView*)&m_ViewReadOnlyDepth;
		}

		virtual GPUTextureView* ViewStencil() const override
		{
			assert(EnumHasAllFlags(m_Desc.Flags, TextureFlags::DepthStencil));
			return (GPUTextureView*)&m_ViewStencil;
		}

	private:
		VulkanDevice* m_Device;
		VkImage m_Image;
		VmaAllocation m_Allocation;

		VulkanTextureView m_ViewArray;
		VulkanTextureView m_ViewVolume;
		VulkanTextureView m_ViewReadOnlyDepth;
		VulkanTextureView m_ViewStencil;
		std::vector<VulkanTextureView> m_ViewsPerSlice;
		std::vector<std::vector<VulkanTextureView>> m_ViewsPerMip;
	};

	class VulkanSwapchain : public GPUSwapchain
	{
	public:
		VulkanSwapchain(uint32 width, uint32 height, PixelFormat desiredFormat, SDL_Window* window, VkQueue presentQueue, VulkanDevice* device);

		virtual ~VulkanSwapchain() override
		{
			ReleaseSwapchainResources(true);
		}

		virtual void Resize(uint32 width, uint32 height) override;
		virtual void Present(bool vsync, bool isPrimary) override;
		virtual GPUTexture* GetBackBuffer() override;

	private:
		void     CreateSwapchain();
		void     CreateSurface();
	    VkResult AcquireImageIndex();
		void     ReleaseSwapchainResources(bool destroySwapchain);
		void     RecreateSwapchain();

	private:
		VulkanDevice* m_Device;
		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_Swapchain;
		VkQueue m_PresentQueue;
		SDL_Window* m_WindowHandle;

		struct BackBuffer
		{
			TRefCountPtr<VulkanTexture> Texture;
			VkSemaphore RenderingDoneSemaphore;
		};

		bool m_Outdated = false;
		bool m_VSyncEnabled = true;
		bool m_ImageAcquired = false;
		uint32 m_AcquiredImageIndex = 0;
		uint32 m_SemaphoreIndex = 0;

		std::vector<BackBuffer> m_Backbuffers;
		std::vector<VkSemaphore> m_AcquireSemaphores;
	};

	class VulkanSamplerState : public GPUSamplerState, public VulkanDescriptorResource
	{
	public:
		VulkanSamplerState(const SamplerStateDesc& desc, VulkanDevice* device);
		virtual ~VulkanSamplerState() override;

		VkSampler     GetSamplerHandle() const { return m_Sampler; }
		virtual void* GetNative() const override { return (void*)m_Sampler; }

		virtual void DescriptorAsSampler(VkSampler& sampler) override
		{
			sampler = m_Sampler;
		}

	private:
		VulkanDevice* m_Device;
		VkSampler m_Sampler;
	};

	// to remove overlapping descriptors
	constexpr uint32 VULKAN_BINDING_SHIFT_B = 0;
	constexpr uint32 VULKAN_BINDING_SHIFT_T = 1000;
	constexpr uint32 VULKAN_BINDING_SHIFT_U = 2000;
	constexpr uint32 VULKAN_BINDING_SHIFT_S = 3000;
	constexpr uint32 VULKAN_IMMUTABLE_SAMPLER_FIRST_BINDING = 100;

	struct SpirvDescriptorInfo
	{
		enum
		{
			MaxDescriptors = 64
		};

		enum class BindingType : uint8
		{
			Unknown = 0,
			CBV,
			SRV,
			UAV,
			Sampler
		};

		enum class ResourceType : uint8
		{
			Unknown = 0,
			ConstantBuffer,
			Buffer,
			Sampler,
			Texture2D,
			Texture3D,
			TextureCube,
			Texture2DArray
		};

		struct Binding
		{
			uint8            Binding;
			bool             IsImmutable;
			ResourceType     Resource;
			BindingType      Type;
			uint32           Count;
			VkDescriptorType DescriptorType;
		};

		uint32  PushConstantSize;
		uint32  PushConstantOffset;
		uint32  DescriptorCount;
		Binding Descriptors[MaxDescriptors];
	};

	class VulkanShader : public GPUShader
	{
	public:
		SpirvDescriptorInfo DescriptorInfo;
		VkShaderStageFlags StageFlags;

		VulkanShader(std::span<uint8> bytecode, ShaderStage stage, VulkanDevice* device);
		virtual ~VulkanShader() override;

		VkShaderModule       GetShaderHandle() const { return m_Module; }
		virtual void*        GetNative() const override { return (void*)m_Module; }
		virtual ShaderStage  GetStage() const override { return m_Stage; }

	private:
		VulkanDevice*  m_Device;
		VkShaderModule m_Module;
		ShaderStage    m_Stage;
	};

	class VulkanPipelineState : public GPUPipelineState
	{
	public:
		SpirvDescriptorInfo DescriptorInfo;

		VulkanPipelineState(const PipelineStateDesc& desc, VulkanDevice* device);
		virtual ~VulkanPipelineState() override;

		VkPipelineLayout      GetLayoutHandle() const { return m_Layout; }
		VkPipeline            GetPipelineHandle() const { return m_Pipeline; }
		VkDescriptorSetLayout GetSetLayoutHandle() const { return m_SetLayout; }

	private:
		VulkanDevice*         m_Device;
		VkPipelineLayout      m_Layout;
		VkPipeline            m_Pipeline;
		VkDescriptorSetLayout m_SetLayout;
	};

	struct VulkanPSOLayout
	{
		VkPipelineLayout Layout;
		VkDescriptorSetLayout SetLayout;
	};

	struct VulkanPSOLayoutHash
	{
		std::vector<VkDescriptorSetLayoutBinding> Bindings;
		VkPushConstantRange PushConstants;
		uint64 Hash;

		bool operator=(const VulkanPSOLayoutHash& other) const
		{
			if (Hash != other.Hash)
				return false;
			if (Bindings.size() != other.Bindings.size())
				return false;
			if (
				PushConstants.size != other.PushConstants.size ||
				PushConstants.offset != other.PushConstants.offset ||
				PushConstants.stageFlags != other.PushConstants.stageFlags
				)
				return false;

			for (uint32 i = 0; i < Bindings.size(); i++)
			{
				auto& b1 = Bindings[i];
				auto& b2 = other.Bindings[i];

				if (
					b1.binding != b2.binding ||
					b1.descriptorCount != b2.descriptorCount ||
					b1.descriptorType != b2.descriptorType ||
					b1.stageFlags != b2.stageFlags ||
					b1.pImmutableSamplers != b2.pImmutableSamplers
					)
					return false;
			}
		}

		void ComputeHash()
		{
			Hash = 0;
			for (uint32 i = 0; i < Bindings.size(); i++)
			{
				auto& b = Bindings[i];
				HashCombine(Hash, b.binding);
				HashCombine(Hash, b.descriptorCount);
				HashCombine(Hash, b.descriptorType);
				HashCombine(Hash, b.stageFlags);
			}

			HashCombine(Hash, PushConstants.size);
			HashCombine(Hash, PushConstants.offset);
			HashCombine(Hash, PushConstants.stageFlags);
		}
	};
}

namespace std 
{
	template<> struct hash<Spikey::Vulkan::VulkanPSOLayoutHash>
	{
		constexpr size_t operator()(const Spikey::Vulkan::VulkanPSOLayoutHash& key) const
		{
			return key.Hash;
		}
	};
}