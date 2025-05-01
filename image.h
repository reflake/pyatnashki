#pragma once

#include <SDL3/SDL_rect.h>

class Image
{
public:

	Image() = default;
	Image(const char* filename);
	Image(const Image& other);
	Image(Image&& other);
	~Image();

	void FitIntoSquare();
	void Resize(unsigned int newWidth, unsigned int newHeight);
	void Draw(void* renderer, SDL_FRect* dstRect, SDL_FRect* srcRect = nullptr);

private:

	void FreeImage();

	void* image = nullptr;
	void* texture = nullptr;
};

