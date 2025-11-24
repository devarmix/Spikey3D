#pragma once

#include <Engine/Graphics/Texture2D.h>
#include <Engine/Graphics/Buffer.h>
#include <Engine/Graphics/TextureCube.h>

namespace Spikey {

	enum class EShaderType : uint8 {
		None = 0,

		Vertex,
		Pixel,
		Compute
	};

	enum class EShaderResourceType : uint8 {
		None = 0,

		TextureSRV,
		TextureUAV,
		BufferSRV,
		BufferUAV,
		ConstantBuffer,

		Sampler
	};

	constexpr char SHADER_MAGIC[4] = { 'S', 'C', 'S', 'F' };
	constexpr char MATERIAL_SHADER_MAGIC[4] = { 'S', 'M', 'S', 'F' };

	/*
	struct ShaderSource {
		std::vector<uint8> VertexCode;
		std::vector<uint8> PixelCode;
		std::vector<uint8> ComputeCode;
		std::vector<ShaderBinding> Bindings;

		uint16 SetCount;
		uint8 PushDataSize;

		friend BinaryReadStream& operator>>(BinaryReadStream& stream, ShaderSource& self) {
			stream >> self.VertexCode >> self.PixelCode >> self.ComputeCode;
			stream >> self.Bindings >> self.SetCount >> self.PushDataSize;

			return stream;
		}

		friend BinaryWriteStream& operator<<(BinaryWriteStream& stream, const ShaderSource& self) {
			stream << self.VertexCode << self.PixelCode << self.ComputeCode;
			stream << self.Bindings << self.SetCount << self.PushDataSize;

			return stream;
		}
	};

	struct MaterialShaderSource {
		std::vector<uint8> VertexCode;
		std::vector<uint8> PixelCode;

		struct {
			std::vector<std::string> Scalar;
			std::vector<std::string> Uint;
			std::vector<std::string> Vec2;
			std::vector<std::string> Vec4;
			std::vector<std::string> Texture;
		} Parameters;

		friend BinaryReadStream& operator>>(BinaryReadStream& stream, MaterialShaderSource& self) {
			stream >> self.VertexCode >> self.PixelCode;
			stream >> self.Parameters.Scalar >> self.Parameters.Uint >> self.Parameters.Vec2
				>> self.Parameters.Vec4 >> self.Parameters.Texture;

			return stream;
		}

		friend BinaryWriteStream& operator<<(BinaryWriteStream& stream, const MaterialShaderSource& self) {
			stream << self.VertexCode << self.PixelCode;
			stream << self.Parameters.Scalar << self.Parameters.Uint << self.Parameters.Vec2
				<< self.Parameters.Vec4 << self.Parameters.Texture;

			return stream;
		}

		class MaterialShader : public IAsset {
	public:
		MaterialShader(MaterialShaderSource&& source, UUID id);
		virtual ~MaterialShader() override;

		static TRef<MaterialShader> Create(BinaryReadStream& stream, UUID id);
		const MaterialShaderSource& GetSource() const { return m_Source; }

	private:
		MaterialShaderSource m_Source;
		std::vector<RHIPipelineState*> m_States;
	};
	}; */

	/* struct PushData {
	    uint32 Input;
		uint32 Output;
		float Value;
		float Padding[3];
		Vec4 OtherValue;
	} */

	class IRHIShader : public IRefCounted {
	public:
		IRHIShader() = default;
		virtual void* GetNative() const = 0;
	};

	using ShaderRHIRef = TRef<IRHIShader>;

	enum class EFrontFace : uint8 {
		None = 0,

		ClockWise,
		CounterClockWise
	};

	enum class EComparisonFunc : uint8 {
		Never = 0,

		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	enum class EStencilOp : uint8 {
		None = 0,

		Keep,
		Zero,
		Replace,
		IncrementSaturated,
		DecrementSaturated,
		Invert,
		Increment,
		Decrement
	};

	enum class EPrimitiveTopology : uint8 {
		None = 0,

		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		TriangleListWithAdjacency,
		TriangleStripWithAdjacency,
		PatchList
	};

	enum class ECullMode : uint8 {
		None = 0,

		FrontFace,
		BackFace
	};

	enum class EBlendOp : uint8 {
		None = 0,

		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class EBlendFactor : uint8 {
		None = 0,

		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		DstColor,
		OneMinusDstColor,
		SrcAlphaSaturate,
		ConstantColor,
		OneMinusConstantColor,
		Src1Color,
		OneMinusSrc1Color,
		Src1Alpha,
		OneMinusSrc1Alpha
	};

	enum class EColorMask : uint8 {
		None = 0,

		R = BIT(0),
		G = BIT(1),
		B = BIT(2),
		A = BIT(3),

		All = R | G | B | A
	};
	ENUM_FLAGS_OPERATORS(EColorMask)

	struct PipelineStateDesc {

		IRHIShader* VertexShader = nullptr;
		IRHIShader* PixelShader = nullptr;
		IRHIShader* ComputeShader = nullptr;

		uint8 PushDataSize;

		bool DepthEnable;
		bool DepthWriteEnable;
		bool DepthClipEnable;
		EComparisonFunc DepthFunc;

		bool StencilEnable;
		uint8 StencilReadMask;
		uint8 StencilWriteMask;
		EComparisonFunc StencilFunc;
		EStencilOp StencilFailOp;
		EStencilOp StencilDepthFailop;
		EStencilOp StencilPassOp;

		EPrimitiveTopology PrimitiveTopology;
		ECullMode CullMode;
		EFrontFace FrontFace;

		struct RenderTarget {
			ETextureFormat Format;

			bool EnableBlend;
			EBlendFactor SrcBlend;
			EBlendFactor DstBlend;
			EBlendOp BlendOp;
			EBlendFactor SrcBlendAlpha;
			EBlendFactor DstBlendAlpha;
			EBlendOp BlendOpAplha;
			EColorMask ColorMask;
		} RenderTargets[8];
	};

	class IRHIPipelineState : public IRefCounted {
	public:
		IRHIPipelineState() = default;
	};

	using PipelineStateRHIRef = TRef<IRHIPipelineState>;

	class ShaderManager {
	public:
		static uint32 GetMaterialID();
		static uint32 GetShaderTextureID(IRHITextureView* view);
		static uint32 GetShaderSamplerID(IRHISampler* sampler);

		//static IRHIShader* LoadDefaultShader(EShader shader);

		static void FreeShaderTextureID(uint32 id);
		static void FreeShaderSamplerID(uint32 id);
		static void FreeMaterialID(uint32 id);
		static void UpdateMaterial(uint32 id);
		//void UpdateTexture(uint32 id, RHITextureView* view);

		struct alignas(16) MaterialData {

			float ScalarData[16];
			uint32 UintData[16];
			Vec2 Float2Data[16];
			Vec4 Float4Data[16];
			uint32 TextureData[16];
			uint32 SamplerData[16];
		};

		static MaterialData& GetMaterialData(uint32 id);
	};
}