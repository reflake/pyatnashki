#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <vector>

#include "field.h"
#include "image.h"
#include "level.h"
#include "shuffler.h"

#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3/SDL_timer.h>

//Screen dimension constants
const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 720;

namespace fs = std::filesystem;

enum class GameState
{
	FadeIn,
	FadeOut,
	Shuffling,
	InProgress,
	End
};

using std::cerr;

SDL_AppResult initGame(const char* levelsPath);
void handleKeys(SDL_Event* ev);
void gameInputCycle(Field &field);

Field gameField;
SDL_Texture* helpTexture = nullptr;
Shuffler shuffler;
GameState currentGameState;
TTF_Font* font = nullptr;
int padding = 1;
constexpr int FieldSide = 3;
constexpr int squareSide = 600;

struct App
{
	bool isWinnersLevel = false;
	int keyHorizontal = 0;
	int keyVertical = 0;
	std::vector<Level> levels;
	Level* currentLevel;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
};

App app;

SDL_AppResult SDL_AppInit(void **_, int argc, char *args[])
{
	if (argc < 2)
		return SDL_APP_FAILURE;

	if (!SDL_CreateWindowAndRenderer("Epifashki shashki", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &app.window, &app.renderer))
	{
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if (!TTF_Init())
	{
		cerr << "Couldn't initialize SDL_ttf: " << SDL_GetError();
		return SDL_APP_FAILURE;
	}

	return initGame(args[1]);
}

int detachedI, detachedJ;

void startLevel(int levelIndex, int n)
{
	shuffler = Shuffler(time(nullptr), 1);

	app.currentLevel = &*std::find_if(app.levels.begin(), app.levels.end(), [levelIndex](const Level& level) {
		return level.GetIndex() == levelIndex;
	});

	currentGameState = GameState::FadeIn;

	detachedI = n - 1;
	detachedJ = n - 1;

	gameField = Field(n);
	gameField.Detach(detachedI, detachedJ);

	padding = 600 / n / 2;
}

Image LoadImage(std::string& filename)
{
	Image image(filename.c_str());
	image.FitIntoSquare();
	image.Resize(squareSide, squareSide);
	return image;
}

const int victoryScreenIndex = -1;

SDL_AppResult initGame(const char* levelsPath)
{
	// load levels
	std::string path = levelsPath;

	int index = 0;

	app.levels.reserve(20);

	for (const auto& entry : fs::directory_iterator(path))
	{
		auto filePath = entry.path().string();
		bool isVictoryScreen = entry.path().stem() == "winner";

		app.levels.emplace_back(isVictoryScreen ? victoryScreenIndex : index++, LoadImage(filePath), isVictoryScreen);
	}

	font = TTF_OpenFont("font/RobotoCondensed-Light.ttf", 24);

	if (font == nullptr)
	{
		cerr << "Couldn't load font";
		return SDL_APP_FAILURE;
	}

	startLevel(0, FieldSide);

	// Create text texture of instructions for the player
	const char* text = "Place each piece in its place\nUse arrow keys to move the pieces";

	SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(font, text, 0, { 34, 34, 34, 255 }, 400);
	helpTexture = SDL_CreateTextureFromSurface(app.renderer, textSurface);

	SDL_DestroySurface(textSurface);
	
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *_)
{
	int paddingSpeed = squareSide / FieldSide / 50;

	if (currentGameState == GameState::FadeIn)
	{
		padding = padding % (squareSide / FieldSide) - paddingSpeed;

		if (padding <= 1)
		{
			currentGameState = app.isWinnersLevel ? GameState::InProgress : GameState::Shuffling;
			padding = 1;
		}
	}
	else if (currentGameState == GameState::FadeOut)
	{
		padding = padding % (squareSide / FieldSide) + paddingSpeed;

		if (padding >= squareSide / FieldSide / 2)
		{
			currentGameState = GameState::End;

			int levelCounts = std::count_if(app.levels.begin(), app.levels.end(), [](const Level& level) {
				return !level.IsVictoryScreen();
			});
			bool currentLevelIsLast = app.currentLevel->GetIndex() == levelCounts - 1;
			app.isWinnersLevel = currentLevelIsLast;

			if (currentLevelIsLast)
			{
				SDL_SetWindowTitle(app.window, "Win!");
				startLevel(victoryScreenIndex, FieldSide);
			}
			else
			{
				startLevel(app.currentLevel->GetIndex() + 1, FieldSide);
			}
		}
	}

	// Clear screen
	SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
	SDL_RenderClear(app.renderer);

	for (int i = 0; i < FieldSide; i++)
	{
		for (int j = 0; j < FieldSide; j++)
		{
			if (gameField.GetState()[i * FieldSide + j] == Field::empty_cell)
				continue;

			SDL_FRect dstRect = SDL_FRect{
				1.0f * j * squareSide / FieldSide + padding,
				1.0f * i * squareSide / FieldSide + padding,
				1.0f * squareSide / FieldSide - padding * 2,
				1.0f * squareSide / FieldSide - padding * 2
			};
			
			int index = gameField.GetState()[i * FieldSide + j];
			int sI, sJ;

			sI = index / FieldSide;
			sJ = index % FieldSide;

			SDL_FRect srcRect = SDL_FRect{
				1.0f * sJ * squareSide / FieldSide + padding,
				1.0f * sI * squareSide / FieldSide + padding,
				1.0f * squareSide / FieldSide - padding * 2,
				1.0f * squareSide / FieldSide - padding * 2
			};

			Image& currentLevelImage = app.currentLevel->GetLevelImage();
			currentLevelImage.Draw(app.renderer, &dstRect, &srcRect);
		}
	}

	{
		SDL_FRect dstRect = SDL_FRect{ 24, squareSide + 24, 0, 0 };
		SDL_GetTextureSize(helpTexture, &dstRect.w, &dstRect.h);

		SDL_RenderTexture(app.renderer, helpTexture, nullptr, &dstRect);
	}

	switch (currentGameState)
	{
		case GameState::Shuffling:
			shuffler.next(gameField, detachedI, detachedJ);

			if (shuffler.isDone(gameField))
				currentGameState = GameState::InProgress;

			break;
		case GameState::InProgress:
			gameInputCycle(gameField);

			if (gameField.IsAssembled() && !app.isWinnersLevel)
			{
				currentGameState = GameState::FadeOut;
			}
			break;
		default: break;
	}

	SDL_RenderPresent(app.renderer);
	SDL_Delay(16);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *_, SDL_Event *event)
{
	switch (event->type) {
		case SDL_EVENT_KEY_DOWN:
			handleKeys(event);
			break;
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
		default: break;
	}

	return SDL_APP_CONTINUE;
}

void handleKeys(SDL_Event *ev)
{
	switch (ev->key.key)
	{
		case SDLK_UP:
			++app.keyVertical;
			break;
		case SDLK_DOWN:
			--app.keyVertical;
			break;
		case SDLK_LEFT:
			++app.keyHorizontal;
			break;
		case SDLK_RIGHT:
			--app.keyHorizontal;
			break;
	}
}

void gameInputCycle(Field &field)
{
	if (app.keyHorizontal == 0 && app.keyVertical == 0)
		return;

	if (app.keyHorizontal != 0)
		app.keyVertical = 0;

	field.Turn(detachedI, detachedJ, app.keyHorizontal, app.keyVertical);

	app.keyHorizontal = 0;
	app.keyVertical = 0;
}

void SDL_AppQuit(void *_, SDL_AppResult __)
{
	if (font != nullptr)
	{
		TTF_CloseFont(font);
		font = nullptr;
	}

	TTF_Quit();
}