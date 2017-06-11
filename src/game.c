#include "engine/game.h"

SDL_Window *window;
SDL_Renderer *renderer;
Mix_Music *bgm;
Mix_Chunk *sound_jump, *sound_crash;
SDL_Rect 	body_background = {0, 0, 1280, 330},
			body_ground = {0, 330, 1280, 150},
			body_sonic = {80, 235, 82, 100};
SDL_Texture *background, *ground, *obstacle, *sonic[SONIC_AMOUNT];

int gSpeed = 3, bgSpeed = 1, stage = 1, sonic_sprite = 0;

void load() {
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
		sprintf(sonic_path, "../img/sonic_%02d.png", i+1);
		sonic[i] = IMG_LoadTexture(renderer, sonic_path);
	}
	
	Mix_PlayMusic(bgm, -1);
}

void onExit(const SDL_Event *e) {
	Mix_HaltMusic();
	Mix_FreeMusic(bgm);
	Mix_Quit();
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void onKeyDown(const SDL_Event *e) {
	if(e->key.keysym.sym == SDLK_UP && ((body_sonic.y == 235 && (stage == 1 || stage == 2)) || (body_sonic.y == 255 && stage == 3))) // Essa lógica só permite o pulo quando o sonic estiver com os pés no chão. Caso queiramos que ele pule quando esteja em cima de um obstáculo, essa lógica deve ser adaptada.
	{
		Mix_PlayChannel(-1, sound_jump, 0);
		//pulo aqui
	}
}

void update(const Uint32 dt) {
	// printf("dt %d\n", dt);
	if(body_background.x < -640)
		body_background.x = 0;
	else
		body_background.x -= bgSpeed; // Ainda não está acelerando

	
	if(body_ground.x < -640)
		body_ground.x = 0;
	else
		body_ground.x -= gSpeed; // Ainda não está acelerando

	/*
		else if(sonic COLIDIU COM obstacle)
		{
			Mix_PlayChannel(-1, crash, 0);
			game_over();
		}
	*/
	
	Uint32 time = SDL_GetTicks();
	if(time > 5000 && time < 10000 && stage == 1)
	{
		stage = 2;
	}
	else if(time > 10000 && stage == 2)
	{
		stage = 3;
		body_sonic.x = 82;
		body_sonic.y = 255;
		body_sonic.w = 80;
		body_sonic.h = 80;
	}
}

void draw() {
	SDL_RenderCopy(renderer, background, NULL, &body_background);
	SDL_RenderCopy(renderer, ground, NULL, &body_ground);
	switch(stage)
	{
		case 1:
			sonic_sprite = ((sonic_sprite + 1) % 8);
			break;
		case 2:
			sonic_sprite = 8 + ((sonic_sprite + 1) % 4);
			break;
		case 3:
			sonic_sprite = 12 + ((sonic_sprite + 1) % 7);
			break;
	}	
	SDL_RenderCopy(renderer, sonic[sonic_sprite], NULL, &body_sonic);
	SDL_RenderPresent(renderer);
}