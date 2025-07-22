#include <stdio.h>
#include <SDL3/SDL.h>
#include <math.h>

#define WIDTH 600
#define HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_BG 0xFF1F1F1F
#define PI 3.14159265359
#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define TILE_SIZE 100
#define FOV 30.0
#define NUM_RAYS WIDTH

int map[] = {
	1,1,1,1,1,1,1,1,1,1,
	1,0,1,0,0,0,0,0,0,1,
	1,0,1,0,0,0,0,1,1,1,
	1,0,0,0,0,1,0,1,0,1,
	1,0,0,0,0,0,0,1,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,1,0,0,1,0,1,
	1,0,0,0,1,0,0,1,0,1,
	1,1,1,1,1,1,1,1,1,1
};

float playerX = 300, playerY = 300;
float playerAngle = 0;
float moveSpeed = 5.0;
float rotSpeed =  (PI / 180);

int get_map(int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT)
        return 1;
    return map[y * MAP_WIDTH + x];
}

void draw_rect(SDL_Renderer* renderer, int x, int y, int w, int h, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(renderer, &rect);
}

bool wall_at(float x, float y) {
    int mapX = (int)(x / TILE_SIZE);
    int mapY = (int)(y / TILE_SIZE);
    return get_map(mapX, mapY) != 0;
}

void render_map(SDL_Renderer* renderer) {
	    for (int y = 0; y < MAP_HEIGHT; y++) {
	        for (int x = 0; x < MAP_WIDTH; x++) {
	            SDL_Color color = map[y * MAP_WIDTH + x] ? (SDL_Color) { 200, 200, 200 } : (SDL_Color) { 50, 50, 50 };
	            draw_rect(renderer, x * 10, y * 10, 10, 10, color);
	        }
	    }
	
	    // draw player
	    draw_rect(renderer, playerX / 6 - 2, playerY / 6 - 2, 4, 4, (SDL_Color) { 255, 0, 0 });
}

void render_3d(SDL_Renderer* renderer) {
	    float rayAngle = playerAngle - (FOV * (PI / 180)) / 2;
	    for (int r = 0; r < NUM_RAYS; r++) {
	        float rayX = playerX;
	        float rayY = playerY;
	
	        float rayStep = 0.5;
	        while (!wall_at(rayX, rayY)) {
	            rayX += cos(rayAngle) * rayStep;
	            rayY += sin(rayAngle) * rayStep;
	        }
	
	        float dist = hypot(rayX - playerX, rayY - playerY);
	        dist *= cos(rayAngle - playerAngle); // remove fisheye
	
	        float lineHeight = (TILE_SIZE * HEIGHT) / (dist + 0.0001f);
	        if (lineHeight > HEIGHT) lineHeight = HEIGHT;
	
	        int drawStart = HEIGHT / 2 - lineHeight / 2;
	        int drawEnd = drawStart + lineHeight;
	
	        // shading based on distance
	        int shade = 255 - (int)(dist * 0.5f);
	        if (shade < 50) shade = 50;
	        if (shade > 255) shade = 255;
	        SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);
	        SDL_RenderLine(renderer, r, drawStart, r, drawEnd);
	
	        rayAngle += (FOV * (PI / 180)) / (NUM_RAYS);
	    }
	}

void move_player(const Uint8* keys) {
    float nextX = playerX;
    float nextY = playerY;

    if (keys[SDL_SCANCODE_W]) {
        nextX += cos(playerAngle) * moveSpeed;
        nextY += sin(playerAngle) * moveSpeed;
    }
    if (keys[SDL_SCANCODE_S]) {
        nextX -= cos(playerAngle) * moveSpeed;
        nextY -= sin(playerAngle) * moveSpeed;
    }
    if (keys[SDL_SCANCODE_A])
        playerAngle -= rotSpeed;
    if (keys[SDL_SCANCODE_D])
        playerAngle += rotSpeed;

    if (!wall_at(nextX, nextY)) {
        playerX = nextX;
        playerY = nextY;
    }
}

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Raycaster in C", WIDTH, HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, 0);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	if (!window) {
		printf("WINDOW ISSUE");
		SDL_Quit();
		return -1;
	}

	if (!surface) {
		printf("SURFACE ISSUE");
		SDL_Quit();
		return -1;
	}
	if (!renderer) {
		SDL_Log("Renderer creation failed: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	int running = 1;
	SDL_Event event;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	while (running==1) {
		SDL_PumpEvents();
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = 0;
			}
		}
		move_player(keys);
		SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
		SDL_RenderClear(renderer);
		render_3d(renderer);
		render_map(renderer);
		SDL_RenderPresent(renderer);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroySurface(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}