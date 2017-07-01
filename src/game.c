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
#include <string.h>
#include <math.h>
#include <time.h>
#include "engine/game.h"

#define SONIC_AMOUNT 19
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define SCORE_HORIZONTAL_MARGIN 16
#define SCORE_VERTICAL_MARGIN 10
#define RECORD_FILE_NAME "record.txt"
#define min(a, b) (a < b ? a : b)

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

const float background_speed = 50.0, background_acceleration = 2.0, ground_speed = 200.0, ground_acceleration = 8.0, initial_speed_y = 990.0, initial_acceleration_y = -2200.0;
float i_background, i_ground, i_jump, actual_speed_y, actual_acceleration_y, sprite_time;
const int obstacle_distance = 400, stage_time = 15, sonic_radius = 37, max_time = 140000;
int stage, sonic_sprite, sprites_per_second, i, timeToIgnore, last_time_star = 0, time_to_special = 5, time_list[] = {10, 20, 30, 40};
bool is_playing, is_crashing, is_jumping, is_dropping, fast_drop, low_drop, changed_background, changed_ground, changed_sonic, changed_score, changed_record, star_passing = false;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Rect background_body = {0, 0, 1280, 330}, ground_body = {0, 330, 1280, 150}, sonic_body = {80, 235, 82, 100}, star_body = {WINDOW_WIDTH, 50, 30, 30};
SDL_Rect_Chained *obstacles_list;
SDL_Texture *background, *ground, *sonic[SONIC_AMOUNT], *obstacle_texture, *star;
Mix_Music *bgm;
Mix_Chunk *sound_jump, *sound_crash;
Score score, record;
const SDL_Color black_color = {50, 50, 50};
const SDL_Color gold_color = {255, 255, 0};
SDL_Surface *record_surface;
SDL_Texture *record_texture;

void initEverything()
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Sonic, jump!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
	background = IMG_LoadTexture(renderer, "../res/img/background.jpg");
	ground = IMG_LoadTexture(renderer, "../res/img/ground.jpg");
	obstacle_texture = IMG_LoadTexture(renderer, "../res/img/obstacle.png");
	star = IMG_LoadTexture(renderer, "../res/img/star.png");
	int i;
	for (i = 0; i < SONIC_AMOUNT; i++)
	{
		char sonic_path[20];
		sprintf(sonic_path, "../res/img/sonic_%02d.png", i + 1);
		sonic[i] = IMG_LoadTexture(renderer, sonic_path);
	}
	SDL_Texture * sonic_begin = IMG_LoadTexture(renderer, "../res/img/sonic_begin.png");
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	bgm = Mix_LoadMUS("../res/audio/sonic_theme.mp3");
	sound_jump = Mix_LoadWAV("../res/audio/jump.wav");
	sound_crash = Mix_LoadWAV("../res/audio/crash.wav");
	Mix_VolumeChunk(sound_jump, 30);
	Mix_VolumeChunk(sound_crash, 30);
	Mix_VolumeMusic(15);
	Mix_PlayMusic(bgm, -1);
	TTF_Init();
	score.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	score.color = black_color;
	score.body.h = 40;
	score.body.y = WINDOW_HEIGHT - score.body.h - SCORE_VERTICAL_MARGIN;
	
	FILE *fp = fopen(RECORD_FILE_NAME, "a+");
	if (!fp) return;
	fscanf(fp, "%s", record.score_str);
	fclose(fp);
	record.score = atoi(record.score_str);
	record.font = score.font;
	record.color = gold_color;
	record.body.w = 20 * strlen(record.score_str);
	record.body.h = 20;
	record.body.x = 0 + SCORE_HORIZONTAL_MARGIN;
	record.body.y = WINDOW_HEIGHT - record.body.h - SCORE_HORIZONTAL_MARGIN;
	record_surface = TTF_RenderText_Solid(record.font, record.score_str, record.color);	
	record_texture = SDL_CreateTextureFromSurface(renderer, record_surface);
	changed_record = true;
}

