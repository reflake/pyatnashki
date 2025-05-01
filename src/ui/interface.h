#pragma once

#include <SDL3/SDL_render.h>
#include <vector>

#include "component.h"

class Interface
{
public:

	Interface() = default;
	Interface(SDL_Renderer* renderer);
	~Interface();

	void Draw();

	template<typename T, typename... Args>
	T* CreateComponent(Args&&... args)
	{
		T* component = new T(std::forward<Args>(args)...);
		component->SetRenderer(renderer);
		component->OnMount();
		components.push_back(component);

		return component;
	}

private:

	SDL_Renderer* renderer = nullptr;
	std::vector<Component*> components;
};