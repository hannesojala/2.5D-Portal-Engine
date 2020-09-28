#pragma once

#include <SDL2/SDL.h>
#include <linalg.h>
#include <string>
#include <vector>

struct Ray {
    linalg::aliases::float2 origin, direction;
};

struct RGBA {
    unsigned char r, g, b, a;
};

class Window {
private:
    SDL_Window* pWindow;
    SDL_Renderer* pRenderer;
public:
    Window(std::string title, int w, int h) {
        pWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
        pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
        // Transparency
        SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);
    }
    Window(std::string title, int w, int h, int px, int py) {
        pWindow = SDL_CreateWindow(title.c_str(), px, py, w, h, SDL_WINDOW_SHOWN);
        pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
        // Transparency
        SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);
    }
    ~Window() {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
    }
    void setTitle(std::string title) {
        SDL_SetWindowTitle(pWindow, title.c_str());
    }
    void setColor(RGBA clr) {
        SDL_SetRenderDrawColor(pRenderer, clr.r, clr.g, clr.b, clr.a);
    }
    void getPos(int& x, int& y) {
        SDL_GetWindowPosition(pWindow, &x, &y);
    }
    void drawLine(int x1, int y1, int x2, int y2) {
        SDL_RenderDrawLine(pRenderer, x1, y1, x2, y2);
    }
    void drawLineRGBA(int x1, int y1, int x2, int y2, RGBA clr) {
        SDL_SetRenderDrawColor(pRenderer, clr.r, clr.g, clr.b, clr.a);
        SDL_RenderDrawLine(pRenderer, x1, y1, x2, y2);
    }
    void drawRect(SDL_Rect rect, bool filled) {
        if (filled) SDL_RenderFillRect(pRenderer, &rect);
        else SDL_RenderDrawRect(pRenderer, &rect);
    }
    void drawRectRGBA(SDL_Rect rect, bool filled, RGBA clr) {
        SDL_SetRenderDrawColor(pRenderer, clr.r, clr.g, clr.b, clr.a);
        if (filled) SDL_RenderFillRect(pRenderer, &rect);
        else SDL_RenderDrawRect(pRenderer, &rect);
    }
    void clear(RGBA clr) {
        SDL_SetRenderDrawColor(pRenderer, clr.r, clr.g, clr.b, clr.a);
        SDL_RenderClear(pRenderer);
    }
    void render() {
        SDL_RenderPresent(pRenderer);
    }
    void focus() {
        SDL_RaiseWindow(pWindow);
    }
};
