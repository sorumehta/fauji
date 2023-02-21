#include "SimpleGameEngine.hpp"
#include <cmath>
#include <list>
#include <memory>

const float PI = 3.14159f;

class cPhysicsObject {
public:
    float px = 0.0f;
    float py = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
    float radius = 4.0f;
    bool bStable = false;
    float fFriction = 0.8f;
    int nBounceBeforeDeath = -1;
    bool bDead = false;

    cPhysicsObject(float x = 0.0f, float y = 0.0f) : px(x), py(y) {}

    virtual void draw(GameEngine *engine, float fOffsetX, float fOffsetY) = 0;

    virtual int ObjDeadAction() = 0;
};

class cDummy : public cPhysicsObject {
public:
    cDummy(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y) {
        radius = 16.0f;
    }

    void draw(GameEngine *engine, float fOffsetX, float fOffsetY) override {
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, std::atan2f(vy, vx), radius);
    }

    int ObjDeadAction() override;

private:
    // we want vecModel to be shared among all instances, so make it static
    // since it is static, it must be initialised out of line
    static std::vector<std::pair<float, float>> vecModel;
};

std::vector<std::pair<float, float>> defineDummy() {
    std::vector<std::pair<float, float>> vecModel;
    vecModel.emplace_back(0, 0);
    int nVertices = 10;
    for (int i = 0; i <= nVertices; i++) {
        vecModel.emplace_back(std::cos((i / (float) (nVertices)) * 2.0f * PI),
                              std::sin((i / (float) (nVertices)) * 2.0f * PI));
    }
    return vecModel;
}

// out of line initialisation of static member
std::vector<std::pair<float, float>> cDummy::vecModel = defineDummy();

int cDummy::ObjDeadAction() {
    return 0;
}


class cDebris : public cPhysicsObject {
public:
    cDebris(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y) {
        // Set velocity to random direction and size for "boom" effect
        vx = 10.0f * cosf(((float) rand() / (float) RAND_MAX) * 2.0f * PI);
        vy = 10.0f * sinf(((float) rand() / (float) RAND_MAX) * 2.0f * PI);
        radius = 1.0f;
        fFriction = 0.8f;
        nBounceBeforeDeath = 5;
    }

    void draw(GameEngine *engine, float fOffsetX, float fOffsetY) override {
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, std::atan2f(vy, vx), radius,
                                   {0x00, 0x64, 0x00});
    }

    int ObjDeadAction() override;

private:
    // we want vecModel to be shared among all instances, so make it static
    // since it is static, it must be initialised out of line
    static std::vector<std::pair<float, float>> vecModel;
};

std::vector<std::pair<float, float>> defineDebris() {
    // A small unit rectangle
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({0.0f, 0.0f});
    vecModel.push_back({1.0f, 0.0f});
    vecModel.push_back({1.0f, 1.0f});
    vecModel.push_back({0.0f, 1.0f});
    return vecModel;
}

// out of line initialisation of static member
std::vector<std::pair<float, float>> cDebris::vecModel = defineDebris();

int cDebris::ObjDeadAction() {
    return 0;
}


class cMissile : public cPhysicsObject // A projectile weapon
{
public:
    cMissile(float x = 0.0f, float y = 0.0f, float _vx = 0.0f, float _vy = 0.0f) : cPhysicsObject(x, y) {
        radius = 5.0f;
        fFriction = 0.5f;
        vx = _vx;
        vy = _vy;
        bDead = false;
        nBounceBeforeDeath = 1;
    }

    virtual void draw(GameEngine *engine, float fOffsetX, float fOffsetY) override {
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), radius, {0xFF, 0, 0});
    }

    int ObjDeadAction() override;


private:
    static std::vector<std::pair<float, float>> vecModel;
};

int cMissile::ObjDeadAction() {
    return 20;
}

