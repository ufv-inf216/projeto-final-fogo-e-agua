//
// Created by guilh on 05/12/2023.
//

#include "Level.h"
#include "Scene.h"
#include "../Actors/Actor.h"
#include "../Actors/Player.h"
#include "../Actors/Block.h"
#include "../Components/DrawComponents/DrawTileComponent.h"
#include "../Components/WorldBodyComponent.h"
#include "../Components/SensorBodyComponent.h"
#include "../Actors/Liquid.h"

// BOX2D
float TIME_STEP = 1.0f / 60.0f;
const int VELOCITY_ITERATIONS = 8;
const int POSITION_ITERATIONS = 3;

Level::Level(Game *game,std::string layerFileName, std::string objectsFileName)
        : mGame(game),
          Scene(game),
          mLayerFileName(std::move(layerFileName)),
          mObjectsFileName(std::move(objectsFileName)),
          mFireboy(nullptr),
          mWatergirl(nullptr),
          mWorld(nullptr),
          tf(new Transform()),
          mContactListener(new MyContactListener()),
          mIsPaused(false)
{
}

Level::~Level()
{
    delete mWorld;
    delete mFireboy;
    delete mWatergirl;
    delete tf;
    delete mContactListener;
    while(!mWorldColliders.empty()){
        delete mWorldColliders.back();
    }
    while(!mBodies.empty()){
        delete mBodies.back();
    }
    while(!mActors.empty()){
        delete mActors.back();
    }
}

void Level::Load(){
    InicializeLevel();
}

void Level::InicializeLevel() {
    b2Vec2 gravity(0.0f, -40.0f);
    mWorld = new b2World(gravity);
    mWorld->SetContactListener(mContactListener);

    // Tiled
    auto* map = new Actor(GetGame());

    new DrawTileComponent(map, mLayerFileName, "../Assets/Maps/finalBlocks.png", 800, 800, 32);
    LoadData(mObjectsFileName);
    mGame->GetSound()->PlaySound("Level Music.mp3", true);
}

void Level::UpdateLevel(float deltaTime) {
    if(!mGame->GetStatePaused() && !mGame->GetStateDead() && !mGame->GetStateWin()) {
        if(mIsPaused){
            mIsPaused = false;
            delete pausedImage;
        }

        GetWorld()->Step(deltaTime, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

        for (auto &body: mBodies) {
            body->Update();
        }

        for (auto &platform: mPlatforms) {
            platform->OnUpdate();
            platform->SetPosition(platform->GetBodyComponent()->GetPosition());
        }

        for (auto &mActor: mActors) {
            auto actor = dynamic_cast<Block *>(mActor);
            actor->SetPosition(actor->GetBodyComponent()->GetPosition());
        }
        if (!GetGame()->GetStateWin()) {
            mWatergirl->GetBodyComponent()->Update();
            mWatergirl->SetPosition(mWatergirl->GetBodyComponent()->GetPosition());
            mFireboy->GetBodyComponent()->Update();
            mFireboy->SetPosition(mFireboy->GetBodyComponent()->GetPosition());
        }
    }
    else if(!mIsPaused){
        if(mGame->GetStatePaused()) {
            mIsPaused = true;
            auto aux = new Actor(GetGame());
            pausedImage = new DrawSpriteComponent(aux, "../Assets/Sprites/Menu/Paused.png", 800, 800, 1000);
            aux->SetPosition(Vector2(0, 0));
        }
    }

}

void Level::LoadData(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        SDL_Log("Failed to load paths: %s", fileName.c_str());
    }

    int row = 0;

    std::string line;
    while (!file.eof())
    {
        std::getline(file, line);

        if(!line.empty())
        {
            auto tiles = CSVHelper::Split(line);
            if(tiles[0] == "Player"){
                std::string name = tiles[1];
                if(name == "FireBoy")
                {
                    mFireboy = new class Player(GetGame(), line, GetWorld(), PlayerType::FireBoyHead, tf);
                    mFireboy->SetPosition(mFireboy->GetBodyComponent()->GetPosition());
                }
                else if(name == "WaterGirl")
                {
                    mWatergirl = new class Player(GetGame(), line, GetWorld(), PlayerType::WaterGirlHead, tf);
                    mWatergirl->SetPosition(mWatergirl->GetBodyComponent()->GetPosition());
                }
            } else if(tiles[0] == "Platform"){
                auto newPlatform = new class Platform(GetGame(), line, GetWorld(), tf);
                mPlatforms.push_back(newPlatform);
                mBodies.push_back(mPlatforms.back()->GetBodyComponent());
                /*
                } else {
                    int id = tiles[1][1] - '0';
                    auto myPlatform = mPlatforms[id];
                    auto mySensor = new SensorBodyComponent("Platform", "Platform", line, GetWorld(), tf, myPlatform);
                    if(tiles[1][2] == 'T') myPlatform->SetTopSensor(mySensor);
                    else myPlatform->SetBottomSensor(mySensor);
                    mBodies.push_back(mySensor);
                }
                */
            } else if(tiles[0] == "Block" || tiles[0] == "Ball"){
                auto w = std::stoi(tiles[4]);
                auto h = std::stoi(tiles[5]);

                auto myBlock = new Block(GetGame(), "Blocks/" + tiles[0], w, h);
                auto block = new WorldBodyComponent(line, GetWorld(), tf, myBlock);
                mBodies.push_back(block);
                myBlock->SetPosition(block->GetPosition());
                myBlock->SetBodyComponent(block);
                mActors.push_back(myBlock);
            } else if(tiles[0] == "Sensor") {
                auto w = std::stoi(tiles[4]);
                auto h = std::stoi(tiles[5]);
                std::string type = tiles[1][0] == 'S' ? "Portal" : tiles[1][0] == 'D' ? "Diamond" : tiles[1][0] == 'L' ? "Liquid" : "None";
                std::string affect = tiles[1][1] == 'F' ? "FireBoy" : tiles[1][1] == 'W' ? "WaterGirl" : "Both";
                if(type == "Liquid"){
                    std::string orientation = tiles[1][2] == 'R' ? "Right" : tiles[1][2] == 'L' ? "Left" : "Center";
                    auto myLiquid = new Liquid(type, affect, orientation, GetGame(), line, GetWorld(), tf);
                    myLiquid->SetPosition(Vector2(std::stoi(tiles[2]),std::stoi(tiles[3])));
                    mBodies.push_back(myLiquid->GetBodyComponent());
                } else {
                    Block* myBlock;
                    if(type == "Portal" && affect == "FireBoy")
                        myBlock = new Block(GetGame(), "Temple/DoorFire", w, h);
                    else if(type == "Portal" && affect == "WaterGirl")
                        myBlock = new Block(GetGame(), "Temple/DoorWater", w, h);
                    else if(type == "Diamond" && affect == "FireBoy")
                        myBlock = new Block(GetGame(), "Characters/DiamondFire", w, h);
                    else if(type == "Diamond" && affect == "WaterGirl")
                        myBlock = new Block(GetGame(), "Characters/DiamondWater", w, h);
                    myBlock->SetBodyComponent(new SensorBodyComponent(type, affect, line, GetWorld(), tf, myBlock));
                    myBlock->SetPosition(myBlock->GetBodyComponent()->GetPosition());
                    mBodies.push_back(myBlock->GetBodyComponent());
                }
            }
            else {
                mBodies.push_back(new WorldBodyComponent(line, GetWorld(), tf));
            }
        }
    }
}

