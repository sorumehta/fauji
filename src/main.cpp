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

    virtual bool Damage(float d) = 0;
};

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

    bool Damage(float d) override {
        return true; // Cannot be damaged
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

    bool Damage(float d) override {
        return true; // Cannot be damaged
    }

    int ObjDeadAction() override;


private:
    static std::vector<std::pair<float, float>> vecModel;
};

int cMissile::ObjDeadAction() {
    return 30;
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
    float fShootingAngle = 0.0f;
    float fHealth = 1.0f;
    bool bIsPlayable = true;
    int nTeam = 0;
    static LTexture *spritePtr; // shared across instances
    static LTexture *tombSpritePtr;
    SDL_Rect spriteClips[4];
    int frame;

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
            spritePtr->loadTextureFromFile("../res/man.png");
        }
        if (tombSpritePtr == nullptr) {
            tombSpritePtr = new LTexture();
            tombSpritePtr->loadTextureFromFile("../res/tomb-removebg.png");
        }
    }

    virtual bool Damage(float d) // Reduce worm's health by said amount
    {
        fHealth -= d;
        if (fHealth <= 0) { // Worm has died, no longer playable
            fHealth = 0.0f;
            bIsPlayable = false;
        }
        return fHealth > 0;
    }

    int ObjDeadAction() override;

    void draw(GameEngine *engine, float fOffsetX, float fOffsetY) override;

};

LTexture *cMan::spritePtr = nullptr;
LTexture *cMan::tombSpritePtr = nullptr;

void cMan::draw(GameEngine *engine, float fOffsetX, float fOffsetY) {
    if (bIsPlayable) {
        SDL_Rect *currentClip = nullptr;
        if (std::abs(vx) > 4 && std::abs(vx) < 6) {
            currentClip = &spriteClips[frame / 2];
        } else {
            currentClip = &spriteClips[0];
        }
        spritePtr->drawTexture(px - fOffsetX - radius, py - fOffsetY - radius, radius * 2, radius * 2, currentClip, 0,
                               NULL,
                               flipType);
        frame++;
        if (frame / 2 >= 4) {
            frame = 0;
        }
        Color healthColor = {};
        if (nTeam == 0) {
            healthColor = {0, 0, 0xFF};
        } else {
            healthColor = {0xFF, 0, 0};
        }

        // draw health bar
        for (int i = 0; i < 22 * fHealth; i++) {
            engine->drawPoint(px - 5 + i - fOffsetX, py - 20 - fOffsetY, healthColor);
            engine->drawPoint(px - 5 + i - fOffsetX, py - 21 - fOffsetY, healthColor);
            engine->drawPoint(px - 5 + i - fOffsetX, py - 22 - fOffsetY, healthColor);
            engine->drawPoint(px - 5 + i - fOffsetX, py - 23 - fOffsetY, healthColor);
        }
    } else {
        tombSpritePtr->drawTexture(px - fOffsetX - radius, py - fOffsetY - radius, radius * 2, radius * 2);
    }

}

int cMan::ObjDeadAction() {
    return 0;
}

class cTeam {
public:
    std::vector<cMan *> vecMembers;
    int nCurrentMember = 0;
    int nTeamSize = 0;

    bool isTeamStillAlive() {
        bool bAllDead = false;
        for (auto w: vecMembers) {
            bAllDead |= (w->fHealth > 0);
        }
        return bAllDead;
    }