std::vector<std::pair<float, float>> DefineMissile() {
    // Defines a rocket like shape
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({0.0f, 0.0f});
    vecModel.push_back({1.0f, 1.0f});
    vecModel.push_back({2.0f, 1.0f});
    vecModel.push_back({2.5f, 0.0f});
    vecModel.push_back({2.0f, -1.0f});
    vecModel.push_back({1.0f, -1.0f});
    vecModel.push_back({0.0f, 0.0f});
    vecModel.push_back({-1.0f, -1.0f});
    vecModel.push_back({-2.5f, -1.0f});
    vecModel.push_back({-2.0f, 0.0f});
    vecModel.push_back({-2.5f, 1.0f});
    vecModel.push_back({-1.0f, 1.0f});

    // Scale points to make shape unit sized
    for (auto &v: vecModel) {
        v.first /= 2.5f;
        v.second /= 2.5f;
    }
    return vecModel;
}

std::vector<std::pair<float, float>> cMissile::vecModel = DefineMissile();


class cMan : public cPhysicsObject {
public:
    SDL_RendererFlip flipType;
    float fShootingAngle;

    void initSpriteClips() {
        spriteClips[0].x = 0;
        spriteClips[0].y = 0;
        spriteClips[0].w = 64;
        spriteClips[0].h = 205;

        spriteClips[1].x = 64;
        spriteClips[1].y = 0;
        spriteClips[1].w = 64;
        spriteClips[1].h = 205;

        spriteClips[2].x = 128;
        spriteClips[2].y = 0;
        spriteClips[2].w = 64;
        spriteClips[2].h = 205;

        spriteClips[3].x = 192;
        spriteClips[3].y = 0;
        spriteClips[3].w = 64;
        spriteClips[3].h = 205;
    }

    cMan(float x, float y) : cPhysicsObject(x, y) {
        fFriction = 0.2f;
        radius = 16.0f;
        bDead = false;
        nBounceBeforeDeath = -1;
        initSpriteClips();
        frame = 0;
        fShootingAngle = flipType == SDL_FLIP_NONE ? -PI : PI;
        if (spritePtr == nullptr) {
            spritePtr = new LTexture();
            spritePtr->loadTextureFromFile("../res/foo.png");
        }
    }

    int ObjDeadAction() override;

    void draw(GameEngine *engine, float fOffsetX, float fOffsetY) override;

private:
    static LTexture *spritePtr; // shared across instances
    SDL_Rect spriteClips[4];
    int frame;
};

LTexture *cMan::spritePtr = nullptr;

void cMan::draw(GameEngine *engine, float fOffsetX, float fOffsetY) {
    SDL_Rect *currentClip = &spriteClips[frame / 2];
    if (std::abs(vx) < 3) {
        currentClip = &spriteClips[0];
    }
    spritePtr->drawTexture(px - fOffsetX - radius, py - fOffsetY - radius, radius * 2, radius * 2, currentClip, 0, NULL,
                           flipType);
    frame++;
    if (frame / 2 >= 4) {
        frame = 0;
    }
}

int cMan::ObjDeadAction() {
    return 0;
}

class Fauji : public GameEngine {
private:
    int nMapWidth = 1024;
    int nMapHeight = 512;
    unsigned char *map = nullptr;
    float fCameraPosX = 0;
    float fCameraPosY = 0;
    float fCameraPosXTarget = 0;
    float fCameraPosYTarget = 0;

