#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <limits>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BGCOLOR {0,0,0,255}
#define pixel(x,y) helpers::pixels[(x)+SCREEN_WIDTH*(y)]
#define GRIDSIZE 10

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

void VpixelColor(int i, int j, const helpers::Pixel& color)
{
    j = (SCREEN_HEIGHT / GRIDSIZE) - 1 - j; //invert y axis
    for (int x = i * GRIDSIZE; x < (i + 1) * GRIDSIZE; x++)
        for (int y = j * GRIDSIZE; y < (j + 1) * GRIDSIZE; y++)
            pixel(x, y) = color;
}

void DrawPoly(helpers::Point f, helpers::Point l)
{
    if (f.x > l.x) {
        std::swap(f, l);
    }

    float dx = l.x - f.x;//dx = x2 - x1
    float dy = l.y - f.y;//dy = y2 - y1
    float s = (dy / dx); //slope
    float B = f.y - (dy / dx) * f.x; //from line equation y = (dy/dx)x + B;

    helpers::Point crnt(f); //take first pixel as current
    VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });

    helpers::Point m; //m as midpoint

    if (s >= 0 && s <= 1) {
        while (crnt.x < l.x&& crnt.y <= l.y)
        {
            m = { crnt.x + 1.f,crnt.y + 0.5f };
            bool Above = m.y * dx - m.x * dy - B * dx > 0;  //decision variable
            if (Above) {
                //select E pixel
                crnt.x += 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }
            else {
                //select NE pixel
                crnt.x += 1;
                crnt.y += 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }
        }
    }
    else if (s > 1)
    {
        while (crnt.y < l.y&& crnt.x <= l.x)
        {
            m = { crnt.x + 0.5f,crnt.y + 1.f };
            bool isAbove = m.y * dx - m.x * dy - B * dx > 0;  //decision variable
            if (isAbove) {
                //select NE pixel
                crnt.x += 1;
                crnt.y += 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }
            else {
                //select N pixel 
                crnt.y += 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }
        }
    }
    else if (s < 0 && s >= -1)
    {
        while (crnt.x < l.x&& l.y <= crnt.y)
        {
            m = { crnt.x + 1.f,crnt.y - 0.5f };
            bool isAbove = m.y * dx - m.x * dy - B * dx > 0;  //decision variable
            if (isAbove) {
                //select SE pixel
                crnt.x += 1;
                crnt.y -= 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }
            else {
                //select E pixel
                crnt.x += 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }
        }
    }
    else if (s < -1)
    {
        while (crnt.y > l.y && crnt.x <= l.x)
        {
            m = { crnt.x + 0.5f,crnt.y - 1.f };
            bool isAbove = m.y * dx - m.x * dy - B * dx > 0;  //decision variable
            if (isAbove) {
                //select S pixel
                crnt.y -= 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }
            else {
                //select SE pixel
                crnt.x += 1;
                crnt.y -= 1;
                VpixelColor(crnt.x, crnt.y, { 255,255,255,255 });
            }

        }
    }
}

void FillPoly(const helpers::Pixel& color, std::vector<helpers::Point> PolyPoints) {

    for (float y = 0; y < (SCREEN_HEIGHT / GRIDSIZE) - 1; y += 0.5f)
    {
        std::vector<float> x_ip;//vector of intersection points

        helpers::Point ip;

        //finding intersection points
        for (int i = 0; i < (int)PolyPoints.size() - 1; i++)
        {
            float dx = PolyPoints[i + 1].x - PolyPoints[i].x;
            float dy = PolyPoints[i + 1].y - PolyPoints[i].y;

            if (std::abs(dy - 0.f) == 0)
                continue;

            ip.x = (y - PolyPoints[i].y) * (dx / dy) + PolyPoints[i].x;
            ip.y = y;

            if (std::min(PolyPoints[i].y, PolyPoints[i + 1].y) <= ip.y && ip.y <= std::max(PolyPoints[i].y, PolyPoints[i + 1].y) && std::min(PolyPoints[i].x, PolyPoints[i + 1].x) <= ip.x && ip.x <= std::max(PolyPoints[i].x, PolyPoints[i + 1].x))
                x_ip.push_back(ip.x);
            else
                continue;
        }

        //sort intersection points
        std::sort(x_ip.begin(), x_ip.end());

        //fill colour
        for (int i = 0; i < (int)x_ip.size() - 1; i += 2)
        {
            for (float x = x_ip[i]; x <= x_ip[i + 1]; x += 0.5f)
            {
                VpixelColor(x, y, color);
            }
        }
    }
}

void update(std::vector<helpers::Pixel>& pixels)
{
    //virtual display
    for (int x = 0; x < SCREEN_WIDTH; x += GRIDSIZE)//vertical line
    {
        for (int y = 0; y < SCREEN_HEIGHT; y++)
            pixel(x, y) = { 255,255,255,255 };
    }
    for (int y = 0; y < SCREEN_HEIGHT; y += GRIDSIZE)//horizontal line
    {
        for (int x = 0;x < SCREEN_WIDTH;x++)
            pixel(x, y) = { 255,255,255,255 };
    }

    //Polygon drawing
    std::vector<helpers::Point> PolyPoints;
    PolyPoints = { {2.f,2.f},{20.f,10.f},{15.f,26.f},{6.f,20.f},{2.f,2.f} };//convex polygon
    //PolyPoints = { {2.f,2.f},{10.f,26.f},{20.f,16.f},{30.f,26.f},{40.f,2.f} };//concave polygon
    int i = 0;
    for (i = 0;i < PolyPoints.size() - 1;i++) {
        DrawPoly(PolyPoints[i], PolyPoints[i + 1]);
    }
    DrawPoly(PolyPoints[0], PolyPoints[PolyPoints.size() - 1]);

    FillPoly({ 255,255,255,255 }, PolyPoints);
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