#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include "game.h"

int main()
{
	load();
	
	Uint32 dt;
	Uint32 time = SDL_GetTicks();

	int frameTime = (int)(1000/FPS);
	int frameTimeOffset = 0;
	while(1)
	{
		SDL_Event e;
		Uint32 timeBeforeEvent = SDL_GetTicks();
		int hasEvent = SDL_WaitEventTimeout(&e, frameTime - frameTimeOffset);
		Uint32 timeWaitingEvent = SDL_GetTicks() - timeBeforeEvent;
		// if (timeWaitingEvent) printf("t %d\n", timeWaitingEvent);
		// else printf("============================== aquii ================================\n");
		if (hasEvent) {
			if(e.type == SDL_QUIT)
			{
				onExit(&e);
				return 0;
			}
			else if(e.type == SDL_KEYDOWN)
			{
				onKeyDown(&e);
			}
			else
			{
				frameTimeOffset += timeWaitingEvent;
				if (frameTimeOffset < frameTime) continue;
			}
		}
		frameTimeOffset = 0;

		dt = SDL_GetTicks() - time;
		time = SDL_GetTicks();

		update(dt);
		draw();
	}
}
