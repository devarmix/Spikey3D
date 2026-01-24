#pragma once

#include <Engine/Core/RefCounted.h>

namespace Spikey
{
	class GPUBuffer;
	class GPUTexture;
	class GPUTextureView;
	class GPUSamplerState;
	class GPUShader;
	class GPUPipelineState;
	class GPUDevice;
	class GPUCommandList;

	using GPUBufferRef = TRefCountPtr<GPUBuffer>;
	using GPUTextureRef = TRefCountPtr<GPUTexture>;
	using GPUSamplerStateRef = TRefCountPtr<GPUSamplerState>;
	using GPUShaderRef = TRefCountPtr<GPUShader>;
	using GPUPipelineStateRef = TRefCountPtr<GPUPipelineState>;
	using GPUCommandListRef = TRefCountPtr<GPUCommandList>;
}