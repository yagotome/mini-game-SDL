/*
----Universidade do Estado do Rio de Janeiro--------------------
----Sistemas Reativos-------------------------------------------
----Mini-projeto: Sonic, jump!----------------------------------
----Autores: Lucas Alves de Sousa & Yago Gomes Tomé de Sousa----
*/

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include "engine/game.h"

#define SONIC_AMOUNT 19
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

typedef enum {
	false,
	true
} bool;

struct SDL_Rect_Chained
{
	SDL_Rect body;
	struct SDL_Rect_Chained *next;
};

typedef struct SDL_Rect_Chained SDL_Rect_Chained;

typedef struct
{
	uint32_t score;
	char score_str[11]; /* length of max number of uint32_t */
	TTF_Font *font;
	SDL_Color color;
	SDL_Rect body;
} Score;

const float background_speed = 50.0, background_acceleration = 2.0, ground_speed = 200.0, ground_acceleration = 8.0, initial_speed_y = 750.0, initial_acceleration_y = -1300.0;
float i_background = 0.0, i_ground = 0.0, i_jump = 0.0, actual_speed_y = 0.0, actual_acceleration_y = 0.0, sprite_time = 0.0;
const int obstacle_distance = 400, stage_time = 10;
int stage = 1, sonic_sprite = 0, sprites_per_second = 15, i = 0;
bool is_finishing = false, is_jumping = false, is_dropping = false, fast_drop = false, low_drop = false;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Rect background_body = {0, 0, 1280, 330}, ground_body = {0, 330, 1280, 150}, sonic_body = {80, 235, 82, 100};
SDL_Rect_Chained *obstacles_list;
SDL_Texture *background, *ground, *sonic[SONIC_AMOUNT], *obstacle_texture;
Mix_Music *bgm;
Mix_Chunk *sound_jump, *sound_crash;
Score score;
const SDL_Color white_color = {255, 255, 255};

void load()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Sonic, jump!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
	obstacles_list = (SDL_Rect_Chained *)malloc(sizeof(SDL_Rect_Chained));
	obstacles_list->next = NULL;
	(obstacles_list->body).x = 640;
	(obstacles_list->body).y = 255;
	(obstacles_list->body).w = 60;
	(obstacles_list->body).h = 80;
	background = IMG_LoadTexture(renderer, "../res/img/background.jpg");
	ground = IMG_LoadTexture(renderer, "../res/img/ground.jpg");
	obstacle_texture = IMG_LoadTexture(renderer, "../res/img/obstacle.png");
	int i;
	for (i = 0; i < SONIC_AMOUNT; i++)
	{
		char sonic_path[20];
		sprintf(sonic_path, "../res/img/sonic_%02d.png", i + 1);
		sonic[i] = IMG_LoadTexture(renderer, sonic_path);
	}
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	bgm = Mix_LoadMUS("../res/audio/sonic_theme.mp3");
	sound_jump = Mix_LoadWAV("../res/audio/jump.wav");
	sound_crash = Mix_LoadWAV("../res/audio/crash.wav");
	Mix_VolumeChunk(sound_jump, 30);
	Mix_VolumeChunk(sound_crash, 30);
	Mix_VolumeMusic(35);
	Mix_PlayMusic(bgm, -1);
	TTF_Init();
	score.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	score.color = white_color;
	score.body.h = 40;
	score.body.y = WINDOW_HEIGHT - score.body.h - 10;
}