void beginGame()
{
	SDL_Rect_Chained *temp = obstacles_list;
	while (temp != NULL)
	{
		obstacles_list = obstacles_list->next;
		free(temp);
		temp = obstacles_list;
	}
	obstacles_list = (SDL_Rect_Chained *)malloc(sizeof(SDL_Rect_Chained));
	obstacles_list->next = NULL;
	(obstacles_list->body).x = 640;
	(obstacles_list->body).y = 255;
	(obstacles_list->body).w = 60;
	(obstacles_list->body).h = 80;
	Mix_VolumeMusic(35);
	i_background = 0.0;
	i_ground = 0.0;
	i_jump = 0.0;
	actual_speed_y = 0.0;
	actual_acceleration_y = 0.0;
	sprite_time = 0.0;
	stage = 1;
	sonic_sprite = 0;
	sprites_per_second = 15;
	i = 0;
	changed_sonic = true;
	sonic_body.w = 82;
	sonic_body.h = 100;
	sonic_body.y = 235;
	is_jumping = false;
	is_dropping = false;
	fast_drop = false;
	low_drop = false;
	is_playing = true;
	timeToIgnore = SDL_GetTicks();
}

void resetScreen()
{
	is_crashing = false;
	FILE *fp = fopen(RECORD_FILE_NAME, "a+");
	if (!fp) return;
	fscanf(fp, "%s", record.score_str);
	fclose(fp);
	record.score = atoi(record.score_str);
	record.font = score.font;
	record.color = gold_color;
	record.body.w = 20 * strlen(record.score_str);
	record.body.h = 20;
	record.body.x = 0 + SCORE_HORIZONTAL_MARGIN;
	record.body.y = WINDOW_HEIGHT - record.body.h - SCORE_HORIZONTAL_MARGIN;
	record_surface = TTF_RenderText_Solid(record.font, record.score_str, record.color);	
	record_texture = SDL_CreateTextureFromSurface(renderer, record_surface);
	changed_record = true;
}

