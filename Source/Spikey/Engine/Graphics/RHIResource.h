#pragma once

#include <Engine/Core/Common.h>
#include <Engine/Core/RefCounted.h>

namespace Spikey {

	enum class ERHIAccess : uint16
	{
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
	ENUM_FLAGS_OPERATORS(ERHIAccess);

	class IRHIResource : public IRefCounted
	{
	public:
		virtual void* GetNative() const
		{
			return nullptr;
		}
	};
}