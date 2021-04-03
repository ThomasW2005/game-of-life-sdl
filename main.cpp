#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <chrono>

#define RESOLUTION 10
#define WIDTH 1280
#define HEIGHT 720
#define SLEEP 16
#define XSIZE (WIDTH/RESOLUTION)
#define YSIZE (HEIGHT/RESOLUTION)
#define drawCells(CELL) SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);\
SDL_RenderClear(renderer); \
for (int j = 0; j < YSIZE; j++) \
{ \
for (int i = 0; i < XSIZE; i++) \
{ \
if (CELL[i][j].state == 1) \
{ \
SDL_SetRenderDrawColor(renderer, 0, 166, 140, 255); \
SDL_Rect rect; \
rect.x = i * RESOLUTION; \
rect.y = j * RESOLUTION; \
rect.w = RESOLUTION; \
rect.h = RESOLUTION; \
SDL_RenderFillRect(renderer, &rect); \
} \
} \
} \
if(grid)\
{drawGrid(renderer);}\
SDL_RenderPresent(renderer);

#define drawGrid(REND) 	SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255); \
for (int i = 0; i < XSIZE - 1; i++)\
	SDL_RenderDrawLine(REND, i * RESOLUTION + RESOLUTION, 0, i * RESOLUTION + RESOLUTION, HEIGHT);\
for (int i = 0; i < YSIZE; i++)\
	SDL_RenderDrawLine(REND, 0, i * RESOLUTION + RESOLUTION, WIDTH, i * RESOLUTION + RESOLUTION);

typedef struct
{
	int state;
	int age;
}CELLS;

void initCells(CELLS cells[XSIZE][YSIZE], bool clear);
void updateCells(CELLS cells[XSIZE][YSIZE], unsigned long int* generation);
int countNachbar(int x, int y, CELLS array[XSIZE][YSIZE]);
void calcCells(CELLS thisarray[XSIZE][YSIZE], CELLS nextarray[XSIZE][YSIZE]);
void copyCells(CELLS thisarray[XSIZE][YSIZE], CELLS nextarray[XSIZE][YSIZE]);
void manageEvents(SDL_Event* event, bool* update, bool* run, CELLS cells[XSIZE][YSIZE], bool* grid);
int main(int agrc, char* argc[])
{
	unsigned long int generation = 0;
	char title[200];
	bool update = true;
	bool run = true;
	bool grid = false;
	CELLS cells[XSIZE][YSIZE], nextcells[XSIZE][YSIZE];
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Event event;
	srand(time(0));
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);
	initCells(cells, false);

	while (run)
	{
		std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();	//clock start point

		drawCells(cells);
		manageEvents(&event, &update, &run, cells, &grid);	//events...
		if (update)
			updateCells(cells, &generation);

		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();	//clock end point
		std::chrono::duration<double> span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);	//clock difference
		sprintf_s(title, 200, "Game of Life, Generation: %d, Time per Frame: %.4fms, x: %03d, y: %03d, p = paused(%d), g = grid(%d), c = clear, esc = exit", generation, 1000 * span.count(), event.motion.x / RESOLUTION, event.motion.y / RESOLUTION, !update, grid);
		SDL_SetWindowTitle(window, title);
		SDL_Delay(SLEEP);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

void manageEvents(SDL_Event* event, bool* update, bool* run, CELLS cells[XSIZE][YSIZE], bool* grid)
{
	while (SDL_PollEvent(event))
	{

		switch (event->type)
		{
		case SDL_MOUSEBUTTONDOWN:
			if (event->button.button == SDL_BUTTON_LEFT)
				cells[event->motion.x / RESOLUTION][event->motion.y / RESOLUTION].state = 1;
			else if (event->button.button == SDL_BUTTON_RIGHT)
				cells[event->motion.x / RESOLUTION][event->motion.y / RESOLUTION].state = 0;
			break;
		case SDL_KEYDOWN:
			switch (event->key.keysym.scancode)
			{
			case SDL_SCANCODE_ESCAPE:
				*run = 0;
				break;
			case SDL_SCANCODE_P:
				*update = !*update;
				break;
			case SDL_SCANCODE_G:
				*grid = !*grid;
				break;
			case SDL_SCANCODE_C:
				initCells(cells, true);
			}
			break;
		case SDL_QUIT:
			*run = 0;
			break;
		}
	}
}

void copyCells(CELLS newcells[XSIZE][YSIZE], CELLS oldcells[XSIZE][YSIZE])
{
	for (int j = 0; j < YSIZE; j++)
		for (int i = 0; i < XSIZE; i++)
		{
			newcells[i][j].age = oldcells[i][j].age;
			newcells[i][j].state = oldcells[i][j].state;
		}
}

void initCells(CELLS cells[XSIZE][YSIZE], bool clear)
{
	for (int j = 0; j < YSIZE; j++)
		for (int i = 0; i < XSIZE; i++)
		{
			cells[i][j].age = 16;
			cells[i][j].state = clear ? 0 : (rand() % 2);

		}
}

void calcCells(CELLS newcells[XSIZE][YSIZE], CELLS oldcells[XSIZE][YSIZE])
{
	int state, nachbarn;
	for (int i = 0; i < XSIZE; i++)
	{
		for (int j = 0; j < YSIZE; j++)
		{
			state = oldcells[i][j].state;
			nachbarn = countNachbar(i, j, oldcells);
			newcells[i][j].state = (nachbarn == 3 || (nachbarn == 2 && oldcells[i][j].state));
		}
	}
}

int countNachbar(int x, int y, CELLS cells[XSIZE][YSIZE])
{
	int sum = 0, spalte, reihe;
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			spalte = (x + i + XSIZE) % XSIZE;
			reihe = (y + j + YSIZE) % YSIZE;
			sum += cells[spalte][reihe].state;
		}
	}
	sum -= cells[x][y].state;
	return sum;
}

void updateCells(CELLS cells[XSIZE][YSIZE], unsigned long int* generation)
{
	CELLS temp[XSIZE][YSIZE];
	calcCells(temp, cells);
	copyCells(cells, temp);
	(*generation)++;
}