/*
----Universidade do Estado do Rio de Janeiro--------------------
----Sistemas Reativos-------------------------------------------
----Mini-projeto: Sonic, jump!----------------------------------
----Autores: Lucas Alves de Sousa & Yago Gomes Tomé de Sousa----
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

void initEverything();
void beginGame();
void resetScreen();
void onExit();
void onKeyDown(const SDL_Event *event);
void onKeyUp(const SDL_Event *event);
void update(Uint32 dt, Uint32 time);
void draw();
void finishTimeout(int timeout);
void treatKeyboardInput(const Uint8 *state);