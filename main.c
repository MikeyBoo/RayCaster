/* TODO: fix map / correct colorBuffer write access error 
seperate texture logic to textures.h / textures.c 
seperate wallPorjection logic to a wall.h / wall.c */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <SDL.h>
#include "constants.h"
#include "graphics.h"
#include <cfloat>
#include "map.h"
#include "player.h"
#include "ray.h"
#include "textures.h"

bool isGameRunning = false;
int ticksLastFrame;

uint32_t* textures[NUM_TEXTURES]; // should be defined in a textures.h

void setup() {
    // the following should be a loadWallTextures getter function from a textures.c
    textures[0] = (uint32_t*)REDBRICK_TEXTURE;
    textures[1] = (uint32_t*)PURPLESTONE_TEXTURE;
    textures[2] = (uint32_t*)MOSSYSTONE_TEXTURE;
    textures[3] = (uint32_t*)GRAYSTONE_TEXTURE;
    textures[4] = (uint32_t*)COLORSTONE_TEXTURE;
    textures[5] = (uint32_t*)BLUESTONE_TEXTURE;
    textures[6] = (uint32_t*)WOOD_TEXTURE;
    textures[7] = (uint32_t*)EAGLE_TEXTURE;
}

void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                isGameRunning = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    isGameRunning = false;
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    player.walkDirection = +1;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    player.turnDirection = -1;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    player.walkDirection = -1;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    player.turnDirection = +1;
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_W:
                case SDL_SCANCODE_UP:
                    player.walkDirection = 0;
                    break;
                case SDL_SCANCODE_A:
                case SDL_SCANCODE_LEFT:
                    player.turnDirection = 0;
                    break;
                case SDL_SCANCODE_S:
                case SDL_SCANCODE_DOWN:
                    player.walkDirection = 0;
                    break;
                case SDL_SCANCODE_D:
                case SDL_SCANCODE_RIGHT:
                    player.turnDirection = 0;
                    break;
                }
                break;
        }
    }
}

void update() {

    int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);
    if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH) {
        SDL_Delay(timeToWait);
    }
    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

    ticksLastFrame = SDL_GetTicks();

    movePlayer(deltaTime);
    castAllRays();
}

// should be moved to a wall.c - wall.h but textures.c should be developed first
void renderWallProjection(void) {
    for (int x = 0; x < NUM_RAYS; x++) {
        float perpDistance = rays[x].distance * cos(rays[x].rayAngle - player.rotationAngle);
        float projectedWallHeight = (TILE_SIZE / perpDistance) * DIST_PROJ_PLANE;

        int wallStripHeight = (int)projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel; 

        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        for (int y = 0; y < wallTopPixel; y++) {
            drawPixel(x, y, 0xFF333333);
        }
        int textureOffsetX;

        /////////finding the texture for wall strip/////// vertical across surface

        //TODO: calculate textureOffsetX
        if (rays[x].wasHitVertical) {
            // perform offset vertical on map
            textureOffsetX = (int)rays[x].wallHitY % TILE_SIZE;
        }
        else {
            // perform offset horizontal on map
            textureOffsetX = (int)rays[x].wallHitX % TILE_SIZE;
        }

        // get texture
        int texNum = rays[x].wallHitContent - 1;

        /////////finding horizontal element of wall strip////////// horizontal down surface

        // render the wall from wallTopPixel to wallBottomPixel
        for (int y = wallTopPixel; y < wallBottomPixel; y++) {
            // to correct  texture distortion due to line 404
            int distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);

            // calculate textureOffsetY
            int textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT / wallStripHeight);

            // set color of wall based on texture
            uint32_t texelColor = textures[texNum][(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
            drawPixel(x, y, texelColor);
        }

        // set the color of the floor
        for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++) {
            drawPixel(x, y, 0xFF777777);
        }
            
    }
}

void render() {
    clearColorBuffer(0xFF000000);

    renderWallProjection();

    renderMap(); 
    renderRays();
    renderPlayer();

    renderColorBuffer();
}

void releaseResources(void) {
    destroyWindow();
}

int main() {
    isGameRunning = initializeWindow();

    setup();

    while (isGameRunning) {
        processInput();
        update();
        render();
    }

    releaseResources(); 

    return 0;
}
