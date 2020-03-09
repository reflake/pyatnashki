#pragma once

#include <iostream>
#include "field.h"

class Shuffler
{
public:

	Shuffler(int seed, int difficulty);
	bool isDone(Field& field);
	void next(Field& field, int& i, int& j);

private:

	int stepsLeft;
	std::pair<int, int> moveTo;
};