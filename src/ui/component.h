#pragma once

#include <SDL3/SDL_render.h>

class Component
{
public:

	virtual ~Component() = default;

	virtual void Draw() = 0;
	virtual void OnMount() = 0;

	void SetRenderer(SDL_Renderer* renderer)
	{
		this->renderer = renderer;
	}

	void SetRectangle(SDL_FRect rectangle)
	{
		this->rectangle = rectangle;
	}

protected:

	SDL_Renderer* renderer = nullptr;
	SDL_FRect rectangle = SDL_FRect{ 0, 0, 0, 0 };
};