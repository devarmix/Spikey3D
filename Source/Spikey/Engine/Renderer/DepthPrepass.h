#pragma once

#include <Engine/Renderer/RenderPass.h>

namespace Spikey {

	struct RenderContext;

	class DepthPrepass : public TRenderPass<DepthPrepass> {
	public:
		DepthPrepass();
		virtual ~DepthPrepass() override;

		void Render(RenderContext& context);
		virtual bool Init() override;
		virtual void Destroy() override;

	private:
	};
}