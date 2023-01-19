#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <functional>

class LTexture {
private:
    SDL_Texture *mTexture = nullptr;
    int mWidth;
    int mHeight;
public:
    LTexture();

    ~LTexture();

    bool loadTextureFromText(const std::string& text, SDL_Color color);

    bool loadTextureFromFile( std::string path );

    void drawTexture( int x, int y, int w = 0, int h = 0, SDL_Rect* clip = NULL,
                      double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

    int getWidth() const;

    int getHeight() const;

    void free();
};

typedef struct ConsoleInfo{
    int windowWidth;
    int windowHeight;
    int nCharsX;
    int nCharsY;
    char *screenBuffer;
} ConsoleInfo;

struct Color {
    int r;
    int g;
    int b;
};

class GameEngine {
protected:
    int mWindowWidth;
    int mWindowHeight;
    SDL_Event e;
private:
    void initScreen();
    SDL_Window *gWindow = nullptr;
public:
    GameEngine();

    virtual bool onFrameUpdate(float fElapsedTime) = 0;

    virtual bool onInit() = 0;

    virtual void onKeyboardEvent(int keycode, float secPerFrame);

    virtual void
    onMouseEvent(int posX, int posY, float secPerFrame, unsigned int mouseState, unsigned char button);

    virtual bool drawPoint(int x, int y, Color color = {0xFF, 0xFF, 0xFF});

    bool drawLine(int x1, int y1, int x2, int y2, Color color = {0xFF, 0xFF, 0xFF});

    void DrawWireFrameModel(const std::vector<std::pair<float, float>> &vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, Color color = {0xFF, 0xFF, 0xFF});

    bool constructConsole(int nCharsX, int nCharsY, const char * title);

    bool createResources();

    bool renderConsole();

    void startGameLoop();

    void close_sdl();

    ~GameEngine();
};
