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

		bool DepthEnable;
		bool DepthWriteEnable;
		bool DepthClipEnable;
		EComparisonFunc DepthFunc;
		ETextureFormat DepthFormat;

		bool  StencilEnable;
		struct StencilState {
			uint8 ReadMask;
			uint8 WriteMask;
			EComparisonFunc Func;
			EStencilOp FailOp;
			EStencilOp DepthFailOp;
			EStencilOp PassOp;
		};

		StencilState FrontStencil;
		StencilState BackStencil;

		EPrimitiveTopology PrimitiveTopology;
		ECullMode CullMode;
		EFrontFace FrontFace;

		bool Wireframe;
		uint8 NumRenderTargets;

		struct RenderTarget {
			ETextureFormat Format;

			bool EnableBlend = false;
			EBlendFactor SrcBlend = EBlendFactor::One;
			EBlendFactor DstBlend = EBlendFactor::Zero;
			EBlendOp BlendOp = EBlendOp::Add;
			EBlendFactor SrcBlendAlpha = EBlendFactor::One;
			EBlendFactor DstBlendAlpha = EBlendFactor::Zero;
			EBlendOp BlendOpAlpha = EBlendOp::Add;
			EColorMask ColorMask = EColorMask::All;
		} RenderTargets[8];
	};

	class IRHIPipelineState : public IRefCounted {
	public:
		IRHIPipelineState() = default;
	};

	using PipelineStateRHIRef = TRef<IRHIPipelineState>;
}