    cMan *getNextMember() {
        do {
            nCurrentMember++;
            if (nCurrentMember >= nTeamSize) nCurrentMember = 0;
        } while (vecMembers[nCurrentMember]->fHealth <= 0);
        return vecMembers[nCurrentMember];
    }
};

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
    float fEnergyLevel = 0;
    float fOldEnergyLevel = 0;
    bool bFireWeapon = false;
    bool bShowCountDown = false;
    bool bShowNukeAnimation = false;
    int planePosX = 0;
    LTexture planeTexture;
    float fTurnTime = 0.0f;
    bool bGameIsStable = false;
    bool bPlayerHasControl = false;
    bool bComputerHasControl = false;
    bool bPlayerActionComplete = false;
    float fTimeSinceEnergyLevelSet = 0;
    // Vector to store teams
    std::vector<cTeam> vecTeams;
    // Current team being controlled
    int nCurrentTeam = 0;

    // AI control flags
    bool bAI_AimLeft = false;            // AI has pressed "AIM_LEFT" key
    bool bAI_AimRight = false;            // AI has pressed "AIM_RIGHT" key
    bool bAI_Energise = false;            // AI has pressed "FIRE" key
    bool bAI_Walk = false;              // AI is walking, which can include jumping if necessary
    bool bAI_Jump = false;
    bool bAI_Flipped = false;           // AI is facing left

    float fAITargetAngle = 0.0f;        // Angle AI should aim for
    float fAITargetEnergy = 0.0f;        // Energy level AI should aim for
    float fAISafePosition = 0.0f;        // X-Coordinate considered safe for AI to move to
    cMan *pAITargetMan = nullptr;        // Pointer to worm AI has selected as target
    float fAITargetX = 0.0f;            // Coordinates of target missile location
    float fAITargetY = 0.0f;
    std::string gameOverMessage;
    enum GAME_STATE {
        GS_RESET = 0,
        GS_GENERATE_TERRAIN = 1,
        GS_GENERATING_TERRAIN,
        GS_ALLOCATE_UNITS,
        GS_ALLOCATING_UNITS,
        GS_START_PLAY,
        GS_CAMERA_MODE,
        GS_GAME_OVER,
        GS_NUKE,
        GS_GAME_OVER2
    } nGameState, nNextState;

    enum AI_STATE {
        AI_ASSESS_ENVIRONMENT = 0,
        AI_MOVE,
        AI_CHOOSE_TARGET,
        AI_POSITION_FOR_TARGET,
        AI_AIM,
        AI_FIRE,
    } nAIState, nAINextState;

