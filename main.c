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

//struct Ray {
//    float rayAngle;
//    float wallHitX;
//    float wallHitY;
//    float distance;
//    bool wasHitVertical;
//    /*bool isRayFacingUp;
//    bool isRayFacingDown;
//    bool isRayFacingLeft;
//    bool isRayFacingRight;*/
//    int wallHitContent;
//} rays[NUM_RAYS];


// global variables
bool isGameRunning = false;
int ticksLastFrame;

//uint32_t* wallTexture = NULL; //procederal wall testure for testing
uint32_t* textures[NUM_TEXTURES];



void setup() {


    // procederal grid textures for testing
    //wallTexture = (uint32_t*)malloc(sizeof(uint32_t) * (uint32_t)TEXTURE_WIDTH * (uint32_t)TEXTURE_HEIGHT);
    //for (int x = 0; x < TEXTURE_WIDTH; x++) {
    //    for (int y = 0; y < TEXTURE_HEIGHT; y++) {
    //        wallTexture[(TEXTURE_WIDTH * y) + x] = (x % 8 && y % 8) ? 0xff0000ff : 0xFF000000; // makes grid
    //    }
    //}

    //TODO: load PNG
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
                //if (event.key.keysym.sym == SDLK_ESCAPE)
                //    isGameRunning = false;
                //if (event.key.keysym.sym == SDLK_w)
                //    player.walkDirection = +1;
                //if (event.key.keysym.sym == SDLK_s)
                //    player.walkDirection = -1;
                //if (event.key.keysym.sym == SDLK_d)
                //    player.turnDirection = +1;
                //if (event.key.keysym.sym == SDLK_a)
                //    player.turnDirection = -1;
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
    // waste some time until we reach the target frame time length
    //while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksLastFrame + FRAME_TIME_LENGTH)); // old method - slow?
    int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);
    if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH) {
        SDL_Delay(timeToWait);
    }
    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

    ticksLastFrame = SDL_GetTicks();

    movePlayer(deltaTime);
    castAllRays();
}

void renderWallProjection(void) {
    for (int x = 0; x < NUM_RAYS; x++) {
        float perpDistance = rays[x].distance * cos(rays[x].rayAngle - player.rotationAngle);
        float projectedWallHeight = (TILE_SIZE / perpDistance) * DIST_PROJ_PLANE;

        int wallStripHeight = (int)projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel; // clamped wall height to top of screen distorting texture - see line 431


        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        // set the color of the ceiling
        for (int y = 0; y < wallTopPixel; y++) {
            drawPixel(x, y, 0xFF333333);
            // colorBuffer[(WINDOW_WIDTH * y) + x] = 0xFF333333;
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

        /// <summary>
        /// get texture
        /// </summary>
        int texNum = rays[x].wallHitContent - 1;

        /////////finding horizontal element of wall strip////////// horizontal down surface

        // render the wall from wallTopPixel to wallBottomPixel
        for (int y = wallTopPixel; y < wallBottomPixel; y++) {
            // to correct  texture distortion due to line 404
            int distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);

            //TODO: calculate textureOffsetY
            int textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT / wallStripHeight);

            // set color of wall based on texture
            uint32_t texelColor = textures[texNum][(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
            //colorBuffer[(WINDOW_WIDTH * y) + x] = texelColor;
            drawPixel(x, y, texelColor);
            //colorBuffer[(WINDOW_WIDTH * y) + x] = rays[x].wasHitVertical ? 0xFFFFFFFF : 0xFFCCCCCC;
        }

        // set the color of the floor
        for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++) {
            drawPixel(x, y, 0xFF777777);
            //colorBuffer[(WINDOW_WIDTH * y) + x] = 0xFF777777;
        }
            
    }
}

void render() {
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //SDL_RenderClear(renderer);
    clearColorBuffer(0xFF000000);

    //generate3DProjection();

    renderWallProjection();

    //drawRect(10, 10, 42, 34, 0xffffffff);

    renderMap(); // cause colorBuffer write access violation???
    //renderRays();
    renderPlayer();

    renderColorBuffer();

    //SDL_RenderPresent(renderer);
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

    //destroyWindow(); /not sure this needs changed
    releaseResources(); // not sure this was needed - just following tutorial

    return 0;
}
