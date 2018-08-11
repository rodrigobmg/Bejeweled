#include <iostream>
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <ctime>

// Ported by @valt to SDL from https://www.youtube.com/user/FamTrinli Bejeweled tutorial in SFML

enum { BACKGROUND = 0, GEMS, CURSOR };
const int sizeItem = 54;
const int ItemsX = 8, ItemsY = 8;
const int sizeX = 740, sizeY = 480;
const int FPS = 60;
const int ANIMATION_DELAY = 1000;
SDL_Rect boardRect = { 0, 0, sizeX, sizeY };
Uint32 start;
SDL_Point offset = { 48, 24 };

struct piece{
	int x, y, col, row, kind, match, alpha;
	piece() {
		match = 0; 
		alpha = 255; 
	}
} grid[ItemsX + 2][ItemsY + 2];

void swap(piece p1, piece p2)
{
	std::swap(p1.col, p2.col);
	std::swap(p1.row, p2.row);
	grid[p1.row][p1.col] = p1;
	grid[p2.row][p2.col] = p2;
}

int main(int argc, char ** argv) {
	srand(time(0));
	// Error checks
	std::cout << "SDL_Init\n";
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *win = SDL_CreateWindow("Bejeweled", 100, 100, sizeX, sizeY, 0);
	if (win == nullptr) {
		std::cout << "SDL_CreateWindow error\n";
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		std::cout << "SDL_CreateRenderer error\n";
	}
	// Load bitmaps
	std::vector<std::string> Bitmaps;
	Bitmaps.push_back("img/background.png");
	Bitmaps.push_back("img/gems.png");
	Bitmaps.push_back("img/cursor.png");
	// Create textures from bitmaps
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		std::cout << "SDL_image could not initialize! SDL_image Error:" << IMG_GetError() << std::endl;
	}
	std::vector<SDL_Texture *> Textures;
	for (auto bitmap : Bitmaps) {
		Textures.push_back(IMG_LoadTexture(renderer, bitmap.c_str()));
		if (Textures.back() == nullptr) {
			std::cout << bitmap.c_str() << " SDL_CreateTextureFromSurface error\n";
		}
		else
			std::cout << bitmap.c_str() << " SDL_CreateTextureFromSurface OK\n";
	}
	// generate gems
	for (int i = 1; i <= ItemsX; i++)
		for (int j = 1; j <= ItemsY; j++)
		{
			grid[i][j].kind = rand() % 7;
			grid[i][j].col = j;
			grid[i][j].row = i;
			grid[i][j].x = j * sizeItem;
			grid[i][j].y = i * sizeItem;
		}
	int x0 = 0, y0 = 0, x, y; 
	int click = 0; 
	SDL_Point pos;
	bool isSwap = false, isMoving = false;
	while (1) {
		start = SDL_GetTicks();
		SDL_Event e;
		if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				break;
			}
			//SDL_Point mousePos;
			//mousePos.x = (e.motion.x - offset.x)/ sizeItem;
			//mousePos.y = (e.motion.y - offset.y)/ sizeItem;
			if (e.button.button == SDL_BUTTON_LEFT) {
				//std::cout << mousePos.x << "," << mousePos.y << std::endl;
				if (!isSwap && !isMoving) click++;
				pos.x = e.motion.x - offset.x;
				pos.y = e.motion.y - offset.y;
			}
		}
		// mouse click
		if (click == 1)	{
			x0 = pos.x / sizeItem + 1;
			y0 = pos.y / sizeItem + 1;
		}
		if (click == 2)	{
			x = pos.x / sizeItem + 1;
			y = pos.y / sizeItem + 1;
			if (abs(x - x0) + abs(y - y0) == 1)	{
				swap(grid[y0][x0], grid[y][x]); 
				isSwap = 1; 
				click = 0;
			}
			else 
				click = 1;
		}
		//Match finding
		for (int i = 1; i <= 8; i++)
			for (int j = 1; j <= 8; j++) {
				if (grid[i][j].kind == grid[i + 1][j].kind)
					if (grid[i][j].kind == grid[i - 1][j].kind)
						for (int n = -1; n <= 1; n++) 
							grid[i + n][j].match++;

				if (grid[i][j].kind == grid[i][j + 1].kind)
					if (grid[i][j].kind == grid[i][j - 1].kind)
						for (int n = -1; n <= 1; n++) 
							grid[i][j + n].match++;
			}
		//Moving animation
		isMoving = false;
		for (int i = 1; i <= 8; i++)
			for (int j = 1; j <= 8; j++) {
				piece &p = grid[i][j];
				int dx = p.x - p.col*sizeItem;
				int dy = p.y - p.row*sizeItem;
				if (dx) p.x -= dx / abs(dx);
				if (dy) p.y -= dy / abs(dy);
				if (dx || dy) isMoving = 1;
			}

		//Deleting amimation
		if (!isMoving)
			for (int i = 1; i <= 8; i++)
				for (int j = 1; j <= 8; j++)
					if (grid[i][j].match) 
						if (grid[i][j].alpha>10) { 
							grid[i][j].alpha -= 50; 
							isMoving = true; 
						}
		//Get score
		int score = 0;
		for (int i = 1; i <= 8; i++)
			for (int j = 1; j <= 8; j++)
				score += grid[i][j].match;

		//Second swap if no match
		if (isSwap && !isMoving)
		{
			if (!score) 
				swap(grid[y0][x0], grid[y][x]); 
			isSwap = 0;
		}

		//Update grid
		if (!isMoving)
		{
			for (int i = 8; i>0; i--)
				for (int j = 1; j <= 8; j++)
					if (grid[i][j].match)
						for (int n = i; n>0; n--)
							if (!grid[n][j].match) { 
								swap(grid[n][j], grid[i][j]); 
								break; 
							};

			for (int j = 1; j <= 8; j++)
				for (int i = 8, n = 0; i>0; i--)
					if (grid[i][j].match)
					{
						grid[i][j].kind = rand() % 7;
						grid[i][j].y = -sizeItem*n++;
						grid[i][j].match = 0;
						grid[i][j].alpha = 255;
					}
		}

		SDL_RenderClear(renderer);
		// board texture
		SDL_RenderCopy(renderer, Textures[BACKGROUND], nullptr, &boardRect);
		for (int i = 1; i <= 8; i++)
			for (int j = 1; j <= 8; j++) {
				piece p = grid[i][j];
				SDL_Rect rectGem = { p.x, p.y - offset.y, 49, 49 };
				SDL_Rect rectSprite = {p.kind * 49, 0, 49, 49 };
				//gems.setColor(Color(255, 255, 255, p.alpha));
				SDL_RenderCopy(renderer, Textures[GEMS], &rectSprite, &rectGem);
		}
		// cursor
		SDL_Rect rectCursor = { x0*sizeItem, y0*sizeItem - offset.y, 49, 49 };
		SDL_RenderCopy(renderer, Textures[CURSOR], nullptr, &rectCursor);

		SDL_RenderPresent(renderer);
		// Animation delay
		if (ANIMATION_DELAY / FPS > SDL_GetTicks() - start)
			SDL_Delay(ANIMATION_DELAY / FPS - (SDL_GetTicks() - start));
	}
	IMG_Quit();
	for (auto texture : Textures)
		SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();
	std::cout << "SDL_Quit\n";
	return 0;
}