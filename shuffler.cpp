#include "shuffler.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

Shuffler::Shuffler(int seed, int difficulty) : stepsLeft(difficulty)
{
	std::srand(unsigned(seed));
	moveTo = { 0, 0 };
}

void Shuffler::next(Field& field, int& detachedI, int& detachedJ)
{
	std::pair<int, int> freeSpaces[4];
	int ways = 0;
	int n = field.GetSize();

	ways = 0;

	if (detachedI > 0 && moveTo != std::pair<int, int> { 0, 1 })
		freeSpaces[ways++] = { 0, -1 };

	if (detachedI < n - 1 && moveTo != std::pair<int, int> { 0, -1 })
		freeSpaces[ways++] = { 0, 1 };

	if (detachedJ > 0 && moveTo != std::pair<int, int> { 1, 0 })
		freeSpaces[ways++] = { -1, 0 };

	if (detachedJ < n - 1 && moveTo != std::pair<int, int> { -1, 0 })
		freeSpaces[ways++] = { 1, 0 };

	moveTo = freeSpaces[std::rand() % ways];

	field.Turn(detachedI, detachedJ, moveTo.first, moveTo.second);

	--stepsLeft;
}

bool Shuffler::isDone(Field& field)
{
	return stepsLeft <= 0 && !field.IsAssembled();
}