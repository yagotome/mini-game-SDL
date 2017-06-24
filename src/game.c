/*
----Universidade do Estado do Rio de Janeiro--------------------
----Sistemas Reativos-------------------------------------------
----Mini-projeto: Sonic, jump!----------------------------------
----Autores: Lucas Alves de Sousa & Yago Gomes Tomé de Sousa----
*/

#include "engine/game.h"

#define SONIC_AMOUNT 19

typedef enum { false, true } bool;

struct SDL_Rect_Chained
{
	SDL_Rect body;
	struct SDL_Rect_Chained *next; 
};

const float background_speed = 50.0, background_acceleration = 2.0, ground_speed = 200.0, ground_acceleration = 8.0, initial_speed_y = 750.0, initial_acceleration_y = -1300.0;
float i_background = 0.0, i_ground = 0.0, i_jump = 0.0, actual_speed_y = 0.0, actual_acceleration_y = 0.0, sprite_time = 0.0;
const int obstacle_distance = 400, stage_time = 10;
int stage = 1, sonic_sprite = 0, sprites_per_second = 15, i = 0;
bool is_finishing = false, is_jumping = false, is_dropping = false, fast_drop = false, low_drop = false;

Uint32 time;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Rect background_body = {0, 0, 1280, 330}, ground_body = {0, 330, 1280, 150}, sonic_body = {80, 235, 82, 100};
struct SDL_Rect_Chained *obstacles_list;
SDL_Texture *background, *ground, *sonic[SONIC_AMOUNT], *obstacle;
SDL_Event event;
Mix_Music *bgm;
Mix_Chunk *sound_jump, *sound_crash;

void loadGame()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Sonic, jump!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window,-1,0);
	obstacles_list = (struct SDL_Rect_Chained *) malloc(sizeof(struct SDL_Rect_Chained *));
	obstacles_list->next = NULL;
	(obstacles_list->body).x = 640;
	(obstacles_list->body).y = 255;				
	(obstacles_list->body).w = 60;
	(obstacles_list->body).h = 80;
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
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	bgm = Mix_LoadMUS("../audio/sonic_theme.mp3");
	sound_jump = Mix_LoadWAV("../audio/jump.wav");
	sound_crash = Mix_LoadWAV("../audio/crash.wav");
	Mix_VolumeChunk(sound_jump, 30);
	Mix_VolumeChunk(sound_crash, 30);
	Mix_VolumeMusic(35);
	Mix_PlayMusic(bgm, -1);
	time = SDL_GetTicks();
}

void exitGame()
{
	free(background);
	free(ground);
	free(obstacle);
	int i;
	for (i = 0; i < SONIC_AMOUNT; i++)
	{
		free(sonic[i]);
	}
	struct SDL_Rect_Chained *temp = obstacles_list;
	while (temp != NULL)
	{
		obstacles_list = obstacles_list->next;
		free(temp);
		temp = obstacles_list;
	}
	Mix_HaltMusic();
	Mix_FreeMusic(bgm);
	Mix_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void keyDown(const SDL_Event *event)
{
	if (event->type == SDL_QUIT)
	{
		exitGame();
	}
	else if (event->type == SDL_KEYDOWN)
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
			else if(is_dropping)
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
}

void drawGame()
{
	SDL_RenderCopy(renderer, background, NULL, &background_body);
	SDL_RenderCopy(renderer, ground, NULL, &ground_body);
	SDL_RenderCopy(renderer, sonic[sonic_sprite], NULL, &sonic_body);
	struct SDL_Rect_Chained *temp = obstacles_list;
	while(temp != NULL)
	{
		SDL_RenderCopy(renderer, obstacle, NULL, &(temp->body));
		temp = temp->next;
	}

	SDL_RenderPresent(renderer);
}

void updateGame(Uint32 dt)
{
	while (true)
	{

		dt = SDL_GetTicks() - time;
		time = SDL_GetTicks();
		if (SDL_PollEvent(&event))
		{
			low_drop = false;
			keyDown(&event);
		}
	
		if (is_finishing)
		{
			Mix_PlayChannel(-1, sound_crash, 0);
			Mix_VolumeMusic(15);
			SDL_Delay(2000);
			exitGame();
			break;
		}

		/* ATUALIZA POSIÇÃO Y DO SONIC NO PULO */
		if (is_jumping)
		{
			i_jump += actual_speed_y * (dt/1000.0);
			actual_speed_y += actual_acceleration_y * (dt/1000.0);
			
			if (actual_speed_y < 0) is_dropping = true;

			if (i_jump > 1 || i_jump < 1)
			{
				sonic_body.y -= (int)i_jump;
				i_jump -= (int)i_jump;
			}
			if (sonic_body.y >= 255)
			{
				is_jumping = false;
				if (fast_drop) fast_drop = false;
				if (low_drop) low_drop = false;
				if (is_dropping) is_dropping = false;
				sonic_body.w = 82;
				sonic_body.h = 100;
				sonic_body.y = 235;
			}	
			if (!fast_drop && !low_drop) {
				actual_acceleration_y = initial_acceleration_y;
			}
		}

		/* CRIA NOVO OBSTÁCULO A CADA <obstacle_distance> PIXELS DE DISTÂNCIA */
		struct SDL_Rect_Chained *temp = obstacles_list;
		while (temp->next != NULL)
		{
			temp = temp->next;
		}
		if ((temp->body).x < (640 - obstacle_distance))
		{
			temp->next = (struct SDL_Rect_Chained *) malloc(sizeof(struct SDL_Rect_Chained *));
			temp->next->next = NULL;
			(temp->next->body).x = 640;
			(temp->next->body).y = 255;
			(temp->next->body).w = 60;
			(temp->next->body).h = 80;
		}

		/* DESALOCA OBSTÁCULO QUANDO ELE SAI DA TELA */
		if ((obstacles_list->body).x < -60)
		{
			temp = obstacles_list;
			obstacles_list = obstacles_list->next;
			free(temp);
		}

		/* ATUALIZA POSIÇÃO X DO PLANO DE FUNDO, CHÃO E OBSTÁCULOS */
		if (background_body.x <= -640)
			background_body.x = 0;
		i_background += (background_speed + background_acceleration * (time/1000)) * (dt/1000.0);
		if (i_background >= 1)
		{
			background_body.x -= (int)i_background;
			i_background -= (int)i_background;
		}
		if (ground_body.x <= -640)
			ground_body.x = 0;
		i_ground += (ground_speed + ground_acceleration * (time/1000)) * (dt/1000.0);
		if (i_ground >= 1)
		{
			struct SDL_Rect_Chained *temp = obstacles_list;
			while (temp != NULL)
			{
				(temp->body).x -= (int)i_ground;
				temp = temp->next;				
			}
			ground_body.x -= (int)i_ground;
			i_ground -= (int)i_ground;
		}

		/* NOVO BLOCO DE SPRITES QUANDO TEMPO DE JOGO > 10 SEGUNDOS */
		if (time > stage_time*1000 && stage == 1)
		{
			stage = 2;
		}

		/* ATUALIZA SPRITE */
		sprite_time += dt;
		if (sprite_time >= 1000/sprites_per_second)
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

		/* VERIFICA COLISÃO ENTRE SONIC E O PRIMEIRO OBSTÁCULO DA LISTA */
		temp = obstacles_list;
		if ((temp->body).x <= 155 && (temp->body).x >= 30)
		{
			if (is_jumping)
			{
				if ((temp->body).y <= sonic_body.y + 75)
				{
					is_finishing = true;
				}
			}
			else
			{
				is_finishing = true;
			}
		}

		drawGame();
	}
}
