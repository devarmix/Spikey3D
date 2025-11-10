#include <Engine/Graphics/Resource.h>
#include <Engine/Core/Application.h>

void Spikey::SafeResourceInit(IRHIResource* resource) {
	if (resource) {
		ENQUEUE_RENDER_COMMAND([resource]() {
			resource->InitRHI();
			});
	}
}

void Spikey::SafeResourceRelease(IRHIResource* resource) {
	if (resource) {
		ENQUEUE_RENDER_COMMAND([resource]() {

			resource->ReleaseRHI();
			delete resource;
			});
}