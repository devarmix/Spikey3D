#pragma once 
#include <Engine/Core/Common.h>

namespace Spikey {

	enum class EGPUAccess : uint8 {
		None = 0,

		IndirectArgs = BIT(0),
		SRVCompute = BIT(1),
		SRVGraphics = BIT(2),
		UAVCompute = BIT(3),
		UAVGraphics = BIT(4),
		CopySrc = BIT(5),
		CopyDst = BIT(6),
		ColorTarget = BIT(7),
		DepthTarget = BIT(8),

		SRV = SRVCompute | SRVGraphics,
		UAV = UAVCompute | UAVGraphics
	};
	ENUM_FLAGS_OPERATORS(EGPUAccess);

	using RHIData = uint64;

	class IRHIResource {
	public:
		virtual ~IRHIResource() = default;

		virtual void InitRHI() = 0;
		virtual void ReleaseRHI() = 0;
		virtual void ReleaseRHIImmediate() { ReleaseRHI(); }
	};

	void SafeResourceInit(IRHIResource* resource);
	void SafeResourceRelease(IRHIResource* resource);
}