void onExit()
{
	free(background);
	free(ground);
	free(obstacle_texture);
	free(star);
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

void treatKeyboardInput(const Uint8 *state)
{
	if ((state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN]) && !is_playing && !is_crashing)
	{
		beginGame();
	}
	else if (state[SDL_SCANCODE_UP] && !state[SDL_SCANCODE_DOWN] && is_playing)
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
	else if (state[SDL_SCANCODE_DOWN] && is_jumping && is_playing)
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

void on_sonic_crash()
{
	Mix_PlayChannel(-1, sound_crash, 0);
	Mix_VolumeMusic(15);
	if (score.score > record.score)
	{
		FILE* fp = fopen(RECORD_FILE_NAME, "w");
		if (fp)
		{
			fprintf(fp, "%s", score.score_str);
			fclose(fp);
		}
	}
	finishTimeout(2500);
	is_playing = false;
	is_crashing = true;
}

bool is_colliding(int x, int y)
{
	if (is_jumping && x <= 162 && x + 60 >= 102)
	{
		int b, c, delta;

		/* Reta x = 80 */
		b = -2 * sonic_body.y - 86;
		c = x * x - 246*x + sonic_body.y * sonic_body.y + 86 * sonic_body.y + 15609;
		delta = b * b - 4 * c;
		if (delta >= 0)
		{
			float y1, y2;
			y1 = (-b + sqrt(delta)) / 2;
			y2 = (-b - sqrt(delta)) / 2;
			if ((y1 >= y && y1 <= y+15) || (y2 >= y && y2 <= y+15))
			{
				return true;
			}
		}

		/* Reta y = 80 */
		b = -246;
		c = y * y - 2 * y * (sonic_body.y + 43) + sonic_body.y * sonic_body.y + 86 * sonic_body.y + 15609;
		delta =  b * b - 4 * c;
		if (delta >= 0)
		{
			float x1, x2;
			x1 = (-b + sqrt(delta)) / 2;
			x2 = (-b - sqrt(delta)) / 2;
			if ((x1 >= x && x1 <= x+60) || (x2 >= x && x2 <= x+60))
			{
				return true;
			}
		}
	}
	else if (x + 4 <= 162 && x + 56 >= 162)
	{
		return true;
	}
	return false;
}

void update(Uint32 dt, Uint32 time)
{
	/* VERIFICA COLISÃO ENTRE SONIC E O PRIMEIRO OBSTÁCULO DA LISTA */
	if(is_playing)
	{
		SDL_Rect_Chained *first_obstacle;
		first_obstacle = obstacles_list;
		
		if (is_colliding((first_obstacle->body).x, (first_obstacle->body).y))
		{
			on_sonic_crash();
			return;
		}
	}

	/* ATUALIZA POSIÇÃO Y DO SONIC NO PULO */
	if (is_jumping && is_playing)
	{
		i_jump += actual_speed_y * (dt / 1000.0);
		actual_speed_y += actual_acceleration_y * (dt / 1000.0);

		if (actual_speed_y < 0)
			is_dropping = true;

		if (i_jump > 1 || i_jump < -1)
		{
			sonic_body.y -= (int)i_jump;
			i_jump -= (int)i_jump;
			changed_sonic = true;
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
	if(is_playing)
	{
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
	}

	/* DESALOCA OBSTÁCULO QUANDO ELE SAI DA TELA */
	if (is_playing)
	{
		if ((obstacles_list->body).x < -60)
		{
			SDL_Rect_Chained *obstacle_to_destroy;
			obstacle_to_destroy = obstacles_list;
			obstacles_list = obstacles_list->next;
			free(obstacle_to_destroy);
		}
	}

	/* DECIDE SE ESTRELA DO ESPECIAL VAI APARECER */
	if (round((time - last_time_star)/1000.0) >= time_to_special)
	{
		star_passing = true;
		last_time_star = time;
		time_to_special = time_list[rand() % 4];
		printf("APARECE ESTRELA: %d\n", time);
	}

	/* ATUALIZA POSIÇÃO X DO PLANO DE FUNDO, CHÃO E OBSTÁCULOS */
	if (!is_crashing)
	{
		if (is_playing)
		{
			i_ground += (ground_speed + ground_acceleration * (min((time-timeToIgnore), max_time) / 1000)) * (dt / 1000.0);
			i_background += (background_speed + background_acceleration * (min((time-timeToIgnore), max_time) / 1000)) * (dt / 1000.0);
		}
		else
		{
			i_ground += ground_speed * (dt / 1000.0);
			i_background += background_speed * (dt / 1000.0);
		}
		if (ground_body.x <= -WINDOW_WIDTH)
		{
			ground_body.x = 0;
		}
		if (background_body.x <= -WINDOW_WIDTH)
		{
			background_body.x = 0;
		}
		if (i_ground >= 1)
		{
			if (is_playing)
			{
				SDL_Rect_Chained *temp = obstacles_list;
				while (temp != NULL)
				{
					(temp->body).x -= (int)i_ground;
					temp = temp->next;
				}
			}
			ground_body.x -= (int)i_ground;
			i_ground -= (int)i_ground;
			changed_ground = true;
		}
		if (i_background >= 1)
		{
			background_body.x -= (int)i_background;
			i_background -= (int)i_background;
			changed_background = true;
		}
	}
	if (star_passing && time <= max_time)
	{
		printf("%d - %d\n", star_body.x, star_body.w);
		if (star_body.x <= -star_body.w)
		{
			star_body.x = WINDOW_WIDTH;
			star_passing = false;
		}
		else
		{
			star_body.x -= (int)i_ground;
		}
	}

	/* NOVO BLOCO DE SPRITES QUANDO TEMPO DE JOGO > 10 SEGUNDOS */
	if (is_playing)
	{
		if ((time-timeToIgnore) > stage_time * 1000 && stage == 1)
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
			changed_sonic = true;
		}
	}

	/* ATUALIZA SCORE */
	if (is_playing)
	{
		if (score.score != (uint32_t)((time-timeToIgnore)/163))
		{
			score.score = (uint32_t)((time-timeToIgnore)/163);
			sprintf(score.score_str, "%u", score.score);
			score.body.w = 40 * strlen(score.score_str);
			score.body.x = WINDOW_WIDTH - score.body.w - SCORE_HORIZONTAL_MARGIN;
			changed_score = true;
		}
	}
}

void draw()
{
	if (changed_background || changed_ground || changed_sonic || changed_score || changed_record)
	{
		SDL_RenderCopy(renderer, background, NULL, &background_body);
		SDL_RenderCopy(renderer, ground, NULL, &ground_body);
		if (is_playing || is_crashing)
		{
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
			SDL_FreeSurface(score_surface);
			SDL_DestroyTexture(score_texture);
		}
		SDL_Surface *record_surface = TTF_RenderText_Solid(record.font, record.score_str, record.color);	
		SDL_Texture *record_texture = SDL_CreateTextureFromSurface(renderer, record_surface);
		SDL_RenderCopy(renderer, record_texture, NULL, &record.body);
		SDL_FreeSurface(record_surface);
		SDL_DestroyTexture(record_texture);
		SDL_RenderPresent(renderer);
		changed_background = false;
		changed_ground = false;
		changed_sonic = false;
		changed_score = false;
		changed_record = false;
	}
}
