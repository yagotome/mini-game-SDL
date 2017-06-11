#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#define FPS 30
#define SONIC_AMOUNT 19

void load();
void onExit(const SDL_Event *event);
void onKeyDown(const SDL_Event *event);
void update(const Uint32 dt);
void draw();