public:
    void walkManRight(cMan *pMan) {
        pMan->vx = 5.0f;
        pMan->vy = -5.0f;
        pMan->flipType = SDL_FLIP_HORIZONTAL;
        pMan->fShootingAngle = PI / 2;
        pMan->bStable = false;
    }

    void walkManLeft(cMan *pMan) {
        pMan->vx = -5.0f;
        pMan->vy = -5.0f;
        pMan->flipType = SDL_FLIP_NONE;
        pMan->fShootingAngle = PI / 2;
        pMan->bStable = false;
    }

    void manJump(cMan *pMan) {
        pMan->vx = 3.0f * (pMan->flipType == SDL_FLIP_NONE ? -1.0f : 1.0f);
        pMan->vy = -15.0f;
    }

    void aimLeft(cMan *pMan, float secPerFrame) {
        pMan->fShootingAngle -= 1.0f * secPerFrame;
        if (pMan->flipType != SDL_FLIP_NONE) {
            if (pMan->fShootingAngle < -PI / 2) {
                pMan->fShootingAngle = PI / 2;
            }
        } else {
            if (pMan->fShootingAngle < -PI) {
                pMan->fShootingAngle = PI;
            } else if (pMan->fShootingAngle > 0 && pMan->fShootingAngle < PI / 2) {
                pMan->fShootingAngle = -PI / 2;
            }
        }
    }

    void aimRight(cMan *pMan, float secPerFrame) {
        pMan->fShootingAngle += 1.0f * secPerFrame;
        if (pMan->flipType != SDL_FLIP_NONE) {
            if (pMan->fShootingAngle > PI / 2) {
                pMan->fShootingAngle = -PI / 2;
            }
        } else {
            if (pMan->fShootingAngle > PI) {
                pMan->fShootingAngle = -PI;
            } else if (pMan->fShootingAngle > -PI / 2 && pMan->fShootingAngle < 0) {
                pMan->fShootingAngle = PI / 2;
            }
        }
    }

    void energize(float secPerFrame) {
        fEnergyLevel += 0.75f * secPerFrame;
        if (fEnergyLevel > 1.0f) {
            fEnergyLevel = 1.0f;
        }
    }

    void onUserInputEvent(int eventType, int button, int mousePosX, int mousePosY, float secPerFrame) {
        if (!bPlayerHasControl) {
            return;
        }
        if (eventType == SDL_KEYDOWN) {
            if (pObjectUnderControl != nullptr) {
                if (pObjectUnderControl->bStable) {
                    cMan *pMan = dynamic_cast<cMan *>(pObjectUnderControl);
                    if (button == SDLK_RIGHT) {
                        walkManRight(pMan);
                    } else if (button == SDLK_LEFT) {
                        walkManLeft(pMan);
                    } else if (button == SDLK_UP) {
                        manJump(pMan);
                    } else if (button == SDLK_a) {
                        aimLeft(pMan, secPerFrame);
                    } else if (button == SDLK_s) {
                        aimRight(pMan, secPerFrame);
                    } else if (button == SDLK_SPACE) {
                        energize(secPerFrame);
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
        // music: Battle Of The Dragons by TommyMutiu from Pixabay
        loadMusic("../res/battle-of-the-dragons.mp3");
        playMusic();
        planeTexture.loadTextureFromFile("../res/fighter-jet.png");
        return true;
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
                int nTeams = 2;
                int nMembersPerTeam = 2;
                float fSpacePerTeam = (float) nMapWidth / (float) nTeams;
                float fSpacePerMember = (float) fSpacePerTeam / ((float) nMembersPerTeam * 2.0f);

                // create teams
                for (int t = 0; t < nTeams; t++) {
                    vecTeams.emplace_back(cTeam());
                    float fTeamMiddle = ((fSpacePerTeam) / 2.0f) + (t * fSpacePerTeam);
                    for (int w = 0; w < nMembersPerTeam; w++) {
                        float fManX =
                                fTeamMiddle - ((fSpacePerMember * (float) nMembersPerTeam) / 2) + w * fSpacePerMember;;
                        float fManY = 0.0f;

                        // add members to teams
                        cMan *man = new cMan(fManX, fManY);
                        man->nTeam = t;
                        listObjects.push_back(std::unique_ptr<cMan>(man));
                        vecTeams[t].vecMembers.push_back(man);
                        vecTeams[t].nTeamSize = nMembersPerTeam;
                    }
                }
                // Select players first man for control and camera tracking
                pObjectUnderControl = vecTeams[0].vecMembers[vecTeams[0].nCurrentMember];
                pCameraTrackingObject = pObjectUnderControl;
                bShowCountDown = false;
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
                bShowCountDown = true;
                // clamp the players so that they don't walk off map
                for (auto t: vecTeams) {
                    for (auto m: t.vecMembers) {
                        if (m->px < 5) m->px = 5;
                        if (m->px > nMapWidth - 5) m->px = nMapWidth - 5;
                    }
                }
                // if any player has gone off screen, bring him back
                if (bPlayerActionComplete || fTurnTime <= 0.0f) {
                    nNextState = GS_CAMERA_MODE;
                }
            }
                break;
            case GS_CAMERA_MODE: {
                bPlayerHasControl = false;
                bPlayerActionComplete = false;
                bComputerHasControl = false;
                bShowCountDown = false;
                fEnergyLevel = 0;
                if (bGameIsStable) {
                    // get next team
                    int nOldTeam = nCurrentTeam;
                    do {
                        nCurrentTeam++;
                        nCurrentTeam %= vecTeams.size();
                    } while (!vecTeams[nCurrentTeam].isTeamStillAlive());

                    // lock control if AI team is playing
                    if (nCurrentTeam == 0) { // player team
                        bPlayerHasControl = true;
                        bComputerHasControl = false;
                    } else {
                        bPlayerHasControl = false;
                        bComputerHasControl = true;
                    }
                    nNextState = GS_START_PLAY;
                    pObjectUnderControl = vecTeams[nCurrentTeam].getNextMember();
                    pCameraTrackingObject = pObjectUnderControl;
                    fTurnTime = 15.0f;

                    // if it is the same team, current team won
                    if (nCurrentTeam == nOldTeam) {
                        nNextState = GS_GAME_OVER;
                    }
                }
            }
                break;
            case GS_GAME_OVER: {
                bComputerHasControl = false;
                bPlayerHasControl = false;
                bShowCountDown = false;
                if (nCurrentTeam == 0){
                    gameOverMessage = "Good job soldier, you saved us a nuke bomb!";
                }
                else {
                    gameOverMessage = "The battle is lost. Executing Plan B (nuke) ...";

                    nNextState = GS_NUKE;
                }
            }
            break;
            case GS_NUKE: {
                bComputerHasControl = false;
                bPlayerHasControl = false;
                bShowCountDown = false;
                bShowNukeAnimation = true;
                for (int i = 0; i < 100; i ++)
                {
                    int nBombX = rand() % nMapWidth;
                    int nBombY = rand() % (nMapHeight / 2);
                    listObjects.push_back(std::unique_ptr<cMissile>(new cMissile(nBombX, nBombY, 0.0f, 0.5f)));
                }
                for (auto t : vecTeams[nCurrentTeam].vecMembers){
                    t->fHealth = 0.0f;
                }
                nNextState = GS_GAME_OVER2;
            }
                break;
            case GS_GAME_OVER2: {
            }
                break;
        }

        if (bComputerHasControl) {
            cMan *origin = nullptr;
            switch (nAIState) {
                case AI_ASSESS_ENVIRONMENT: {
                    int nAction = rand() % 3;
                    origin = (cMan *) pObjectUnderControl;
                    if (nAction == 0) { // Move away from the team
                        float fNearestAllyDistance = INFINITY;
                        float fDirection = 0;

                        for (auto w: vecTeams[nCurrentTeam].vecMembers) {
                            fNearestAllyDistance = std::fabs(w->px - origin->px);
                            fDirection = (w->px - origin->px) < 0 ? 1.0f : -1.0f;
                        }
                        if (fNearestAllyDistance < 50.0f) {
                            fAISafePosition = origin->px + fDirection * 80.0f;
                        } else {
                            fAISafePosition = origin->px;
                        }

                    } else if (nAction == 1) {

                        float fDirection = ((float) (nMapWidth / 2.0f) - origin->px) < 0.0f ? 1.0f : -1.0f;
                        fAISafePosition = origin->px + fDirection * 200;
                    } else if (nAction == 2) {
                        origin = (cMan *) pObjectUnderControl;
                        fAISafePosition = origin->px;
                    }
                    // clamp position
                    if (fAISafePosition <= 20.0f) fAISafePosition = 20.0f;
                    if (fAISafePosition >= nMapWidth - 20.0f) fAISafePosition = nMapWidth - 20.0f;

                    nAINextState = AI_MOVE;
                }
                    break;
                case AI_MOVE: {
                    origin = (cMan *) pObjectUnderControl;
                    if (fTurnTime >= 10.0f && std::abs(fAISafePosition - origin->px) > 1.0f) {
                        if (bGameIsStable) {
                            // walk towards the target
                            if (fAISafePosition < origin->px) {
                                bAI_Flipped = false;
                            } else {
                                bAI_Flipped = true;
                            }
                            bAI_Walk = true;
                            nAINextState = AI_MOVE;
                        }
                    } else if(fTurnTime < 10.0f && fTurnTime >= 9.0f && std::abs(fAISafePosition - origin->px) > 1.0f) {
                        bAI_Walk = false;
                        bAI_Jump = true;
                    }
                    else {
                        nAINextState = AI_CHOOSE_TARGET;
                    }
                }
                    break;
                case AI_CHOOSE_TARGET: {
                    bAI_Walk = false;
                    bAI_Jump = false;
                    origin = (cMan *) pObjectUnderControl;
                    nCurrentTeam = origin->nTeam;
                    int nTargetTeam = 0;
                    int i = 0;

                    for(i=0; i <= 5; i++){
                        nTargetTeam = rand() % vecTeams.size();
                        if(nTargetTeam != nCurrentTeam && vecTeams[nTargetTeam].isTeamStillAlive()){
                            break;
                        }
                    }
                    if(i > 5){
                        do {
                            nTargetTeam++;
                            nTargetTeam %= vecTeams.size();
                        } while (!vecTeams[nCurrentTeam].isTeamStillAlive());
                        if(nTargetTeam == nCurrentTeam){
                            nNextState = GS_GAME_OVER;
                        }
                    }
                    // aim for the healthiest opponent
                    cMan *mostHealthy = vecTeams[nTargetTeam].vecMembers[0];
                    for (auto w: vecTeams[nTargetTeam].vecMembers) {
                        if (w->fHealth > mostHealthy->fHealth) {
                            mostHealthy = w;
                        }
                    }
                    pAITargetMan = mostHealthy;
                    fAITargetX = pAITargetMan->px;
                    fAITargetY = pAITargetMan->py;
                    if (fAITargetX < origin->px) {
                        bAI_Flipped = false;
                    } else {
                        bAI_Flipped = true;
                    }
                    nAINextState = AI_POSITION_FOR_TARGET;
                }
                    break;

                case AI_POSITION_FOR_TARGET: {
                    origin = (cMan *) pObjectUnderControl;
                    float dy = origin->py - fAITargetY;
                    float dx = origin->px - fAITargetX;
                    float fSpeed = 30.0f;
                    float fGravity = 2.0f;
                    bAI_Walk = false;
                    bAI_Jump = false;
                    // taken from wiki article: https://en.wikipedia.org/wiki/Projectile_motion
                    float a = fSpeed * fSpeed * fSpeed * fSpeed -
                                  fGravity * (fGravity * dx * dx + 2.0f * dy * fSpeed * fSpeed);
                    if(bGameIsStable) {
                        if (a < 0) { // target is out of range
                            if (fTurnTime > 7) {
                                // walk towards the target
                                if (pAITargetMan->px < origin->px) {
                                    bAI_Flipped = false;
                                } else {
                                    bAI_Flipped = true;
                                }
                                bAI_Walk = true;
                                nAINextState = AI_POSITION_FOR_TARGET;
                            } else if (fTurnTime <= 7 && fTurnTime > 6) {
                                // if still hasn't reached, maybe its stuck. Try jumping
                                bAI_Jump = true;
                                nAINextState = AI_POSITION_FOR_TARGET;
                            } else {
                                // fire from wherever you are
                                fAITargetAngle = bAI_Flipped ? -(PI/2.0f) + (PI/4.0f) : -(PI/2.0f) - (PI/4.0f);
                                fAITargetEnergy = 0.75f;
                                nAINextState = AI_AIM;
                            }
                        } else { // target is in range
                            // calculate the trajectory, based on wiki link given above
                            float b1 = fSpeed * fSpeed + sqrtf(a);
                            float b2 = fSpeed * fSpeed - sqrtf(a);

                            float fTheta1 = atanf(b1 / (fGravity * dx)); // Max Height
                            float fTheta2 = atanf(b2 / (fGravity * dx)); // Min Height

                            // we'll use max as it has greater chance of avoiding obstacles
                            fAITargetAngle = fTheta1 - (dx > 0 ? PI : 0.0f);
                            fAITargetEnergy = 0.75f;
                            nAINextState = AI_AIM;
                        }
                    }
                }
                    break;
                case AI_AIM: {
                    origin = (cMan *)pObjectUnderControl;
                    bAI_Walk = false;
                    bAI_Jump = false;

                    if(origin->fShootingAngle < fAITargetAngle){
                        bAI_AimRight = true;
                    } else {
                        bAI_AimLeft = true;
                    }
                    if(std::abs(origin->fShootingAngle - fAITargetAngle) < 0.075f){
                        bAI_AimLeft = false;
                        bAI_AimRight = false;
                        fEnergyLevel = 0.0f;
                        nAINextState = AI_FIRE;
                    } else {
                        nAINextState = AI_AIM;
                    }
                }
                break;
                case AI_FIRE: {
                    origin = (cMan *)pObjectUnderControl;
                    bAI_Energise = true;
                    if(fEnergyLevel >= fAITargetEnergy){
                        bAI_Energise = false;
                        bComputerHasControl = false;
                        nAINextState = AI_ASSESS_ENVIRONMENT;
                    }
                }
                break;
            }
            if (origin != nullptr && origin->bStable) {
                origin->flipType = bAI_Flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
                if (bAI_Walk && bAI_Flipped) {
                    //TODO: if collision then jump otherwise walk
                    walkManRight(origin);
                } else if (bAI_Walk && !bAI_Flipped) {
                    walkManLeft(origin);
                } else if (bAI_Jump) {
                    manJump(origin);
                } else if(bAI_AimLeft){
                    aimLeft(origin, fElapsedTime);
                } else if(bAI_AimRight){
                    aimRight(origin, fElapsedTime);
                } else if(bAI_Energise){
                    energize(fElapsedTime);
                }
            }

        }

        fTurnTime -= fElapsedTime;
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
                    // ????=???????2(???????????)????


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
                float fMapVal = map[static_cast<int>(std::round(
                        (y + fCameraPosY) * nMapWidth + (x + fCameraPosX)))];
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

            // fire the weapon if the energy level is set to a specific value for 1 second
            if (fEnergyLevel > 0.0f) {
                if (fEnergyLevel != fOldEnergyLevel) {
                    fOldEnergyLevel = fEnergyLevel;
                    fTimeSinceEnergyLevelSet = 0;
                } else {
                    fTimeSinceEnergyLevelSet += fElapsedTime;
                    if (fTimeSinceEnergyLevelSet > 1) {
                        bFireWeapon = true;
                    }
                }

            } else {
                fTimeSinceEnergyLevelSet = 0;
            }

            if (bFireWeapon) {
                const float fMagFireVelocity = 40;
                cMissile *missile = new cMissile(pMan->px, pMan->py, fMagFireVelocity * fEnergyLevel * dx,
                                                 fMagFireVelocity * fEnergyLevel * dy);
                listObjects.push_back(std::unique_ptr<cMissile>(missile));
                pCameraTrackingObject = missile;
                bFireWeapon = false;
                fEnergyLevel = 0.0f;
                fTimeSinceEnergyLevelSet = 0;
                bPlayerActionComplete = true;
            }

            const int aimLength = 30;
            int cx = static_cast<int>(aimLength * dx + pMan->px - fCameraPosX);
            int cy = static_cast<int>(aimLength * dy + pMan->py - fCameraPosY);
            // draw missile aim
            fillRect(cx, cy, 4, 4, {0xFF, 0, 0});

            // draw timer
            if (bShowCountDown) {
                LTexture timerTexture;
                timerTexture.loadTextureFromText(std::to_string(static_cast<int>(fTurnTime)), {0, 0, 0});
                timerTexture.drawTexture(3, 6);
            }
            if(gameOverMessage.length() != 0){
                LTexture gameOverTexture;
                gameOverTexture.loadTextureFromText(gameOverMessage, {0, 0, 0});
                gameOverTexture.drawTexture((mWindowWidth/2 - gameOverTexture.getWidth())/2, mWindowHeight/2 - 20);
            }
            if(bShowNukeAnimation){
                planeTexture.drawTexture(planePosX, 40, planeTexture.getWidth()/4.0f, planeTexture.getHeight()/4.0f);
                planePosX += 10;
                if(planePosX > mWindowWidth - 10){
                    bShowNukeAnimation = false;
                }

            }
            // draw energy bar
            for (int i = 0; i < 22 * fEnergyLevel; i++) {
                drawPoint(pMan->px - 5 + i - fCameraPosX, pMan->py + 20 - fCameraPosY, {0xFF, 0, 0xFF});
                drawPoint(pMan->px - 5 + i - fCameraPosX, pMan->py + 21 - fCameraPosY, {0xFF, 0, 0xFF});
                drawPoint(pMan->px - 5 + i - fCameraPosX, pMan->py + 22 - fCameraPosY, {0xFF, 0, 0xFF});
                drawPoint(pMan->px - 5 + i - fCameraPosX, pMan->py + 23 - fCameraPosY, {0xFF, 0, 0xFF});
            }
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
//        if (bGameIsStable) {
//            fillRect(4, 4, 10, 10, {0xFF, 0, 0});
//        }
        nGameState = nNextState;
        nAIState = nAINextState;
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
                p->Damage(((fRadius - fDist) / fRadius) * 0.8f);
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
