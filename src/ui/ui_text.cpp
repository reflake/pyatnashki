#include "ui_text.h"
#include <SDL3/SDL_render.h>

Text::Text(const char* text, SDL_Color color, TTF_Font* font) :
	text(text), color(color), font(font)
{}

Text::~Text()
{
	FreeTexture();
}

void Text::OnMount()
{
	if (renderer == nullptr || font == nullptr)
		return;

	SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, text, 0, color, rectangle.w);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);
}

void Text::Draw()
{
	if (renderer == nullptr || font == nullptr || texture == nullptr)
		return;

	SDL_FRect srcRect = SDL_FRect{ 0, 0, rectangle.w, rectangle.h };
	SDL_FRect dstRect = rectangle;

	float w, h;
	SDL_GetTextureSize(texture, &w, &h);

	if (w < rectangle.w)
	{
		srcRect.w = w;
		dstRect.w = w;
	}
	
	if (h < rectangle.h)
	{
		srcRect.h = h;
		dstRect.h = h;
	}

	SDL_RenderTexture(renderer, texture, &srcRect, &dstRect);
}

void Text::FreeTexture()
{
	if (texture != nullptr)
	{
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
}