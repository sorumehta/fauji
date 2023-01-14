#include <iostream>
#include "SimpleGameEngine.hpp"
#include <cmath>
class Fauji : public GameEngine {
private:
    int nMapWidth = 800;
    int nMapHeight = 450;
    unsigned char *map = nullptr;

public:
    void onKeyboardEvent(int keycode, float secPerFrame) override {

    }

    void onMouseEvent(int posX, int posY, float secPerFrame, unsigned int mouseState, unsigned char button) override {

    }

    bool onInit() override {
        // create map
        map = new unsigned char[nMapHeight * nMapWidth];
        // initialise it with 0
        memset(map, 0, nMapWidth * nMapHeight * sizeof(unsigned char));
        createMap();

        return true;
    }

    bool onFrameUpdate(float fElapsedTime) override {

        // Draw landscape
        for (int y=0; y<mWindowHeight; y++){
            for (int x=0; x<mWindowWidth; x++){
                float fMapVal = map[y*nMapWidth + x];
                if(fMapVal == 0){
                    drawPoint(x, y, {0x00, 0xFF, 0xFF});
                    // draw sky
                } else if(fMapVal == 1){
                    // draw land 006400
                    drawPoint(x, y, {0x00, 0x64, 0x00});
                }
            }
        }

        return true;
    }

    void createMap(){
        // used in 1D perlin noise
        float *fSurface = new float[nMapWidth];
        float *fNoiseSeed = new float[nMapWidth];

        for(int i=0; i<nMapWidth; i++){
            fNoiseSeed[i] = (float)rand() / (float)RAND_MAX;
        }
        fNoiseSeed[0] = 0.5f; // Because we want the terrain to start and end at half the height of the screen
        perlinNoise1D(nMapWidth, fNoiseSeed, 8, 2.0f, fSurface);
        for(int y=0; y<nMapHeight; y++){
            for(int x=0; x<nMapWidth; x++){
                // if the current pixel in map is greater than corresponding pixel in noise output
                // we set it to 1 (land) (y=0 is at top)
                if(y > static_cast<int>( std::round(fSurface[x] * static_cast<float>(nMapHeight) )))
                {
                    map[y * nMapWidth + x] = 1;
                } else{
                    map[y * nMapWidth + x] = 0;
                }
            }
        }
        delete[] fNoiseSeed;
        delete[] fSurface;

    }

    // Taken from Perlin Noise Video https://youtu.be/6-0UaeJBumA
    void perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput)
    {
        // Used 1D Perlin Noise
        for (int x = 0; x < nCount; x++)
        {
            float fNoise = 0.0f;
            float fScaleAcc = 0.0f;
            float fScale = 1.0f;

            for (int o = 0; o < nOctaves; o++)
            {
                int nPitch = nCount >> o;
                int nSample1 = (x / nPitch) * nPitch;
                int nSample2 = (nSample1 + nPitch) % nCount;
                float fBlend = (float)(x - nSample1) / (float)nPitch;
                float fSample = (1.0f - fBlend) * fSeed[nSample1] + fBlend * fSeed[nSample2];
                fScaleAcc += fScale;
                fNoise += fSample * fScale;
                fScale = fScale / fBias;
            }

            // Scale to seed range
            fOutput[x] = fNoise / fScaleAcc;
        }
    }
};

int main() {
    Fauji fauji;
    fauji.constructConsole(800, 450, "Fauji");
    fauji.startGameLoop();
    return 0;
}
