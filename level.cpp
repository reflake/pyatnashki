#include "level.h"

#include <utility>

Level::Level(int index, Image&& levelImage, bool isVictoryScreen) : 
	index(index), 
	levelImage(std::move(levelImage)),
	isVictoryScreen(isVictoryScreen)
{}