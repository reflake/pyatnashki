#include "image.h"

#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <stdexcept>

Image::Image(const char* filePath)
{
	image = IMG_Load(filePath);

	if (image == nullptr)
	{
		SDL_Log("Couldn't load image: %s", SDL_GetError());
		throw std::runtime_error("Couldn't load image");
	}
}

Image::Image(const Image& other) :
	width(other.width), height(other.height),
	image(other.image)
{
	if (image != nullptr)
	{
		SDL_Surface* originalImage = static_cast<SDL_Surface*>(image);
		SDL_Surface* newImage = SDL_CreateSurface(originalImage->w, originalImage->h, originalImage->format);

		SDL_BlitSurface(originalImage, nullptr, newImage, nullptr);

		image = newImage;
	}
}

Image::~Image()
{
	FreeImage();
}

void Image::FitIntoSquare()
{
	auto originalImage = static_cast<SDL_Surface*>(this->image);
	if (originalImage == nullptr)
	{
		throw std::runtime_error("Original image is null");
	}

	// Crop source image to square
	SDL_Rect src;
	int sz;

	if (originalImage->w > originalImage->h)
	{
		sz = originalImage->h;

		int leftOffset = (originalImage->w - sz) / 2;

		src.x = leftOffset;
		src.y = 0;
	}
	else
	{
		sz = originalImage->w;

		int topOffset = (originalImage->h - sz) / 2;

		src.x = 0;
		src.y = topOffset;
	}

	src.w = sz;
	src.h = sz;

	auto croppedImage = SDL_CreateSurface(sz, sz, originalImage->format);

	SDL_BlitSurface(originalImage, &src, croppedImage, nullptr);

	FreeImage();
	image = croppedImage;
}

void Image::Resize(unsigned int newWidth, unsigned int newHeight)
{
	auto originalImage = static_cast<SDL_Surface*>(this->image);
	if (originalImage == nullptr)
	{
		throw std::runtime_error("Original image is null");
	}

	auto resizedImage = SDL_CreateSurface(newWidth, newHeight, originalImage->format);

	SDL_BlitSurfaceScaled(originalImage, nullptr, resizedImage, nullptr, SDL_SCALEMODE_LINEAR);

	FreeImage();
	image = resizedImage;
}

void Image::Draw(void* renderer, SDL_FRect* dstRect, SDL_FRect* srcRect)
{
	auto sdlRenderer = static_cast<SDL_Renderer*>(renderer);
	if (sdlRenderer == nullptr)
	{
		throw std::runtime_error("Renderer is null");
	}

	auto imageSurface = static_cast<SDL_Surface*>(this->image);
	if (imageSurface == nullptr)
	{
		return;
	}

	if (texture == nullptr)
	{
		texture = SDL_CreateTextureFromSurface(sdlRenderer, imageSurface);
		if (texture == nullptr)
		{
			throw std::runtime_error("Couldn't create texture: " + std::string(SDL_GetError()));
		}
	}

	SDL_RenderTexture(sdlRenderer, static_cast<SDL_Texture*>(texture), srcRect, dstRect);
}

void Image::FreeImage()
{
	if (image != nullptr)
	{
		SDL_DestroySurface(static_cast<SDL_Surface*>(image));
		image = nullptr;
	}

	if (texture != nullptr)
	{
		SDL_DestroyTexture(static_cast<SDL_Texture*>(texture));
		texture = nullptr;
	}
}