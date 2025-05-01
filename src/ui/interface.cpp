#include "interface.h"

Interface::Interface(SDL_Renderer* renderer) : renderer(renderer)
{}

Interface::~Interface()
{
	for (auto component : components)
	{
		delete component;
	}
	components.clear();
}

void Interface::Draw()
{
	for (auto component : components)
	{
		component->Draw();
	}
}