    float fMapScrollSpeed = 400.0f;
    std::list<std::unique_ptr<cPhysicsObject>> listObjects;
    cPhysicsObject *pObjectUnderControl = nullptr;
    cPhysicsObject *pCameraTrackingObject = nullptr;
    bool bEnergising = false;
    bool bFireWeapon = false;
    float fEnergyLevel = 0;
    bool bGameIsStable = false;
    bool bPlayerHasControl = false;
    bool bPlayerActionComplete = false;
    enum GAME_STATE {
        GS_RESET = 0,
        GS_GENERATE_TERRAIN = 1,
        GS_GENERATING_TERRAIN,
        GS_ALLOCATE_UNITS,
        GS_ALLOCATING_UNITS,
        GS_START_PLAY,
        GS_CAMERA_MODE
    } nGameState, nNextState;

public:
    void onUserInputEvent(int eventType, int button, int mousePosX, int mousePosY, float secPerFrame) {
        if (!bPlayerHasControl) {
            return;
        }
        if (eventType == SDL_MOUSEBUTTONDOWN) {
            if (button == SDL_BUTTON_RIGHT) {
                listObjects.push_back(std::make_unique<cMissile>(mousePosX + fCameraPosX, mousePosY + fCameraPosY));
            } else if (button == SDL_BUTTON_LEFT) {
                cMan *man = new cMan(mousePosX + fCameraPosX, mousePosY + fCameraPosY);
                pObjectUnderControl = man;
                pCameraTrackingObject = man;
                listObjects.push_back(std::unique_ptr<cMan>(man));
            }
        }
        if (eventType == SDL_KEYDOWN) {
            if (pObjectUnderControl != nullptr) {
                if (pObjectUnderControl->bStable) {
                    cMan *pMan = dynamic_cast<cMan *>(pObjectUnderControl);
                    if (button == SDLK_RIGHT) {
                        pMan->vx = 5.0f;
                        pMan->vy = -5.0f;
                        pMan->flipType = SDL_FLIP_HORIZONTAL;
                        pMan->bStable = false;
                    } else if (button == SDLK_LEFT) {
                        pMan->vx = -5.0f;
                        pMan->vy = -5.0f;
                        pMan->flipType = SDL_FLIP_NONE;
                        pMan->bStable = false;
                    } else if (button == SDLK_a) {
                        pMan->fShootingAngle -= 1.0f * secPerFrame;
                        if (pMan->fShootingAngle < -PI) {
                            pMan->fShootingAngle = PI;
                        }
                    } else if (button == SDLK_s) {
                        pMan->fShootingAngle += 1.0f * secPerFrame;
                        if (pMan->fShootingAngle > PI) {
                            pMan->fShootingAngle = -PI;
                        }
                    } else if (button == SDLK_SPACE) {
                        if (!bEnergising) {
                            bEnergising = true;
                            bFireWeapon = false;
                            fEnergyLevel = 0.0f;
                        } else {
                            fEnergyLevel = 0.75f * secPerFrame;
                            if (fEnergyLevel > 1.0f) {
                                fEnergyLevel = 1.0f;
                                bFireWeapon = true;
                            }
                        }
                    }

                }
            }
        } else if (eventType == SDL_KEYUP) {
            if (pObjectUnderControl != nullptr) {
                if (pObjectUnderControl->bStable) {
                    if (button == SDLK_SPACE) {
                        if (bEnergising) {
                            bFireWeapon = true;
                        }
                        bEnergising = false;
                    }
                }
            }
        }
    }

    bool onInit() override {
        // create map
        map = new unsigned char[nMapHeight * nMapWidth];
        // initialise it with 0
        memset(map, 0, nMapWidth * nMapHeight * sizeof(unsigned char));
        //createMap();
        auto onUserInputFn = [this](int eventType, int buttonCode, int mousePosX, int mousePosY, float secPerFrame) {
            onUserInputEvent(eventType, buttonCode, mousePosX, mousePosY, secPerFrame);
        };
        InputEventHandler::addCallback("onUserInputFn_Game", onUserInputFn);
        nGameState = GS_RESET;
        nNextState = GS_RESET;
        return true;
    }

    void drawPlayerAim(int cx, int cy) {
        drawPoint(cx, cy, {0xFF, 0, 0});
        drawPoint(cx - 1, cy, {0xFF, 0, 0});
        drawPoint(cx, cy - 1, {0xFF, 0, 0});
        drawPoint(cx + 1, cy, {0xFF, 0, 0});
        drawPoint(cx, cy + 1, {0xFF, 0, 0});

    }

