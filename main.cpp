#include <SDL_surface.h>
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <vector>

#include "field.h"
#include "shuffler.h"

//Screen dimension constants
const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;

namespace fs = std::filesystem;

enum class GameState
{
	FadeIn,
	FadeOut,
	Shuffling,
	InProgress,
	End
};

struct Level
{
	SDL_Surface* levelSurface;
};

using std::cin;
using std::cout;
using std::endl;
using std::cerr;
using std::min;
using std::max;

void nativeGameCycle(const char* levelsPath, SDL_Window* window);
void handleKeys(SDL_Event& ev, int& h, int& v);
void gameInputCycle(Field &field, int n, int h, int v);

Field gameField;
SDL_Window* window = nullptr;
SDL_Surface* currentLevelSurface = nullptr;
Shuffler shuffler;
GameState currentGameState;
std::vector<SDL_Surface*> loadedSurfaces;
int padding = 1;

int main(int argc, char *args[])
{
	if (argc < 2)
		return 1;

	SDL_Window* window = nullptr;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cerr << "Couldn't initialize SDL: " << SDL_GetError();
	}

	window = SDL_CreateWindow("Epifashki shashki", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (window == nullptr)
	{
		cerr << "Couldn't create SDL window: " << SDL_GetError();
	}

	auto imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;

	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		cerr << "Couldn't initialize SDL_image: " << IMG_GetError();
	}

	nativeGameCycle(args[1], window);

	for (auto surface : loadedSurfaces)
	{
		SDL_FreeSurface(surface);
	}
	loadedSurfaces.clear();

	SDL_DestroyWindow(window);

	IMG_Quit();
	SDL_Quit();

	return 0;
}

SDL_Surface* loadImage(const std::filesystem::path& path, int squareSide)
{
	auto stringPath = path.string();
	auto cStrPath = stringPath.c_str();
	SDL_Surface* originalImage = IMG_Load(cStrPath);
	loadedSurfaces.push_back(originalImage);

	if (originalImage == nullptr)
	{
		throw std::runtime_error("Couldn't load image: " + std::string(SDL_GetError()));
	}

	SDL_Surface* newImage = SDL_CreateRGBSurface(
		originalImage->flags,
		squareSide,
		squareSide,
		originalImage->format->BitsPerPixel,
		originalImage->format->Rmask,
		originalImage->format->Gmask,
		originalImage->format->Bmask,
		originalImage->format->Amask
	);
	
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

		src.x = topOffset;
		src.y = 0;
	}

	src.w = sz;
	src.h = sz;

	SDL_Rect dstRect = SDL_Rect{
		0,
		0,
		squareSide,
		squareSide
	};
	
	SDL_BlitScaled(originalImage, &src, newImage, &dstRect);

	return newImage;
}

int detachedI, detachedJ;

void startLevel(Level level, int n)
{
	shuffler = Shuffler(time(nullptr), 50);

	currentGameState = GameState::FadeIn;
	currentLevelSurface = level.levelSurface;

	detachedI = n - 1;
	detachedJ = n - 1;

	gameField = Field(n);
	gameField.Detach(detachedI, detachedJ);

	padding = 600 / n / 2;
}

void nativeGameCycle(const char* levelsPath, SDL_Window* window)
{
	SDL_Surface* screenSurface = nullptr;
	screenSurface = SDL_GetWindowSurface(window);
	bool quit = false;
	SDL_Event e;

	const int squareSide = 600;

	int keyHorizontal = 0;
	int keyVertical = 0;

	// load levels
	std::string path = levelsPath;
	std::vector<Level> levels;
	Level winnerLevel;

	for (const auto& entry : fs::directory_iterator(path))
	{
		if (entry.path().filename() == "winner.bmp")
		{
			winnerLevel = { loadImage(entry.path(), squareSide) };
		}
		else
		{
			levels.insert(levels.end(), Level{ loadImage(entry.path(), squareSide) });
		}
	}

	int n = 4;
	auto currentLevel = levels.begin();
	startLevel(*currentLevel, n);
	bool isWinnersLevel = false;
	
	int paddingSpeed = squareSide / n / 50;

	do {

		if (currentGameState == GameState::FadeIn)
		{
			padding = padding % (squareSide / n) - paddingSpeed;

			if (padding <= 1)
			{
				currentGameState = isWinnersLevel ? GameState::InProgress : GameState::Shuffling;
				padding = 1;
			}

			SDL_Delay(8);
		}
		else if (currentGameState == GameState::FadeOut)
		{
			padding = padding % (squareSide / n) + paddingSpeed;

			if (padding >= squareSide / n / 2)
			{
				currentGameState = GameState::End;

				if (currentLevel != levels.end())
				{
					startLevel(*currentLevel, n);
				}
				else
				{
					SDL_SetWindowTitle(window, "Win!");
					isWinnersLevel = true;
					startLevel(winnerLevel, n);
				}
			}

			SDL_Delay(8);
		}

		keyHorizontal = 0;
		keyVertical = 0;

		while (SDL_PollEvent(&e) != 0)
		{
			switch(e.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					handleKeys(e, keyHorizontal, keyVertical);
					break;
			}
		}

		if (keyHorizontal != 0)
			keyVertical = 0;

		//Clear
		SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				if (gameField.GetState()[i * n + j] == Field::empty_cell)
					continue;

				SDL_Rect dstRect = SDL_Rect{
					j * squareSide / n + padding,
					i * squareSide / n + padding,
					squareSide / n - padding * 2,
					squareSide / n - padding * 2
				};
				
				int index = gameField.GetState()[i * n + j];
				int sI, sJ;

				sI = index / n;
				sJ = index % n;

				SDL_Rect srcRect = SDL_Rect{
					sJ * squareSide / n + padding,
					sI * squareSide / n + padding,
					squareSide / n - padding * 2,
					squareSide / n - padding * 2
				};

				SDL_BlitSurface(currentLevelSurface, &srcRect, screenSurface, &dstRect);
			}
		}

		switch (currentGameState)
		{
			case GameState::Shuffling:
				shuffler.next(gameField, detachedI, detachedJ);

				if (shuffler.isDone(gameField))
					currentGameState = GameState::InProgress;

				break;
			case GameState::InProgress:
				gameInputCycle(gameField, n, keyHorizontal, keyVertical);

				if (gameField.IsAssembled() && !isWinnersLevel)
				{
					++currentLevel;
					currentGameState = GameState::FadeOut;
				}
				break;
		}

		SDL_UpdateWindowSurface(window);

		SDL_Delay(10);

	} while (!quit);
}

void handleKeys(SDL_Event &ev, int &h, int &v)
{
	switch (ev.key.keysym.sym)
	{
		case SDLK_UP:
			--v;
			break;
		case SDLK_DOWN:
			++v;
			break;
		case SDLK_LEFT:
			--h;
			break;
		case SDLK_RIGHT:
			++h;
			break;
	}
}

void gameInputCycle(Field &field, int n, int h, int v)
{
	if (h == 0 && v == 0)
		return;

	if (h != 0)
		v = 0;

	field.Turn(detachedI, detachedJ, h, v);
}