void Level::DrawColliders(SDL_Renderer *renderer){

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for(int i=0; i<mBodies.size(); i++){
        if(mBodies[i]->GetClass() == "Ramp") continue;
        auto collider = mBodies[i]->GetBody();
        Vector2 size, position;
        if(mBodies[i]->GetClass() == "Ball"){
            auto shape = dynamic_cast<b2CircleShape*>(collider->GetFixtureList()->GetShape());
            float radius = shape->m_radius;
            position = tf->posWorldToMap(collider->GetPosition(), b2Vec2(2 * radius, 2 * radius));
            size = Vector2(2*radius*32, 2*radius*32);
        } else {
            auto groundBox = dynamic_cast<b2PolygonShape*>(collider->GetFixtureList()->GetShape());
            b2Vec2 worldSize = groundBox->m_vertices[2] - groundBox->m_vertices[0];
            position = tf->posWorldToMap(collider->GetPosition(), worldSize);
            size = Vector2(worldSize.x*32, worldSize.y*32);
        }

        std::vector<Vector2> vertices;
        vertices.push_back(position);
        vertices.push_back(position + Vector2(0, size.y));
        vertices.push_back(position + Vector2(size.x, 0));
        vertices.push_back(position+size);

        // Render vertices as lines
        for(int i = 0; i < vertices.size() - 1; i++) {
            SDL_RenderDrawLine(renderer, (int)vertices[i].x,
                               (int)vertices[i].y,
                               (int)vertices[i+1].x,
                               (int)vertices[i+1].y);
        }

        // Close geometry
        SDL_RenderDrawLine(renderer, (int)vertices[vertices.size() - 1].x,
                           (int)vertices[vertices.size() - 1].y,
                           (int)vertices[0].x,
                           (int)vertices[0].y);
    }

    for(auto &v: mRamps){
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for(int i = 0; i < v.size() - 1; i++) {
            SDL_RenderDrawLine(renderer, (int)v[i].x,
                               (int)v[i].y,
                               (int)v[i+1].x,
                               (int)v[i+1].y);
        }

        // Close geometry
        SDL_RenderDrawLine(renderer, (int)v[v.size() - 1].x,
                           (int)v[v.size() - 1].y,
                           (int)v[0].x,
                           (int)v[0].y);
    }
}