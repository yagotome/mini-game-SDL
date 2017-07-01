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
#include <math.h>
#include <time.h>
#include "engine/game.h"

#define SONIC_AMOUNT 19
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define SCORE_HORIZONTAL_MARGIN 16
#define SCORE_VERTICAL_MARGIN 10
#define RECORD_FILE_NAME "record.txt"
#define MIN(a, b) (a < b ? a : b)
#define SQR(x) ((x)*(x))
#define TIME_LIST_LEN 10

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
	char value_str[11]; /* length of max number of uint32_t */
	TTF_Font *font;
	SDL_Color color;
	SDL_Rect body;
} Text;

const float background_speed = 50.0, background_acceleration = 2.0, ground_speed = 200.0, ground_acceleration = 8.0, initial_speed_y = 990.0, initial_acceleration_y = -2200.0;
float i_background, i_ground, i_jump, actual_speed_y, actual_acceleration_y, sprite_time, game_speed;
const int obstacle_distance = 400, stage_time = 15, sonic_radius = 37, max_time = 140000;
int stage, sonic_sprite, sprites_per_second, i, timeToIgnore, time_to_star_appears, time_list[TIME_LIST_LEN];
bool is_playing, is_crashing, is_jumping, is_dropping, fast_drop, low_drop, changed_background, changed_ground, changed_sonic, changed_score, changed_record, changed_star, star_passing, initial_screen, gameover_screen, is_special, star_hit, sonic_blink;
Uint32 last_time_star, special_timeout;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Rect background_body = {0, 0, 1280, 330}, ground_body = {0, 330, 1280, 150}, sonic_body = {80, 235, 82, 100}, star_body = {WINDOW_WIDTH, 50, 40, 40};
SDL_Rect_Chained *obstacles_list;
SDL_Texture *background, *ground, *sonic[SONIC_AMOUNT], *obstacle_texture, *star;
Mix_Music *bgm;
Mix_Chunk *sound_jump, *sound_crash;
Text score, record, sonic_text, jump_text, presskey_text, upordown_text, toplay_text, game_text, over_text, toreset_text;
const SDL_Color grey_color = {50, 50, 50};
const SDL_Color green_color = {0, 255, 0};
const SDL_Color blue_color = {0, 0, 255};
const SDL_Color red_color = {255, 0, 0};
const SDL_Color yellow_color = {255, 255, 0};
SDL_Surface *record_surface;
SDL_Texture *record_texture;
SDL_Texture *sonic_begin;
SDL_Texture *sonic_gameover;

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
	sonic_begin = IMG_LoadTexture(renderer, "../res/img/sonic_begin.png");
	sonic_gameover = IMG_LoadTexture(renderer, "../res/img/sonic_gameover.png");
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
	score.color = grey_color;
	score.body.h = 40;
	score.body.y = WINDOW_HEIGHT - score.body.h - SCORE_VERTICAL_MARGIN;

	sonic_text.font = TTF_OpenFont("../res/font/courier.ttf", 40);
	sonic_text.color = blue_color;
	sprintf(sonic_text.value_str, "Sonic,");
	sonic_text.body.h = 40;
	sonic_text.body.y = 130;
	sonic_text.body.w = 240;
	sonic_text.body.x = 260;

	jump_text.font = TTF_OpenFont("../res/font/courier.ttf", 40);
	jump_text.color = red_color;
	sprintf(jump_text.value_str, "jump!");
	jump_text.body.h = 40;
	jump_text.body.y = 180;
	jump_text.body.w = 200;
	jump_text.body.x = 400;

	presskey_text.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	presskey_text.color = grey_color;
	sprintf(presskey_text.value_str, "Press key");
	presskey_text.body.h = 40;
	presskey_text.body.y = 260;
	presskey_text.body.w = 120;
	presskey_text.body.x = 250;

	upordown_text.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	upordown_text.color = grey_color;
	sprintf(upordown_text.value_str, "UP or DOWN");
	upordown_text.body.h = 40;
	upordown_text.body.y = 260;
	upordown_text.body.w = 120;
	upordown_text.body.x = 380;

	toplay_text.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	toplay_text.color = grey_color;
	sprintf(toplay_text.value_str, "to play");
	toplay_text.body.h = 40;
	toplay_text.body.y = 260;
	toplay_text.body.w = 84;
	toplay_text.body.x = 510;

	initial_screen = true;
	gameover_screen = false;

	FILE *fp = fopen(RECORD_FILE_NAME, "a+");
	if (!fp)
		return;
	fscanf(fp, "Best: %s", record.value_str);
	fclose(fp);
	record.score = atoi(record.value_str);
	i = 0;
	do
	{
		record.value_str[i + 5] = record.value_str[i];
	} while (record.value_str[i++] != '\0');
	record.value_str[0] = 'B';
	record.value_str[1] = 'e';
	record.value_str[2] = 's';
	record.value_str[3] = 't';
	record.value_str[4] = ':';
	if (i == 1)
	{
		record.value_str[5] = '0';
		record.value_str[6] = '\0';
	}
	record.font = score.font;
	record.color = yellow_color;
	record.body.w = 20 * strlen(record.value_str);
	record.body.h = 20;
	record.body.x = 0 + SCORE_HORIZONTAL_MARGIN;
	record.body.y = WINDOW_HEIGHT - record.body.h - SCORE_HORIZONTAL_MARGIN;
	record_surface = TTF_RenderText_Solid(record.font, record.value_str, record.color);
	record_texture = SDL_CreateTextureFromSurface(renderer, record_surface);
	changed_record = true;

	for (i = 0; i < TIME_LIST_LEN; i++)
	{
		time_list[i] = (i + 5) * 3;
	}
}