    bool onFrameUpdate(float fElapsedTime) override {
        switch (nGameState) {

            case GS_RESET: {
                nNextState = GS_GENERATE_TERRAIN;
                bPlayerHasControl = false;
            }
                break;
            case GS_GENERATE_TERRAIN: {
                createMap();
                nNextState = GS_GENERATING_TERRAIN;
                bPlayerHasControl = false;
            }
                break;
            case GS_GENERATING_TERRAIN: {
                nNextState = GS_ALLOCATE_UNITS;
                bPlayerHasControl = false;
            }
                break;
            case GS_ALLOCATE_UNITS: {
                bPlayerHasControl = false;
                cMan *man = new cMan(64.0f, 4.0f);
                listObjects.push_back(std::unique_ptr<cMan>(man));
                pObjectUnderControl = man;
                pCameraTrackingObject = pObjectUnderControl;
                nNextState = GS_ALLOCATING_UNITS;
            }
                break;

            case GS_ALLOCATING_UNITS: {
                bPlayerHasControl = false;
                if (bGameIsStable) {
                    nNextState = GS_START_PLAY;
                    bPlayerActionComplete = false;
                }
            }
                break;
            case GS_START_PLAY: {
                bPlayerHasControl = true;

                if (bPlayerActionComplete) {
                    nNextState = GS_CAMERA_MODE;
                }
            }
                break;
            case GS_CAMERA_MODE: {
                bPlayerHasControl = false;
                bPlayerActionComplete = false;
                if(bGameIsStable){
                    nNextState = GS_START_PLAY;
                    pCameraTrackingObject = pObjectUnderControl;
                }
            }
                break;
        }

        // do 10 physics iterations per frame, since drawing a frame is slower than updating physics
        for (int z = 0; z < 10; z++) {

            // update physics of physical objects
            for (auto &obj: listObjects) {
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
                for (float r = fAngle - PI / 2.0f; r < fAngle + PI / 2.0f; r += PI / 8.0f) {
                    float fTestPosX = obj->radius * std::cos(r) + fPotentialX;
                    float fTestPosY = obj->radius * std::sin(r) + fPotentialY;
                    // clamp the boundaries
                    if (fTestPosX >= nMapWidth) fTestPosX = nMapWidth - 1;
                    if (fTestPosY >= nMapHeight) fTestPosY = nMapHeight - 1;
                    if (fTestPosX < 0) fTestPosX = 0;
                    if (fTestPosY < 0) fTestPosY = 0;

                    // check if map collides at test position
                    if (map[static_cast<int>(std::round(fTestPosY)) * (nMapWidth) +
                            static_cast<int>(std::round(fTestPosX))] != 0) {
                        // Accumulate the collision vectors to create a response vector
                        // the final response vector will be normal to the areas of contact
                        fResponseX += fPotentialX - fTestPosX;
                        fResponseY += fPotentialY - fTestPosY;
                        bCollision = true;
                    }
                }
                float fMagVelocity = std::sqrt(obj->vx * obj->vx + obj->vy * obj->vy); // |d|
                float fMagResponse = std::sqrt(fResponseX * fResponseX + fResponseY * fResponseY); // |n|

                if (bCollision) {
                    obj->bStable = true;

                    // reflection equation, where d is the impact vector (velocity), and n is normal to the surface which is normalised (response vector)
                    // 𝑟=𝑑−2(𝑑⋅𝑛)𝑛


                    float fDdotN = obj->vx * (fResponseX / fMagResponse) + obj->vy * (fResponseY / fMagResponse);
                    obj->vx = obj->fFriction * (obj->vx - 2.0f * fDdotN * fResponseX / fMagResponse);
                    obj->vy = obj->fFriction * (obj->vy - 2.0f * fDdotN * fResponseY / fMagResponse);

                    if (obj->nBounceBeforeDeath > 0) {
                        (obj->nBounceBeforeDeath)--;
                        obj->bDead = obj->nBounceBeforeDeath == 0; // object is dead if no more bounces left
                        if (obj->bDead) {
                            int nResponse = obj->ObjDeadAction();
                            if (nResponse > 0) {
                                BOOM(obj->px, obj->py, nResponse);
                                pCameraTrackingObject = nullptr;
                            }
                        }
                    }
                } else {
                    // we let an object update its position only when it is not colliding
                    obj->px = fPotentialX;
                    obj->py = fPotentialY;
                }
                // Turn off movement when tiny
                if (fMagVelocity < 0.4f) obj->bStable = true;
            }

            // remove dead objects from list
            // Note that this function just removes the object, doesn't deallocate memory.
            // That's why we use unique_pointer, so that removing the pointer also frees the memory.
            // make sure to pass references to unique pointer in argument, since its copy constructor
            // is explicitly disabled (for obvious reasons).
            listObjects.remove_if([](std::unique_ptr<cPhysicsObject> &o) { return o->bDead; });
        }

        if (pCameraTrackingObject != nullptr) {
            fCameraPosXTarget = pCameraTrackingObject->px - mWindowWidth / 2;
            // we interpolate the camera position slowly between current
            // position and target position to give a smooth transition effect
            fCameraPosX += (fCameraPosXTarget - fCameraPosX) * 5.0f * fElapsedTime;
            // TODO: tracking y is causing bugs, fix it later
//            fCameraPosY = pCameraTrackingObject->py - mWindowHeight/2;
        }

        if (fCameraPosX < 0) fCameraPosX = 0;
        if (fCameraPosX >= nMapWidth - mWindowWidth) fCameraPosX = nMapWidth - mWindowWidth;
        if (fCameraPosY < 0) fCameraPosY = 0;
        if (fCameraPosY >= nMapHeight - mWindowHeight) fCameraPosY = nMapHeight - mWindowHeight;

        // Draw landscape
        for (int y = 0; y < mWindowHeight; y++) {
            for (int x = 0; x < mWindowWidth; x++) {
                float fMapVal = map[static_cast<int>(std::round((y + fCameraPosY) * nMapWidth + (x + fCameraPosX)))];
                if (fMapVal == 0) {
                    drawPoint(x, y, {0x00, 0xFF, 0xFF});
                    // draw sky
                } else if (fMapVal == 1) {
                    // draw land dark green 006400
                    drawPoint(x, y, {0x00, 0x64, 0x00});
                }
            }
        }

        if (pObjectUnderControl != nullptr) {
            cMan *pMan = dynamic_cast<cMan *>(pObjectUnderControl);
            // directions of shooting
            float dx = std::cos(pMan->fShootingAngle);
            float dy = std::sin(pMan->fShootingAngle);
            if (bFireWeapon) {
                const float fMagFireVelocity = 40;
                fEnergyLevel = 0.5f;
                cMissile *missile = new cMissile(pMan->px, pMan->py, fMagFireVelocity * fEnergyLevel * dx,
                                                 fMagFireVelocity * fEnergyLevel * dy);
                listObjects.push_back(std::unique_ptr<cMissile>(missile));
                pCameraTrackingObject = missile;
                bFireWeapon = false;
                fEnergyLevel = 0.0f;
                bEnergising = false;
                bPlayerActionComplete = true;
            }

            const int aimLength = 30;
            int cx = static_cast<int>(aimLength * dx + pMan->px - fCameraPosX);
            int cy = static_cast<int>(aimLength * dy + pMan->py - fCameraPosY);
            drawPlayerAim(cx, cy);
        }
        // draw objects
        for (auto &p: listObjects) {
            p->draw(this, fCameraPosX, fCameraPosY);
        }

        // check for game stability
        bGameIsStable = true;
        for (auto &p: listObjects) {
            if (!p->bStable) {
                bGameIsStable = false;
                break;
            }
        }
        if (bGameIsStable) {
            fillRect(4, 4, 10, 10, {0xFF, 0, 0});
        }
        nGameState = nNextState;
        return true;
    }

