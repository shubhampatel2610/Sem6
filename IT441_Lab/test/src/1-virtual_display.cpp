#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BGCOLOR {0,0,0,255}
#define pixel(x,y) pixels[(x)+SCREEN_WIDTH*(y)]

#define GRIDSIZE 10

//origin is at top left window 
//x goes from left to right and y goes from top to bottom

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

namespace helpers
{
    struct Pixel
    {
        uint8_t r, g, b, a;
    };

    struct Point
    {
        float x, y;
        bool operator==(Point rhs)
        {
            return (abs(x - rhs.x) < 1e-8) && (abs(y - rhs.y) < 1e-8);
        }

        bool operator!=(Point rhs)
        {
            return !((abs(x - rhs.x) < 1e-8) && (abs(y - rhs.y) < 1e-8));
        }
    };

    std::vector<helpers::Pixel> pixels{ SCREEN_WIDTH * SCREEN_HEIGHT, BGCOLOR };

    SDL_Surface* loadSurface(std::string path)
    {
        //The final optimized image
        SDL_Surface* optimizedSurface = NULL;

        //Load image at specified path
        SDL_Surface* loadedSurface = IMG_Load(path.c_str());
        if (loadedSurface == NULL)
        {
            std::cerr << "Unable to load image. SDL_image Error: \n" << path.c_str() << " - " << IMG_GetError();
        }
        else
        {
            //Convert surface to screen format
            optimizedSurface = SDL_ConvertSurface(loadedSurface, screenSurface->format, 0);
            if (optimizedSurface == NULL)
            {
                std::cerr << "Unable to optimize image! SDL Error: \n" << path.c_str() << " - " << SDL_GetError();
            }

            //Get rid of old loaded surface
            SDL_FreeSurface(loadedSurface);
        }

        return optimizedSurface;
    }
}

void VpixelColor(int i, int j, const helpers::Pixel& color, std::vector<helpers::Pixel>& pixels)
{
    j = (SCREEN_HEIGHT / GRIDSIZE) - 1 - j;
    for (int x = i * GRIDSIZE; x < (i + 1) * GRIDSIZE; x++)
        for (int y = j * GRIDSIZE; y < (j + 1) * GRIDSIZE; y++)
            pixel(x, y) = color;
}

void update(std::vector<helpers::Pixel>& pixels)
{
    /*//Draw line
    for (int i = 0;i<SCREEN_WIDTH;i++)
        pixel(i,20) = { 255,255,255,255 };*/

    /*//Virtual display
    int min = 40;
    int max = 80;
    for (int r = min; r <= max; r++) // row #
         for (int c = min; c <= max; c++) // col #
             pixel(r, c) = { 255,255,255,255 };
    for (int c = min; c <= max; c++) // row #
         for (int r = 100 - (c - min); r <= 100 + (c - min); r++)
             pixel(r, c) = { 255,255,255,255 };*/

    /*//color (x,y) pixel
    VpixelColor(2, 2, { 255,255,255,255 }, pixels);*/


    for (int x = 0; x < SCREEN_WIDTH; x += GRIDSIZE)
    {
        //vertical line
        for (int y = 0; y < SCREEN_HEIGHT; y++)
            pixel(x, y) = { 255,255,255,255 };
    }
    for (int y = 0; y < SCREEN_HEIGHT; y += GRIDSIZE)
    {
        //horizontal line
        for (int x = 0;x < SCREEN_WIDTH;x++)
            pixel(x, y) = { 255,255,255,255 };
    }

    
}

int main(int argc, char* args[]) {
    bool quit = false;

    SDL_Event event;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Computer Graphics",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGHT);
    //Pixel* pixels = new Pixel[SCREEN_WIDTH * SCREEN_HEIGHT];

    while (!quit)
    {
        SDL_UpdateTexture(texture, NULL, &helpers::pixels[0], 640 * sizeof(uint32_t));
        SDL_WaitEvent(&event);

        switch (event.type)
        {
        case SDL_QUIT:
            quit = true;
            break;
        }

        update(helpers::pixels);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}