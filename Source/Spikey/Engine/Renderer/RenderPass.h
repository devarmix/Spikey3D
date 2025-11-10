#pragma once

namespace Spikey {

	class IRenderPass {
	public:
		IRenderPass() { m_IsValid = false; }
		virtual ~IRenderPass() = default;

		virtual bool Init() = 0;
		virtual void Destroy() = 0;

	protected:
		bool m_IsValid;
	};

	// singleton pattern for render passes
	// based of https://github.com/FlaxEngine/FlaxEngine/blob/master/Source/Engine/Renderer/RendererPass.h
	template<typename T>
	class TRenderPass : public IRenderPass {
	public:
		static T* Instance() {
			static T instance{};
			return &instance;
		}

	protected:
		TRenderPass() {}
		~TRenderPass() {}

	private:
		TRenderPass(const TRenderPass& other) = delete;
		TRenderPass& operator=(const TRenderPass& other) = delete;
	};
}