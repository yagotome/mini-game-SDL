/*
----Universidade do Estado do Rio de Janeiro--------------------
----Sistemas Reativos-------------------------------------------
----Mini-projeto: Sonic, jump!----------------------------------
----Autores: Lucas Alves de Sousa & Yago Gomes Tomé de Sousa----
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "game.h"

int main()
{
	loadGame();

	Uint32 dt;

	updateGame(dt);

	return 0;
}

