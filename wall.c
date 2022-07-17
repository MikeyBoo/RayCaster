#include "wall.h"

void renderWallProjection(void) {
    for (int x = 0; x < NUM_RAYS; x++) {
        float perpDistance = rays[x].distance * cos(rays[x].rayAngle - player.rotationAngle);
        float projectedWallHeight = (TILE_SIZE / perpDistance) * DIST_PROJ_PLANE;

        int wallStripHeight = (int)projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;


        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        // set the color of the ceiling
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