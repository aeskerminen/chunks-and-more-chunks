#ifndef CAM
#define CAM

#include <SDL2/SDL.h>
#include "globals.h"

extern SDL_FRect camera;

void do_camera_move(int ox, int oy, const Uint8 *keystate, const float dt);

#endif