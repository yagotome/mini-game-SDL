#include "engine/game.h"
#include <math.h>

#define SONIC_AMOUNT 19

typedef enum { false, true } bool;

SDL_Window *window;
SDL_Renderer *renderer;
Mix_Music *bgm;
Mix_Chunk *sound_jump, *sound_crash;
SDL_Rect body_background = {0, 0, 1280, 330},
		 body_ground = {0, 330, 1280, 150},
		 body_sonic = {80, 235, 82, 100};
SDL_Texture *background, *ground, *obstacle, *sonic[SONIC_AMOUNT];

int obstacle_amount = 0, obstacle_time = 0, new_obstacles_index = 0;
SDL_Rect *body_obstacles, obstacle_initial_position = {640, 255, 60, 80};

int g_speed = -50, bg_speed = -5, jump_speed = -80, stage = 1, sonic_sprite = 0, game_speed = 1,
	stage_time = 0, jump_time = 0, sonic_time = 0;

bool is_jumping = false, is_falling = false;

bool is_finishing = false;

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
	obstacle = IMG_LoadTexture(renderer, "../img/obstacle.png");
	int i;
	for (i = 0; i < SONIC_AMOUNT; i++)
	{
		char sonic_path[20];
		sprintf(sonic_path, "../img/sonic_%02d.png", i + 1);
		sonic[i] = IMG_LoadTexture(renderer, sonic_path);
	}

	Mix_PlayMusic(bgm, -1);
}

void onExit()
{
	free(background);
	free(ground);
	free(obstacle);
	int i; for (i = 0; i < SONIC_AMOUNT; i++) free(sonic[i]);
	Mix_HaltMusic();
	Mix_FreeMusic(bgm);
	Mix_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void onKeyDown(const SDL_Event *e)
{
	if (e->key.keysym.sym == SDLK_UP && !is_jumping)
	{
		is_jumping = true;
		Mix_PlayChannel(-1, sound_jump, 0);
	}
}

void move_x(SDL_Rect *r, const int speed, const int dt)
{
	r->x += (int)round(speed*0.01 * dt);
}

void move_y(SDL_Rect *r, const int speed, const int dt)
{
	r->y += (int)round(speed*0.01 * dt);
}

bool is_colliding(SDL_Rect *rect1, SDL_Rect* rect2)
{
	int l1 = rect1->x, r1 = rect1->x + rect1->w,
		l2 = rect2->x, r2 = rect2->x + rect2->w;		
	int t1 = rect1->y, b1 = rect1->y + rect1->h,
		t2 = rect2->y, b2 = rect2->y + rect2->h;
	return 	(l1 >= l2 && l1 <= r2 || r1 >= l2 && r1 <= r2) && 
			(t1 >= t2 && t1 <= b2 || b1 >= t2 && b1 <= b2);	
}

void update(const Uint32 dt)
{
	if (is_finishing) return;
	// printf("dt %d\n", dt);

	// moving background
	if (body_background.x < -640)
		body_background.x = 0;
	else
		move_x(&body_background, bg_speed, dt);

	// moving ground
	if (body_ground.x < -640)
		body_ground.x = 0;
	else
		move_x(&body_ground, g_speed, dt);

	// moving sonic
	sonic_time += dt;
	if (sonic_time > 50)
	{		
		if (is_jumping)
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
		sonic_time = 0;
	}

	// make sonic jump
	if (is_jumping)
	{
		if (body_sonic.y > 60 && !is_falling)
		{
			move_y(&body_sonic, jump_speed, dt);
		}
		else
		{
			if (!is_falling)
				is_falling = true;
			jump_time += dt;
			if (jump_time > 400)
			{
				move_y(&body_sonic, -jump_speed, dt);
			}
		}

		if (body_sonic.y >= 235 && (stage == 1 || stage == 2))
		{
			body_sonic.y = 235;
			is_jumping = false;
			is_falling = false;
			jump_time = 0;
		}
		else if (body_sonic.y >= 255 && stage == 3)
		{
			body_sonic.y = 255;
			is_jumping = false;
			is_falling = false;
			jump_time = 0;
		}
	}

	// moving obstacles and test collisions
	int i;
	for (i = 0; i < obstacle_amount; i++)
	{
		move_x(&body_obstacles[i], g_speed, dt);
		if (is_colliding(&body_obstacles[i], &body_sonic))
		{
			Mix_PlayChannel(-1, sound_crash, 0);
			finish(2000);
			is_finishing = true;
			return;
		}
	}

	// creating new obstacles
	obstacle_time += dt;
	if (obstacle_time > 1000 - 20*game_speed && (rand() % 50) == 0)
	{		
		if (obstacle_amount < 5)
			body_obstacles = (SDL_Rect *)realloc(body_obstacles, (++obstacle_amount) * sizeof(SDL_Rect));			
		new_obstacles_index = (new_obstacles_index + 1) % obstacle_amount;
		body_obstacles[new_obstacles_index] = obstacle_initial_position;
		obstacle_time = 0;
		// for (i = 0; i < obstacle_amount; i++)
		// 	printf("{%d, %d, %d, %d}\n",
		// 		body_obstacles[i].x,
		// 		body_obstacles[i].y,
		// 		body_obstacles[i].w,
		// 		body_obstacles[i].h);
	}


	stage_time += dt;
	if (stage_time > 7000)
	{
		bg_speed -= 5;
		g_speed -= 5;
		stage_time = 0;
		game_speed++;
	}

	Uint32 time = SDL_GetTicks();
	if (time > 40 * 1000 && time < 70 * 1000 && stage == 1)
	{
		stage = 2;
	}
	else if (time > 70 * 1000 && stage == 2)
	{
		stage = 3;
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
	SDL_RenderCopy(renderer, sonic[sonic_sprite], NULL, &body_sonic);
	int i;
	for (i = 0; i < obstacle_amount; i++)
	{
		SDL_RenderCopy(renderer, obstacle, NULL, &body_obstacles[i]);
	}
	SDL_RenderPresent(renderer);
}