void onExit()
{
	free(background);
	free(ground);
	free(obstacle_texture);
	int i;
	for (i = 0; i < SONIC_AMOUNT; i++)
	{
		free(sonic[i]);
	}
	SDL_Rect_Chained *temp = obstacles_list;
	while (temp != NULL)
	{
		obstacles_list = obstacles_list->next;
		free(temp);
		temp = obstacles_list;
	}
	TTF_CloseFont(score.font);
	Mix_HaltMusic();
	Mix_FreeMusic(bgm);
	Mix_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void onKeyDown(const SDL_Event *event)
{
	if (event->key.keysym.sym == SDLK_UP)
	{
		if (!is_jumping)
		{
			is_jumping = true;
			i_jump = 0;
			actual_speed_y = initial_speed_y;
			actual_acceleration_y = initial_acceleration_y;
			sonic_body.w = 80;
			sonic_body.h = 80;
			sonic_body.y = 234;
			Mix_PlayChannel(-1, sound_jump, 0);
		}
		else if (is_dropping)
		{
			actual_acceleration_y = -200;
			low_drop = true;
		}
	}
	else if (event->key.keysym.sym == SDLK_DOWN && is_jumping)
	{
		actual_acceleration_y = -10000.0;
		fast_drop = true;
	}
}

void onKeyUp(const SDL_Event *event)
{
	if (event->key.keysym.sym == SDLK_UP)
	{
		low_drop = false;
	}
}

void update(Uint32 dt, Uint32 time)
{
	if (is_finishing)
		return;

	/* VERIFICA COLISÃO ENTRE SONIC E O PRIMEIRO OBSTÁCULO DA LISTA */
	SDL_Rect_Chained *first_obstacle;
	first_obstacle = obstacles_list;
	if ((first_obstacle->body).x <= 155 && (first_obstacle->body).x >= 30)
	{
		if (!is_jumping || (first_obstacle->body).y <= sonic_body.y + 75)
		{
			Mix_PlayChannel(-1, sound_crash, 0);
			Mix_VolumeMusic(15);
			is_finishing = true;
			finishTimeout(2000);
			return;
		}
	}

	/* ATUALIZA POSIÇÃO Y DO SONIC NO PULO */
	if (is_jumping)
	{
		i_jump += actual_speed_y * (dt / 1000.0);
		actual_speed_y += actual_acceleration_y * (dt / 1000.0);

		if (actual_speed_y < 0)
			is_dropping = true;

		if (i_jump > 1 || i_jump < 1)
		{
			sonic_body.y -= (int)i_jump;
			i_jump -= (int)i_jump;
		}
		if (sonic_body.y >= 255)
		{
			is_jumping = false;
			if (fast_drop)
				fast_drop = false;
			if (low_drop)
				low_drop = false;
			if (is_dropping)
				is_dropping = false;
			sonic_body.w = 82;
			sonic_body.h = 100;
			sonic_body.y = 235;
		}
		if (!fast_drop && !low_drop)
		{
			actual_acceleration_y = initial_acceleration_y;
		}
	}

	/* CRIA NOVO OBSTÁCULO A CADA <obstacle_distance> PIXELS DE DISTÂNCIA */
	SDL_Rect_Chained *obstacle = obstacles_list;
	while (obstacle->next != NULL)
	{
		obstacle = obstacle->next;
	}
	if ((obstacle->body).x < (WINDOW_WIDTH - obstacle_distance))
	{
		obstacle->next = (SDL_Rect_Chained *)malloc(sizeof(SDL_Rect_Chained));
		obstacle->next->next = NULL;
		(obstacle->next->body).x = WINDOW_WIDTH;
		(obstacle->next->body).y = 255;
		(obstacle->next->body).w = 60;
		(obstacle->next->body).h = 80;
	}

	/* DESALOCA OBSTÁCULO QUANDO ELE SAI DA TELA */
	if ((obstacles_list->body).x < -60)
	{
		SDL_Rect_Chained *obstacle_to_destroy;
		obstacle_to_destroy = obstacles_list;
		obstacles_list = obstacles_list->next;
		free(obstacle_to_destroy);
	}

	/* ATUALIZA POSIÇÃO X DO PLANO DE FUNDO, CHÃO E OBSTÁCULOS */
	if (background_body.x <= -WINDOW_WIDTH)
		background_body.x = 0;
	i_background += (background_speed + background_acceleration * (time / 1000)) * (dt / 1000.0);
	if (i_background >= 1)
	{
		background_body.x -= (int)i_background;
		i_background -= (int)i_background;
	}
	if (ground_body.x <= -WINDOW_WIDTH)
		ground_body.x = 0;
	i_ground += (ground_speed + ground_acceleration * (time / 1000)) * (dt / 1000.0);
	if (i_ground >= 1)
	{
		SDL_Rect_Chained *temp = obstacles_list;
		while (temp != NULL)
		{
			(temp->body).x -= (int)i_ground;
			temp = temp->next;
		}
		ground_body.x -= (int)i_ground;
		i_ground -= (int)i_ground;
	}

	/* NOVO BLOCO DE SPRITES QUANDO TEMPO DE JOGO > 10 SEGUNDOS */
	if (time > stage_time * 1000 && stage == 1)
	{
		stage = 2;
	}

	/* ATUALIZA SPRITE */
	sprite_time += dt;
	if (sprite_time >= 1000 / sprites_per_second)
	{
		if (is_jumping)
		{
			sonic_sprite = 12 + ((sonic_sprite + 3) % 7);
		}
		else
		{
			switch (stage)
			{
			case 1:
				sonic_sprite = (sonic_sprite + 1) % 8;
				break;
			case 2:
				sonic_sprite = 8 + (sonic_sprite + 1) % 4;
				sprites_per_second = 20;
				break;
			}
		}
		sprite_time = 0;
	}

	/* ATUALIZA SCORE */
	score.score = (uint32_t)(time/163);
	sprintf(score.score_str, "%u", score.score);
	score.body.w = 40 * strlen(score.score_str);
	score.body.x = WINDOW_WIDTH - score.body.w - 16;
}

void draw()
{
	SDL_RenderCopy(renderer, background, NULL, &background_body);
	SDL_RenderCopy(renderer, ground, NULL, &ground_body);
	SDL_RenderCopy(renderer, sonic[sonic_sprite], NULL, &sonic_body);
	SDL_Rect_Chained *obstacle = obstacles_list;
	while (obstacle != NULL)
	{
		SDL_RenderCopy(renderer, obstacle_texture, NULL, &(obstacle->body));
		obstacle = obstacle->next;
	}
	SDL_Surface *score_surface = TTF_RenderText_Solid(score.font, score.score_str, score.color);	
	SDL_Texture *score_texture = SDL_CreateTextureFromSurface(renderer, score_surface);
	SDL_RenderCopy(renderer, score_texture, NULL, &score.body);

	SDL_RenderPresent(renderer);
}
