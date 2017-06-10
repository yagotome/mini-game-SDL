#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

int main()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("Sonic, jump!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Event e;

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	Mix_Music *bgm, *jump, *crash;
	bgm = Mix_LoadMUS("../mp3/sonic_theme.mp3");
	jump = Mix_LoadMUS("../mp3/jump.mp3");
	crash = Mix_LoadMUS("../mp3/crash.mp3");

	SDL_Texture *background, *ground, *obstacle, *sonic_01, *sonic_02, *sonic_03, *sonic_04, *sonic_05, *sonic_06, *sonic_07, *sonic_08, *sonic_09, *sonic_10, *sonic_11, *sonic_12, *sonic_13, *sonic_14, *sonic_15, *sonic_16, *sonic_17, *sonic_18, *sonic_19;
	SDL_Rect body_background = {0, 0, 1280, 330}, body_ground = {0, 330, 1280, 150}, body_sonic = {80, 235, 82, 100};	

	background = IMG_LoadTexture(renderer, "../img/background.jpg");
	ground = IMG_LoadTexture(renderer, "../img/ground.jpg");
	obstacle = IMG_LoadTexture(renderer, "../img/obstacle.jpg");
	sonic_01 = IMG_LoadTexture(renderer, "../img/sonic_01.png");
	sonic_02 = IMG_LoadTexture(renderer, "../img/sonic_02.png");
	sonic_03 = IMG_LoadTexture(renderer, "../img/sonic_03.png");
	sonic_04 = IMG_LoadTexture(renderer, "../img/sonic_04.png");
	sonic_05 = IMG_LoadTexture(renderer, "../img/sonic_05.png");
	sonic_06 = IMG_LoadTexture(renderer, "../img/sonic_06.png");
	sonic_07 = IMG_LoadTexture(renderer, "../img/sonic_07.png");
	sonic_08 = IMG_LoadTexture(renderer, "../img/sonic_08.png");

	int gSpeed = 3, bgSpeed = 1, stage = 1, aux_stage1 = 0, aux_stage2 = 0, aux_stage3 = 0;
	const int sprite_delay = 50;
	int dt, time, sprite_time = 0;

	time = SDL_GetTicks();
	Mix_PlayMusic(bgm, -1);

	while(1)
	{
		SDL_WaitEventTimeout(&e, 1);
		dt = SDL_GetTicks() - time;
		time = SDL_GetTicks();

		if(e.type == SDL_QUIT)
		{
			Mix_HaltMusic();
			Mix_FreeMusic(bgm);
			Mix_Quit();
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 0;
		}
	/*
		else if(e.type == TECLA PRA CIMA)
		{
			Mix_PlayMusic(jump, 0);
		}
	*/	
		
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
			Mix_PlayMusic(crash, 0);
			game_over();
		}
	*/

		SDL_RenderCopy(renderer, background, NULL, &body_background);
		SDL_RenderCopy(renderer, ground, NULL, &body_ground);

		if(time > 15000 && time < 30000 && stage == 1)
		{
			stage = 2;
			sonic_09 = IMG_LoadTexture(renderer, "../img/sonic_09.png");
			sonic_10 = IMG_LoadTexture(renderer, "../img/sonic_10.png");
			sonic_11 = IMG_LoadTexture(renderer, "../img/sonic_11.png");
			sonic_12 = IMG_LoadTexture(renderer, "../img/sonic_12.png");
		}
		else if(time > 30000 && stage == 2)
		{
			stage = 3;
			body_sonic.x = 82;
			body_sonic.y = 255;
			body_sonic.w = 80;
			body_sonic.h = 80;
			sonic_13 = IMG_LoadTexture(renderer, "../img/sonic_13.png");
			sonic_14 = IMG_LoadTexture(renderer, "../img/sonic_14.png");
			sonic_15 = IMG_LoadTexture(renderer, "../img/sonic_15.png");
			sonic_16 = IMG_LoadTexture(renderer, "../img/sonic_16.png");
			sonic_17 = IMG_LoadTexture(renderer, "../img/sonic_17.png");
			sonic_18 = IMG_LoadTexture(renderer, "../img/sonic_18.png");
			sonic_19 = IMG_LoadTexture(renderer, "../img/sonic_19.png");
		}
		
		sprite_time += dt;

		if(stage == 1)
		{
			if(sprite_time > sprite_delay)
			{
				aux_stage1 = (aux_stage1 + 1) % 7;
				sprite_time = 0;
			}

			if(aux_stage1 == 0) SDL_RenderCopy(renderer, sonic_01, NULL, &body_sonic);
			else if(aux_stage1 == 1) SDL_RenderCopy(renderer, sonic_02, NULL, &body_sonic);
			else if(aux_stage1 == 2) SDL_RenderCopy(renderer, sonic_03, NULL, &body_sonic);
			else if(aux_stage1 == 3) SDL_RenderCopy(renderer, sonic_04, NULL, &body_sonic);
			else if(aux_stage1 == 4) SDL_RenderCopy(renderer, sonic_05, NULL, &body_sonic);
			else if(aux_stage1 == 5) SDL_RenderCopy(renderer, sonic_06, NULL, &body_sonic);
			else if(aux_stage1 == 6) SDL_RenderCopy(renderer, sonic_07, NULL, &body_sonic);
			else if(aux_stage1 == 7) SDL_RenderCopy(renderer, sonic_08, NULL, &body_sonic);
		}
		else if(stage == 2)
		{
			if(sprite_time > sprite_delay)
			{
				aux_stage2 = (aux_stage2 + 1) % 3;
				sprite_time = 0;
			}

			if(aux_stage2 == 0) SDL_RenderCopy(renderer, sonic_09, NULL, &body_sonic);
			else if(aux_stage2 == 1) SDL_RenderCopy(renderer, sonic_10, NULL, &body_sonic);
			else if(aux_stage2 == 2) SDL_RenderCopy(renderer, sonic_11, NULL, &body_sonic);
			else if(aux_stage2 == 3) SDL_RenderCopy(renderer, sonic_12, NULL, &body_sonic);
		}
		else
		{
			if(sprite_time > sprite_delay)
			{
				aux_stage3 = (aux_stage3 + 1) % 6;
				sprite_time = 0;
			}

			if(aux_stage3 == 0) SDL_RenderCopy(renderer, sonic_13, NULL, &body_sonic);
			else if(aux_stage3 == 1) SDL_RenderCopy(renderer, sonic_14, NULL, &body_sonic);
			else if(aux_stage3 == 2) SDL_RenderCopy(renderer, sonic_15, NULL, &body_sonic);
			else if(aux_stage3 == 3) SDL_RenderCopy(renderer, sonic_16, NULL, &body_sonic);
			else if(aux_stage3 == 4) SDL_RenderCopy(renderer, sonic_17, NULL, &body_sonic);
			else if(aux_stage3 == 5) SDL_RenderCopy(renderer, sonic_18, NULL, &body_sonic);
			else if(aux_stage3 == 6) SDL_RenderCopy(renderer, sonic_19, NULL, &body_sonic);
		}

		SDL_RenderPresent(renderer);
	}
}
