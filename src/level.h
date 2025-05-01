#pragma once

#include "image.h"

class Level
{
public:

	Level() = default;
	Level(int index, Image&& levelImage, bool isVictoryScreen);

	inline int GetIndex() const
	{
		return index;
	}

	inline Image& GetLevelImage()
	{
		return levelImage;
	}

	inline bool IsVictoryScreen() const
	{
		return isVictoryScreen;
	}

	inline bool operator==(const Level& other) const
	{
		return index == other.index;
	}

	inline bool operator!=(const Level& other) const
	{
		return !(*this == other);
	}

private:

	int index = 0;
	Image levelImage;
	bool isVictoryScreen = false;
};