#pragma once

#include "component.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

class Text : public Component
{
public:

	Text(const char* text, SDL_Color color, TTF_Font* font);
	~Text();

	void OnMount() override;
	void Draw() override;

private:

	void FreeTexture();

private:

	const char* text = "";
	SDL_Color color = { 0, 0, 0, 255 };
	SDL_Texture* texture = nullptr;
	TTF_Font* font = nullptr;
};