    // create an explosion of a certain radius at a certain position in the world
    void BOOM(float fWorldX, float fWorldY, float fRadius) {
        auto CircleBresenham = [&](int xc, int yc, int r) {
            // Taken from wikipedia
            int x = 0;
            int y = r;
            int p = 3 - 2 * r;
            if (!r) return;

            // procedure to create sky along the line
            auto drawSkyline = [&](int sx, int ex, int ny) {
                for (int i = sx; i < ex; i++)
                    if (ny >= 0 && ny < nMapHeight && i >= 0 && i < nMapWidth)
                        map[ny * nMapWidth + i] = 0;
            };

            while (y >= x) {
                // Modified to draw scan-lines instead of edges
                drawSkyline(xc - x, xc + x, yc - y);
                drawSkyline(xc - y, xc + y, yc - x);
                drawSkyline(xc - x, xc + x, yc + y);
                drawSkyline(xc - y, xc + y, yc + x);
                if (p < 0) p += 4 * x++ + 6;
                else p += 4 * (x++ - y--) + 10;
            }
        };

        // Erase Terrain to form crater
        CircleBresenham(fWorldX, fWorldY, fRadius);
        // impact nearby bodies
        for (auto &p: listObjects) {
            float dx = (p->px - fWorldX);
            float dy = (p->py - fWorldY);
            float fDist = std::sqrt(dx * dx + dy * dy);
            if (fDist < 0.001f) fDist = 0.001f;
            if (fDist < fRadius) {
                // now we have to apply force on the object, for which we change its velocity.
                // the new velocity should be in the direction of the distance vector and inversely proportional to the distance
                p->vx = (dx / fDist) * fRadius;
                p->vy = (dy / fDist) * fRadius;
                p->bStable = false;
            }
        }

        for (int i = 0; i < static_cast<int>(fRadius); i++) {
            listObjects.push_back(std::make_unique<cDebris>(fWorldX, fWorldY));
        }
    }

