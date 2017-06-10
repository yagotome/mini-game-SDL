#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int main()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("Sonic, jump!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Event e;

	SDL_Texture *ground, *background;
	SDL_Rect rectGround = {0, 330, 1280, 150}, rectBackground = {0, 0, 1280, 330};	

	background = IMG_LoadTexture(renderer, "../img/background.jpg");
	ground = IMG_LoadTexture(renderer, "../img/ground.jpg");

	int gSpeed = 3, bgSpeed = 1;
	int dt, time = SDL_GetTicks();

	while(1)
	{
		SDL_WaitEventTimeout(&e, 1);
		dt = SDL_GetTicks() - time;
		time = SDL_GetTicks();

		if(e.type == SDL_QUIT)
		{
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 0;
		}
		
		if(rectBackground.x < -640)
			rectBackground.x = 0;
		else
			rectBackground.x -= bgSpeed;

		
		if(rectGround.x < -640)
			rectGround.x = 0;
		else
			rectGround.x -= gSpeed;

		SDL_RenderCopy(renderer, background, NULL, &rectBackground);
		SDL_RenderCopy(renderer, ground, NULL, &rectGround);

		SDL_RenderPresent(renderer);
	}
}
