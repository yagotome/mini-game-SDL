/*
----Universidade do Estado do Rio de Janeiro--------------------
----Sistemas Reativos-------------------------------------------
----Mini-projeto: Sonic, jump!----------------------------------
----Autores: Lucas Alves de Sousa & Yago Gomes Tomé de Sousa----
*/

#include <SDL2/SDL.h>
#include "game.h"

int timeToFinish = 0;

void finishTimeout(int timeout) {
	timeToFinish = SDL_GetTicks() + timeout;
}

int main()
{
	load();

	Uint32 dt;
	Uint32 time = SDL_GetTicks();

	while (1)
	{
		if (timeToFinish && time >= timeToFinish)
		{
			onExit();
			break;
		}

		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			switch (event.type) 
			{
				case SDL_QUIT:
					onExit();
					goto end_game_loop;
				case SDL_KEYDOWN:
					onKeyDown(&event);
					break;
				case SDL_KEYUP:
					onKeyUp(&event);
					break;
			}
		}

		dt = SDL_GetTicks() - time;
		time = SDL_GetTicks();

		update(dt, time);
		draw();
	}
	end_game_loop: return 0;
}