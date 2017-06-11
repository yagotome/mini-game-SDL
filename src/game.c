#include "engine/game.h"
#include <math.h>

#define SONIC_AMOUNT 19
#define FALSE 0

typedef enum { false, true } bool;

SDL_Window *window;
SDL_Renderer *renderer;
Mix_Music *bgm;
Mix_Chunk *sound_jump, *sound_crash;
SDL_Rect body_background = {0, 0, 1280, 330},
		 body_ground = {0, 330, 1280, 150},
		 body_sonic = {80, 235, 82, 100};
SDL_Texture *background, *ground, *obstacle, *sonic[SONIC_AMOUNT];

int gSpeed = -10, bgSpeed = -1, jumpSpeed = -20, stage = 1, sonic_sprite = 0, frames_passed = 0, gameSpeed = 1;

bool isJumping = false, isFalling = false;
int timeJumping = 0;

void load()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Sonic, jump!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	bgm = Mix_LoadMUS("../audio/sonic_theme.mp3");
	sound_jump = Mix_LoadWAV("../audio/jump.wav");
	sound_crash = Mix_LoadWAV("../audio/crash.wav");

	background = IMG_LoadTexture(renderer, "../img/background.jpg");
	ground = IMG_LoadTexture(renderer, "../img/ground.jpg");
	obstacle = IMG_LoadTexture(renderer, "../img/obstacle.jpg");
	int i;
	for (i = 0; i < SONIC_AMOUNT; i++)
	{
		char sonic_path[20];
		sprintf(sonic_path, "../img/sonic_%02d.png", i + 1);
		sonic[i] = IMG_LoadTexture(renderer, sonic_path);
	}

	Mix_PlayMusic(bgm, -1);
}

void onExit(const SDL_Event *e)
{
	Mix_HaltMusic();
	Mix_FreeMusic(bgm);
	Mix_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void onKeyDown(const SDL_Event *e)
{
	if (e->key.keysym.sym == SDLK_UP && !isJumping)
	{
		isJumping = true;
		Mix_PlayChannel(-1, sound_jump, 0);
	}
}

void move_x(SDL_Rect *r, const int speed, const int dt)
{
	r->x += gameSpeed * (int)round(speed * (float)FPS / dt);
}

void move_y(SDL_Rect *r, const int speed, const int dt)
{
	r->y += gameSpeed * (int)round(speed * (float)FPS / dt);
}

void update(const Uint32 dt)
{
	// printf("dt %d\n", dt);

	// moving background
	if (body_background.x < -640)
		body_background.x = 0;
	else
		move_x(&body_background, bgSpeed, dt);

	// moving ground
	if (body_ground.x < -640)
		body_ground.x = 0;
	else
		move_x(&body_ground, gSpeed, dt);

	/*
		else if(sonic COLIDIU COM obstacle)
		{
			Mix_PlayChannel(-1, crash, 0);
			game_over();
		}
	*/

	// make sonic jump
	if (isJumping)
	{
		if (body_sonic.y > 60 && !isFalling)
		{
			move_y(&body_sonic, jumpSpeed, dt);
		}
		else
		{
			if (!isFalling) isFalling = true;
			move_y(&body_sonic, -jumpSpeed, dt);
		}

		if(body_sonic.y >= 235 && (stage == 1 || stage == 2))
		{
			body_sonic.y = 235;
			isJumping = false;
			isFalling = false;
		} 
		else if(body_sonic.y >= 255 && stage == 3)
		{
			body_sonic.y = 255;
			isJumping = false;
			isFalling = false;
		}
	}

	Uint32 time = SDL_GetTicks();
	if (time > 30 * 1000 && time < 50 * 1000 && stage == 1)
	{
		stage = 2;
		gameSpeed++;
	}
	else if (time > 50 * 1000 && stage == 2)
	{
		stage = 3;
		gameSpeed++;
		body_sonic.x = 82;
		body_sonic.y = 255;
		body_sonic.w = 80;
		body_sonic.h = 80;
	}
}

void draw()
{
	SDL_RenderCopy(renderer, background, NULL, &body_background);
	SDL_RenderCopy(renderer, ground, NULL, &body_ground);

	if (isJumping)
	{
		sonic_sprite = 12 + ((sonic_sprite + 1) % 7);
	}
	else
	{
		switch (stage)
		{
		case 1:
			sonic_sprite = (sonic_sprite + 1) % 8;
			break;
		case 2:
			sonic_sprite = 8 + ((sonic_sprite + 1) % 4);
			break;
		case 3:
			sonic_sprite = 12 + ((sonic_sprite + 1) % 7);
			break;
		}
	}
	
	SDL_RenderCopy(renderer, sonic[sonic_sprite], NULL, &body_sonic);
	SDL_RenderPresent(renderer);
	frames_passed++;
}