void beginGame()
{
	initial_screen = false;
	gameover_screen = false;
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
	game_speed = 1.0;
	stage = 1;
	sonic_sprite = 0;
	sprites_per_second = 15;
	i = 0;
	time_to_star_appears = time_list[rand() % TIME_LIST_LEN];
	changed_sonic = true;
	sonic_body.w = 82;
	sonic_body.h = 100;
	sonic_body.y = 235;
	is_jumping = false;
	is_dropping = false;
	fast_drop = false;
	low_drop = false;
	is_playing = true;
	star_passing = false;
	is_special = false;
	star_hit = false;
	sonic_blink = false;
	timeToIgnore = SDL_GetTicks();
}

void resetScreen()
{
	gameover_screen = true;
	is_crashing = false;
	star_passing = false;
	changed_star = false;
	star_hit = false;
	star_body.x = WINDOW_WIDTH;

	game_text.font = TTF_OpenFont("../res/font/courier.ttf", 40);
	game_text.color = grey_color;
	sprintf(game_text.value_str, "Game");
	game_text.body.h = 40;
	game_text.body.y = 130;
	game_text.body.w = 160;
	game_text.body.x = 260;

	over_text.font = TTF_OpenFont("../res/font/courier.ttf", 40);
	over_text.color = grey_color;
	sprintf(over_text.value_str, "Over! =(");
	over_text.body.h = 40;
	over_text.body.y = 180;
	over_text.body.w = 200;
	over_text.body.x = 370;

	presskey_text.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	presskey_text.color = grey_color;
	sprintf(presskey_text.value_str, "Press key");
	presskey_text.body.h = 40;
	presskey_text.body.y = 260;
	presskey_text.body.w = 120;
	presskey_text.body.x = 250;

	upordown_text.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	upordown_text.color = grey_color;
	sprintf(upordown_text.value_str, "UP or DOWN");
	upordown_text.body.h = 40;
	upordown_text.body.y = 260;
	upordown_text.body.w = 120;
	upordown_text.body.x = 380;

	toreset_text.font = TTF_OpenFont("../res/font/courier.ttf", 24);
	toreset_text.color = grey_color;
	sprintf(toreset_text.value_str, "to reset");
	toreset_text.body.h = 40;
	toreset_text.body.y = 260;
	toreset_text.body.w = 84;
	toreset_text.body.x = 510;

	FILE *fp = fopen(RECORD_FILE_NAME, "a+");
	if (!fp)
		return;
	fscanf(fp, "Best: %s", record.value_str);
	fclose(fp);
	record.score = atoi(record.value_str);
	int i = 0;
	do
	{
		record.value_str[i + 5] = record.value_str[i];
	} while (record.value_str[i++] != '\0');
	record.value_str[0] = 'B';
	record.value_str[1] = 'e';
	record.value_str[2] = 's';
	record.value_str[3] = 't';
	record.value_str[4] = ':';
	record.font = score.font;
	record.color = yellow_color;
	record.body.w = 20 * strlen(record.value_str);
	record.body.h = 20;
	record.body.x = 0 + SCORE_HORIZONTAL_MARGIN;
	record.body.y = WINDOW_HEIGHT - record.body.h - SCORE_HORIZONTAL_MARGIN;
	record_surface = TTF_RenderText_Solid(record.font, record.value_str, record.color);
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
		FILE *fp = fopen(RECORD_FILE_NAME, "w");
		if (fp)
		{
			fprintf(fp, "Best: %s", score.value_str);
			fclose(fp);
		}
	}
	finishTimeout(1500);
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
		c = x * x - 246 * x + sonic_body.y * sonic_body.y + 86 * sonic_body.y + 15609;
		delta = b * b - 4 * c;
		if (delta >= 0)
		{
			float y1, y2;
			y1 = (-b + sqrt(delta)) / 2;
			y2 = (-b - sqrt(delta)) / 2;
			if ((y1 >= y && y1 <= y + 15) || (y2 >= y && y2 <= y + 15))
			{
				return true;
			}
		}

		/* Reta y = 80 */
		b = -246;
		c = y * y - 2 * y * (sonic_body.y + 43) + sonic_body.y * sonic_body.y + 86 * sonic_body.y + 15609;
		delta = b * b - 4 * c;
		if (delta >= 0)
		{
			float x1, x2;
			x1 = (-b + sqrt(delta)) / 2;
			x2 = (-b - sqrt(delta)) / 2;
			if ((x1 >= x && x1 <= x + 60) || (x2 >= x && x2 <= x + 60))
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

bool is_colliding_circles(const SDL_Rect *rect1, const SDL_Rect *rect2)
{	
	typedef struct
	{
		int x;
		int y;
	} Point;
	typedef struct
	{
		Point center;
		int r;
	} Circle;
	Circle c1, c2;
	c1.center.x = (int)round((2*rect1->x + rect1->w)/2.);
	c1.center.y = (int)round((2*rect1->y + rect1->h)/2.);
	c1.r = (int)round(rect1->w/2.);
	
	c2.center.x = (int)round((2*rect2->x + rect2->w)/2.);
	c2.center.y = (int)round((2*rect2->y + rect2->h)/2.);
	c2.r = (int)round(rect2->w/2.);

	Circle *cg, *cl;
	if (c1.r > c2.r)
	{
		cg = &c1;
		cl = &c2;
	}
	else
	{		
		cg = &c2;
		cl = &c1;
	}

	int theta;
	for (theta = 0; theta < 360; theta++)
	{
		int xbl = (int)round(cl->center.x + cl->r * cos(theta)); // x of border of lower circle
		int ybl = (int)round(cl->center.y + cl->r * sin(theta)); // y of border of lower circle

		int d = sqrt(SQR(cg->center.x - cl->center.x) + SQR(cg->center.y - cl->center.y));
		if (d <= cg->r)
			return true;
	}
	return false;
}

void update(Uint32 dt, Uint32 time)
{
	if (is_playing)
	{
		/* VERIFICA COLISÃO ENTRE SONIC E O PRIMEIRO OBSTÁCULO DA LISTA */
		SDL_Rect_Chained *first_obstacle;
		first_obstacle = obstacles_list;

		if (!is_special && is_colliding((first_obstacle->body).x, (first_obstacle->body).y))
		{
			on_sonic_crash();
			return;
		}

		/* TRATA FIM DO ESPECIAL: DESACELERA O JOGO E QUANDO ESTIVER QUASE ACABANDO SONIC VOLTA A CORRER NORMALMENTE */
		if (is_special && time >= special_timeout - 3000)
		{
			Uint32 diff = special_timeout - time;
			if (diff <= 1000)
			{
				game_speed = 1.0;
				stage = 2;
			}
			else 
				game_speed = diff / 1000.0;
			if (diff <= 0)
			{
				is_special = false;
				sonic_body.w = 82;
				sonic_body.h = 100;
				sonic_body.y = 235;
			}
		}

		/* VERIFICA SE SONIC PEGOU A ESTRELA DO ESPECIAL */
		if (is_jumping && is_colliding_circles(&sonic_body, &star_body))
		{
			is_special = true;
			special_timeout = time + 5000 + 1000*(rand() % 7);
			game_speed = 4.0;
			star_hit = true;
			stage = 3;
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
	if (is_playing)
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
	if (!is_playing)
	{
		last_time_star = time;
	}
	else if (round((time - last_time_star) / 1000.0) >= time_to_star_appears)
	{
		star_passing = true;
		last_time_star = time;
		time_to_star_appears = time_list[rand() % TIME_LIST_LEN];
	}

	/* ATUALIZA POSIÇÃO X DO PLANO DE FUNDO, CHÃO E OBSTÁCULOS */
	if (!is_crashing)
	{
		if (is_playing)
		{
			i_ground += (ground_speed + ground_acceleration * (MIN((time - timeToIgnore), max_time) / 1000)) * (dt / 1000.0) * game_speed;
			i_background += (background_speed + background_acceleration * (MIN((time - timeToIgnore), max_time) / 1000)) * (dt / 1000.0) * game_speed;
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
				if(star_passing)
				{
					if (star_hit) star_body.x = -star_body.w;
					if (star_body.x <= -star_body.w)
					{
						star_body.x = WINDOW_WIDTH;
						star_passing = false;
						changed_star = false;
						star_hit = false;
					}
					else
					{
						star_body.x -= (int)i_ground;
						changed_star = true;
					}
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

	/* NOVO BLOCO DE SPRITES QUANDO TEMPO DE JOGO > 10 SEGUNDOS */
	if (is_playing)
	{
		if ((time - timeToIgnore) > stage_time * 1000 && stage == 1)
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
					case 3:
						sonic_sprite = 12 + ((sonic_sprite + 3) % 7);
						sprites_per_second = 35;
						sonic_body.w = 80;
						sonic_body.h = 80;
						sonic_body.y = 255;
						break;
				}
			}
			sprite_time = 0;
			changed_sonic = true;
			if (is_special) sonic_blink = !sonic_blink;
		}
	}

	/* ATUALIZA SCORE */
	if (is_playing)
	{
		if (score.score != (uint32_t)((time - timeToIgnore) / 163))
		{
			score.score = (uint32_t)((time - timeToIgnore) / 163);
			sprintf(score.value_str, "%u", score.score);
			score.body.w = 40 * strlen(score.value_str);
			score.body.x = WINDOW_WIDTH - score.body.w - SCORE_HORIZONTAL_MARGIN;
			changed_score = true;
		}
	}
}

void draw()
{
	if (changed_background || changed_ground || changed_sonic || changed_score || changed_record || changed_star)
	{
		SDL_RenderCopy(renderer, background, NULL, &background_body);
		SDL_RenderCopy(renderer, ground, NULL, &ground_body);
		SDL_RenderCopy(renderer, star, NULL, &star_body);
		if (is_playing || is_crashing)
		{
			if (!is_special || !sonic_blink)
				SDL_RenderCopy(renderer, sonic[sonic_sprite], NULL, &sonic_body);
			SDL_Rect_Chained *obstacle = obstacles_list;
			while (obstacle != NULL)
			{
				SDL_RenderCopy(renderer, obstacle_texture, NULL, &(obstacle->body));
				obstacle = obstacle->next;
			}
			SDL_Surface *score_surface = TTF_RenderText_Solid(score.font, score.value_str, score.color);
			SDL_Texture *score_texture = SDL_CreateTextureFromSurface(renderer, score_surface);
			SDL_RenderCopy(renderer, score_texture, NULL, &score.body);
			SDL_FreeSurface(score_surface);
			SDL_DestroyTexture(score_texture);
		}
		SDL_Surface *record_surface = TTF_RenderText_Solid(record.font, record.value_str, record.color);
		SDL_Texture *record_texture = SDL_CreateTextureFromSurface(renderer, record_surface);
		SDL_RenderCopy(renderer, record_texture, NULL, &record.body);
		SDL_FreeSurface(record_surface);
		SDL_DestroyTexture(record_texture);
		if (initial_screen)
		{
			SDL_Rect begin_sonic = {80, 158, 140, 178};
			SDL_RenderCopy(renderer, sonic_begin, NULL, &begin_sonic);

			SDL_Surface *sonic_text_surface = TTF_RenderText_Solid(sonic_text.font, sonic_text.value_str, sonic_text.color);
			SDL_Texture *sonic_text_texture = SDL_CreateTextureFromSurface(renderer, sonic_text_surface);
			SDL_RenderCopy(renderer, sonic_text_texture, NULL, &sonic_text.body);
			SDL_FreeSurface(sonic_text_surface);
			SDL_DestroyTexture(sonic_text_texture);

			SDL_Surface *jump_text_surface = TTF_RenderText_Solid(jump_text.font, jump_text.value_str, jump_text.color);
			SDL_Texture *jump_text_texture = SDL_CreateTextureFromSurface(renderer, jump_text_surface);
			SDL_RenderCopy(renderer, jump_text_texture, NULL, &jump_text.body);
			SDL_FreeSurface(jump_text_surface);
			SDL_DestroyTexture(jump_text_texture);

			SDL_Surface *presskey_text_surface = TTF_RenderText_Solid(presskey_text.font, presskey_text.value_str, presskey_text.color);
			SDL_Texture *presskey_text_texture = SDL_CreateTextureFromSurface(renderer, presskey_text_surface);
			SDL_RenderCopy(renderer, presskey_text_texture, NULL, &presskey_text.body);
			SDL_FreeSurface(presskey_text_surface);
			SDL_DestroyTexture(presskey_text_texture);

			SDL_Surface *upordown_text_surface = TTF_RenderText_Solid(upordown_text.font, upordown_text.value_str, upordown_text.color);
			SDL_Texture *upordown_text_texture = SDL_CreateTextureFromSurface(renderer, upordown_text_surface);
			SDL_RenderCopy(renderer, upordown_text_texture, NULL, &upordown_text.body);
			SDL_FreeSurface(upordown_text_surface);
			SDL_DestroyTexture(upordown_text_texture);

			SDL_Surface *toplay_text_surface = TTF_RenderText_Solid(toplay_text.font, toplay_text.value_str, toplay_text.color);
			SDL_Texture *toplay_text_texture = SDL_CreateTextureFromSurface(renderer, toplay_text_surface);
			SDL_RenderCopy(renderer, toplay_text_texture, NULL, &toplay_text.body);
			SDL_FreeSurface(toplay_text_surface);
			SDL_DestroyTexture(toplay_text_texture);
		}
		else if (gameover_screen)
		{
			SDL_Rect gameover_sonic = {80, 196, 140, 140};
			SDL_RenderCopy(renderer, sonic_gameover, NULL, &gameover_sonic);

			SDL_Surface *game_text_surface = TTF_RenderText_Solid(game_text.font, game_text.value_str, game_text.color);
			SDL_Texture *game_text_texture = SDL_CreateTextureFromSurface(renderer, game_text_surface);
			SDL_RenderCopy(renderer, game_text_texture, NULL, &game_text.body);
			SDL_FreeSurface(game_text_surface);
			SDL_DestroyTexture(game_text_texture);

			SDL_Surface *over_text_surface = TTF_RenderText_Solid(over_text.font, over_text.value_str, over_text.color);
			SDL_Texture *over_text_texture = SDL_CreateTextureFromSurface(renderer, over_text_surface);
			SDL_RenderCopy(renderer, over_text_texture, NULL, &over_text.body);
			SDL_FreeSurface(over_text_surface);
			SDL_DestroyTexture(over_text_texture);

			SDL_Surface *presskey_text_surface = TTF_RenderText_Solid(presskey_text.font, presskey_text.value_str, presskey_text.color);
			SDL_Texture *presskey_text_texture = SDL_CreateTextureFromSurface(renderer, presskey_text_surface);
			SDL_RenderCopy(renderer, presskey_text_texture, NULL, &presskey_text.body);
			SDL_FreeSurface(presskey_text_surface);
			SDL_DestroyTexture(presskey_text_texture);

			SDL_Surface *upordown_text_surface = TTF_RenderText_Solid(upordown_text.font, upordown_text.value_str, upordown_text.color);
			SDL_Texture *upordown_text_texture = SDL_CreateTextureFromSurface(renderer, upordown_text_surface);
			SDL_RenderCopy(renderer, upordown_text_texture, NULL, &upordown_text.body);
			SDL_FreeSurface(upordown_text_surface);
			SDL_DestroyTexture(upordown_text_texture);

			SDL_Surface *toreset_text_surface = TTF_RenderText_Solid(toreset_text.font, toreset_text.value_str, toreset_text.color);
			SDL_Texture *toreset_text_texture = SDL_CreateTextureFromSurface(renderer, toreset_text_surface);
			SDL_RenderCopy(renderer, toreset_text_texture, NULL, &toreset_text.body);
			SDL_FreeSurface(toreset_text_surface);
			SDL_DestroyTexture(toreset_text_texture);
		}
		SDL_RenderPresent(renderer);
		changed_background = false;
		changed_ground = false;
		changed_sonic = false;
		changed_score = false;
		changed_record = false;
		changed_star = false;
	}
}
