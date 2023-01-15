#include <iostream>
#include "SimpleGameEngine.hpp"
#include <cmath>
#include <list>

const float PI = 3.1415f;

class cPhysicsObject {
public:
    float px = 0.0f;
    float py = 0.0f;
    float vx = 0.0f ;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
    float radius = 4.0f;
    bool bStable = false;
    float fFriction = 0.8f;

    cPhysicsObject(float x = 0.0f, float y = 0.0f): px(x), py(y) {}

    virtual void draw(GameEngine *engine, float fOffsetX, float fOffsetY) = 0;
};

class cDummy : public cPhysicsObject {
public:
    cDummy(float x = 0.0f, float y = 0.0f): cPhysicsObject(x, y) {}

    void draw(GameEngine *engine, float fOffsetX, float fOffsetY) override {
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, std::atan2f(vy, vx), radius);
    }
private:
    // we want vecModel to be shared among all instances, so make it static
    // since it is static, it must be initialised out of line
     static std::vector<std::pair<float, float>> vecModel;
};

std::vector<std::pair<float, float>> defineDummy() {
    std::vector<std::pair<float, float>> vecModel;
    vecModel.emplace_back(0, 0);
    int nVertices = 10;
    for(int i = 0; i <= nVertices; i++){
        vecModel.emplace_back(std::cos((i/(float)(nVertices)) * 2.0f * PI), std::sin((i/(float)(nVertices)) * 2.0f * PI ));
    }
    return vecModel;
}
// out of line initialisation of static member
std::vector<std::pair<float, float>> cDummy::vecModel = defineDummy();

class Fauji : public GameEngine {
private:
    int nMapWidth = 1024;
    int nMapHeight = 512;
    unsigned char *map = nullptr;
    float fCameraPosX = 0;
    float fCameraPosY = 0;
    float fMapScrollSpeed = 400.0f;
    std::list<cPhysicsObject *> listObjects;

public:
    void onKeyboardEvent(int keycode, float secPerFrame) override {

    }

    void onMouseEvent(int mousePosX, int mousePosY, float secPerFrame, unsigned int mouseEvent, unsigned char button) override {
        if(mouseEvent == SDL_MOUSEMOTION){
            if(mousePosX < 15) fCameraPosX -= fMapScrollSpeed * secPerFrame;
            if (mousePosX > mWindowWidth - 15) fCameraPosX += fMapScrollSpeed * secPerFrame;
            if (mousePosY < 15) fCameraPosY -= fMapScrollSpeed * secPerFrame;
            if (mousePosY > mWindowHeight - 15) fCameraPosY += fMapScrollSpeed * secPerFrame;
            // Clamp map boundaries
            if (fCameraPosX < 0) fCameraPosX = 0;
            if (fCameraPosX >= nMapWidth - mWindowWidth) fCameraPosX = nMapWidth - mWindowWidth;
            if (fCameraPosY < 0) fCameraPosY = 0;
            if (fCameraPosY >= nMapHeight - mWindowHeight) fCameraPosY = nMapHeight - mWindowHeight;
        }
        if(mouseEvent == SDL_MOUSEBUTTONDOWN){
            if(button == SDL_BUTTON_RIGHT){
                cDummy *p = new cDummy(mousePosX + fCameraPosX, mousePosY + fCameraPosY);
                p->radius = 16.0f;
                listObjects.push_back(p);
            }
        }
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

        // do 10 physics iterations per frame, since drawing a frame is slower than updating physics
        for(int z=0; z < 10; z++) {

            // update physics of physical objects
            for (auto obj: listObjects) {
                // apply gravity
                obj->ay += 2.0f;
                // update velocity
                obj->vx += obj->ax * fElapsedTime;
                obj->vy += obj->ay * fElapsedTime;
                // update positions
                float fPotentialX = obj->px + obj->vx * fElapsedTime;
                float fPotentialY = obj->py + obj->vy * fElapsedTime;

                // reset forces after applying them
                obj->ay = 0;
                obj->ax = 0;
                obj->bStable = false;

                // Collision check with map
                float fAngle = std::atan2f(obj->vy, obj->vx);
                float fResponseX = 0;
                float fResponseY = 0;
                bool bCollision = false;
                // Iterate through the semicircle in direction of motion
                for(float r = fAngle - PI/2.0f; r < fAngle + PI/2.0f; r += PI/8.0f){
                    float fTestPosX = obj->radius * std::cos(r) + fPotentialX;
                    float fTestPosY = obj->radius * std::sin(r) + fPotentialY;
                    // clamp the boundaries
                    if (fTestPosX >= nMapWidth) fTestPosX = nMapWidth - 1;
                    if (fTestPosY >= nMapHeight) fTestPosY = nMapHeight - 1;
                    if (fTestPosX < 0) fTestPosX = 0;
                    if (fTestPosY < 0) fTestPosY = 0;

                    // check if map collides at test position
                    if(map[static_cast<int>(std::round( fTestPosY))*(nMapWidth) + static_cast<int>(std::round(fTestPosX))] != 0){
                        // Accumulate the collision vectors to create a response vector
                        // the final response vector will be normal to the areas of contact
                        fResponseX += fPotentialX - fTestPosX;
                        fResponseY += fPotentialY - fTestPosY;
                        bCollision = true;
                    }
                }
                float fMagVelocity = std::sqrt(obj->vx*obj->vx + obj->vy*obj->vy); // |d|
                float fMagResponse = std::sqrt(fResponseX*fResponseX + fResponseY*fResponseY); // |n|

                if(bCollision){
                    obj->bStable = true;

                    // reflection equation, where d is the impact vector (velocity), and n is normal to the surface which is normalised (response vector)
                    // 𝑟=𝑑−2(𝑑⋅𝑛)𝑛


                    float fDdotN = obj->vx*(fResponseX/fMagResponse) + obj->vy*(fResponseY/fMagResponse);
                    obj->vx = obj->fFriction * ( obj->vx - 2.0f*fDdotN*fResponseX / fMagResponse );
                    obj->vy = obj->fFriction * ( obj->vy - 2.0f*fDdotN*fResponseY / fMagResponse );


                } else{
                    obj->px = fPotentialX;
                    obj->py = fPotentialY;
                }
                // Turn off movement when tiny
                if (fMagVelocity < 0.4f) obj->bStable = true;
            }
        }

        // Draw landscape
        for (int y=0; y<mWindowHeight; y++){
            for (int x=0; x<mWindowWidth; x++){
                float fMapVal = map[static_cast<int>(std::round((y + fCameraPosY)*nMapWidth + (x + fCameraPosX)))];
                if(fMapVal == 0){
                    drawPoint(x, y, {0x00, 0xFF, 0xFF});
                    // draw sky
                } else if(fMapVal == 1){
                    // draw land 006400
                    drawPoint(x, y, {0x00, 0x64, 0x00});
                }
            }
        }

        for(auto &p : listObjects){
            p->draw(this, fCameraPosX, fCameraPosY);
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
