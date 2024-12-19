#include "camera.h"

SDL_FRect camera{4 * CHUNK_PIXEL_WIDTH, 4 * CHUNK_PIXEL_WIDTH, SCREEN_WIDTH, SCREEN_HEIGHT};

void do_camera_move(int ox, int oy, const Uint8 *keystate, const float dt)
{
    const float cam_base_vel = 0.75f;

    float cam_vel_x = 0;
    float cam_vel_y = 0;

    // CAMERA FREE-MOVE
    if (keystate[SDL_SCANCODE_LEFT] == 1)
        cam_vel_x = -cam_base_vel;
    if (keystate[SDL_SCANCODE_RIGHT] == 1)
        cam_vel_x = cam_base_vel;
    if (keystate[SDL_SCANCODE_UP] == 1)
        cam_vel_y = -cam_base_vel;
    if (keystate[SDL_SCANCODE_DOWN] == 1)
        cam_vel_y = cam_base_vel;

    if (camera.x + cam_vel_x <= WORLD_CHUNK_W * CHUNK_SIZE * BLOCK_SIZE)
    {
        camera.x += cam_vel_x * dt;
    }
    if (camera.y + cam_vel_y <= WORLD_CHUNK_H * CHUNK_SIZE * BLOCK_SIZE - SCREEN_HEIGHT && camera.y + cam_vel_y >= 0)
    {
        camera.y += cam_vel_y * dt;
    }

    SDL_FRect player_rect{ox - camera.x, oy - camera.y, PLAYER_WIDTH, PLAYER_HEIGHT};

    camera.x += (ox - camera.x - SCREEN_WIDTH / 2) * dt * 0.005f;
    camera.y += (oy - camera.y - SCREEN_HEIGHT / 2) * dt * 0.005f;
}