    void createMap() {
        // used in 1D perlin noise
        float *fSurface = new float[nMapWidth];
        float *fNoiseSeed = new float[nMapWidth];

        for (int i = 0; i < nMapWidth; i++) {
            fNoiseSeed[i] = (float) rand() / (float) RAND_MAX;
        }
        fNoiseSeed[0] = 0.5f; // Because we want the terrain to start and end at half the height of the screen
        perlinNoise1D(nMapWidth, fNoiseSeed, 8, 2.0f, fSurface);
        for (int y = 0; y < nMapHeight; y++) {
            for (int x = 0; x < nMapWidth; x++) {
                // if the current pixel in map is greater than corresponding pixel in noise output
                // we set it to 1 (land) (y=0 is at top)
                if (y > fSurface[x] * nMapHeight) {
                    map[y * nMapWidth + x] = 1;
                } else {
                    map[y * nMapWidth + x] = 0;
                }
            }
        }
        delete[] fNoiseSeed;
        delete[] fSurface;

    }

    // Taken from Perlin Noise Video https://youtu.be/6-0UaeJBumA
    void perlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput) {
        // Used 1D Perlin Noise
        for (int x = 0; x < nCount; x++) {
            float fNoise = 0.0f;
            float fScaleAcc = 0.0f;
            float fScale = 1.0f;

            for (int o = 0; o < nOctaves; o++) {
                int nPitch = nCount >> o;
                int nSample1 = (x / nPitch) * nPitch;
                int nSample2 = (nSample1 + nPitch) % nCount;
                float fBlend = (float) (x - nSample1) / (float) nPitch;
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
