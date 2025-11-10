#include <Engine/Core/Window.h>
#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/Graphics/FrameRenderer.h>

using namespace Spikey;

int main() {

	Texture2DDesc desc{};
	RHITexture2D* tex = TransientPool::FindTexture(desc);
	TransientPool::ReleaseTexture(tex);

	TextureViewDesc desc2{};
	RHITextureView* view = TransientPool::FindTextureView(desc2);
	TransientPool::ReleaseTextureView(view);

	return 0;
}