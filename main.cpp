#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <vector>

#include "field.h"
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

struct Level
{
	int index = 0;
	SDL_Texture* levelTexture;

	Level() = default;
	Level(int index, SDL_Texture* levelTexture) : index(index), levelTexture(levelTexture) {}

	bool operator==(const Level& other) const
	{
		return index == other.index;
	}

	bool operator!=(const Level& other) const
	{
		return !(*this == other);
	}
};

using std::cerr;

SDL_AppResult initGame(const char* levelsPath);
void handleKeys(SDL_Event* ev);
void gameInputCycle(Field &field);

Field gameField;
SDL_Texture* currentLevelTexture = nullptr;
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
	Level currentLevel;
	Level winnerLevel;
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

SDL_Texture* loadImage(const std::filesystem::path& path, int squareSide)
{
	auto stringPath = path.string();
	auto cStrPath = stringPath.c_str();
	auto originalImage = IMG_Load(cStrPath);

	if (originalImage == nullptr)
	{
		SDL_Log("Couldn't load image: %s", SDL_GetError());
		throw std::runtime_error("Couldn't load image");
	}

	// Crop source image to square
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

		src.x = 0;
		src.y = topOffset;
	}

	src.w = sz;
	src.h = sz;

	auto croppedImage = SDL_CreateSurface(squareSide, squareSide, originalImage->format);

	SDL_BlitSurfaceScaled(originalImage, &src, croppedImage, nullptr, SDL_SCALEMODE_LINEAR);

	SDL_Texture* texture = SDL_CreateTextureFromSurface(app.renderer, croppedImage);
	if (texture == nullptr)
	{
		throw std::runtime_error("Couldn't create texture: " + std::string(SDL_GetError()));
	}

	SDL_DestroySurface(originalImage);
	SDL_DestroySurface(croppedImage);

	return texture;
}

int detachedI, detachedJ;

void startLevel(Level level, int n)
{
	shuffler = Shuffler(time(nullptr), 50);

	currentGameState = GameState::FadeIn;
	currentLevelTexture = level.levelTexture;

	detachedI = n - 1;
	detachedJ = n - 1;

	gameField = Field(n);
	gameField.Detach(detachedI, detachedJ);

	padding = 600 / n / 2;
}

SDL_AppResult initGame(const char* levelsPath)
{
	// load levels
	std::string path = levelsPath;

	int index = 0;

	for (const auto& entry : fs::directory_iterator(path))
	{
		if (entry.path().filename() == "winner.bmp")
		{
			app.winnerLevel = { index++, loadImage(entry.path(), squareSide) };
		}
		else
		{
			app.levels.emplace_back(index++, loadImage(entry.path(), squareSide));
		}
	}

	font = TTF_OpenFont("font/RobotoCondensed-Light.ttf", 24);

	if (font == nullptr)
	{
		cerr << "Couldn't load font";
		return SDL_APP_FAILURE;
	}

	app.currentLevel = app.levels.front();
	startLevel(app.currentLevel, FieldSide);

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

			if (app.currentLevel != app.levels.back())
			{
				app.currentLevel = app.levels[app.currentLevel.index + 1];
				startLevel(app.currentLevel, FieldSide);
			}
			else
			{
				SDL_SetWindowTitle(app.window, "Win!");
				app.isWinnersLevel = true;
				startLevel(app.winnerLevel, FieldSide);
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

			SDL_RenderTexture(app.renderer, currentLevelTexture, &srcRect, &dstRect);
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