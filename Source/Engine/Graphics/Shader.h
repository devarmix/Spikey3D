#pragma once

#include <Graphics/Texture.h>
#include <Graphics/Buffer.h>

namespace Spikey {

	enum class ShaderStage : uint8 
	{
		Vertex,
		Pixel,
		Compute
	};

	class GPUShader : public RefCounted 
	{
	protected:
		GPUShader()
		{
		}

	public:
		virtual ShaderStage GetStage() const = 0;
		virtual void* GetNative() const = 0;
	};

	enum class FrontFace : uint8 
	{
		ClockWise,
		CounterClockWise
	};

	enum class ComparisonFunc : uint8 
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	enum class StencilOp : uint8 
	{
		Keep,
		Zero,
		Replace,
		IncrementAndClamp,
		DecrementAndClamp,
		Invert,
		IncrementAndWrap,
		DecrementAndWrap
	};

	enum class PrimitiveTopology : uint8 
	{
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

	enum class CullMode : uint8 
	{
		None,
		FrontFace,
		BackFace
	};

	enum class BlendOperation : uint8 
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	enum class BlendFactor : uint8 
	{
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

	enum class ColorMask : uint8 
	{
		None = 0,

		R = BIT(0),
		G = BIT(1),
		B = BIT(2),
		A = BIT(3),

		All = R | G | B | A
	};
	ENUM_FLAGS_OPERATORS(ColorMask)

	struct PipelineStateDesc 
	{
		GPUShader* VertexShader = nullptr;
		GPUShader* PixelShader = nullptr;
		GPUShader* ComputeShader = nullptr;

		bool DepthEnable;
		bool DepthWriteEnable;
		bool DepthClipEnable;
		ComparisonFunc DepthFunc;
		PixelFormat DepthFormat;

		bool StencilEnable;
		struct StencilState 
		{
			uint8 ReadMask;
			uint8 WriteMask;
			ComparisonFunc Func;
			StencilOp FailOp;
			StencilOp DepthFailOp;
			StencilOp PassOp;
		};

		StencilState FrontStencil;
		StencilState BackStencil;

		PrimitiveTopology PrimitiveTopology;
		CullMode CullMode;
		FrontFace FrontFace;

		bool Wireframe;
		uint8 NumRenderTargets;

		struct RenderTarget 
		{
			PixelFormat Format;

			bool EnableBlend = false;
			BlendFactor SrcBlend = BlendFactor::One;
			BlendFactor DstBlend = BlendFactor::Zero;
			BlendOperation BlendOp = BlendOperation::Add;
			BlendFactor SrcBlendAlpha = BlendFactor::One;
			BlendFactor DstBlendAlpha = BlendFactor::Zero;
			BlendOperation BlendOpAlpha = BlendOperation::Add;
			ColorMask ColorMask = ColorMask::All;
		} RenderTargets[8];
	};

	class GPUPipelineState : public RefCounted 
	{
	protected:
		GPUPipelineState() = default;
	};
}