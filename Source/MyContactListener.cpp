//
// Created by guilh on 06/12/2023.
//

#include "MyContactListener.h"
#include "Actors/Liquid.h"

void MyContactListener::BeginContact(b2Contact* contact) {
    auto* bodyA = &contact->GetFixtureA()->GetBody()->GetUserData();
    auto* bodyB = &contact->GetFixtureB()->GetBody()->GetUserData();
    auto a = (WorldBodyComponent*)(bodyA->pointer);
    auto b = (WorldBodyComponent*)(bodyB->pointer);
    if(a && b) {
        if(isPlayerOnGround(a, b) || isPlayerOnBox(a, b) || isPlayerOnBall(a, b)) {
            if(a->GetType() == BodyTypes::Player){
                a->SetIsOnGround(true);
                a->cont_Collision++;
            }
            else{
                b->SetIsOnGround(true);
                b->cont_Collision++;
            }
        } else if (isPlayerOnPlatform(a, b)) {
            if (b->GetType() == BodyTypes::Player) {
                if (a->GetPosition().x < b->GetPosition().x + b->GetSize().x &&
                    a->GetPosition().x + a->GetSize().x > b->GetPosition().x &&
                    a->GetPosition().y + a->GetSize().y > b->GetPosition().y &&
                    a->GetPosition().y < b->GetPosition().y + b->GetSize().y) {
                    b->SetIsOnGround(true);
                }
                b->cont_Collision++;
            } else {
                if (b->GetPosition().x < a->GetPosition().x + a->GetSize().x &&
                    b->GetPosition().x + b->GetSize().x > a->GetPosition().x &&
                    b->GetPosition().y + b->GetSize().y > a->GetPosition().y &&
                    b->GetPosition().y < a->GetPosition().y + a->GetSize().y) {
                    a->SetIsOnGround(true);
                }
                a->cont_Collision++;
            }
        } else if (isPlayerOnSensor(a, b)) {
            SensorBodyComponent *sensor;
            WorldBodyComponent *player;
            if (a->GetClass() == "Sensor") {
                sensor = (SensorBodyComponent *) a;
                player = (WorldBodyComponent *) b;
            }
            else{
                sensor = (SensorBodyComponent *) b;
                player = (WorldBodyComponent *) a;
            }
            if(sensor->GetFunction() == "Liquid"){
                auto character = dynamic_cast<class Player*>(player->GetOwner());
                if(character->GetType() == PlayerType::FireBoyHead &&
                    (sensor->GetAffectBody() == "FireBoy" || sensor->GetAffectBody() == "Both")){
                    character->Kill();
                }
                else if(character->GetType() == PlayerType::WaterGirlHead &&
                    (sensor->GetAffectBody() == "WaterGirl" || sensor->GetAffectBody() == "Both")){
                    character->Kill();
                }
            }
        }

    }
}

void MyContactListener::EndContact(b2Contact *contact) {
    auto* bodyA = &contact->GetFixtureA()->GetBody()->GetUserData();
    auto* bodyB = &contact->GetFixtureB()->GetBody()->GetUserData();
    auto a = reinterpret_cast<WorldBodyComponent*>(bodyA->pointer);
    auto b = (WorldBodyComponent*)(bodyB->pointer);
    if(a && b) {
        if(isPlayerOnGround(a, b) || isPlayerOnBox(a, b) || isPlayerOnPlatform(a, b) || isPlayerOnBall(a, b)) {
            if(a->GetType() == BodyTypes::Player){
                a->cont_Collision--;
                if(a->cont_Collision == 0)
                    a->SetIsOnGround(false);
            }
            else{
                b->cont_Collision--;
                if(b->cont_Collision == 0)
                    b->SetIsOnGround(false);
            }
        } else if(isPlayerOnSensor(a, b)) {
            SensorBodyComponent *sensor;
            WorldBodyComponent *player;
            if (a->GetClass() == "Sensor") {
                sensor = (SensorBodyComponent *) a;
                player = (WorldBodyComponent *) b;
            }
            else{
                sensor = (SensorBodyComponent *) b;
                player = (WorldBodyComponent *) a;
            }

        }
    }
}

bool MyContactListener::isPlayerOnGround(WorldBodyComponent *a, WorldBodyComponent *b) {
    if(a->GetType() == BodyTypes::Player || b->GetType() == BodyTypes::Player)
        if(a->GetType() == BodyTypes::Floor || b->GetType() == BodyTypes::Floor)
            return true;
    return false;
}

bool MyContactListener::isPlayerOnBox(WorldBodyComponent *a, WorldBodyComponent *b) {
    if(a->GetType() == BodyTypes::Player || b->GetType() == BodyTypes::Player)
        if(a->GetClass() == "Block" || b->GetClass() == "Block")
            return true;
    return false;
}

bool MyContactListener::isPlayerOnSensor(WorldBodyComponent *a, WorldBodyComponent *b) {
    if(a->GetType() == BodyTypes::Player || b->GetType() == BodyTypes::Player)
        if(a->GetClass() == "Sensor" || b->GetClass() == "Sensor")
            return true;
    return false;
}

bool MyContactListener::isPlayerOnPlatform(WorldBodyComponent *a, WorldBodyComponent *b) {
    if(a->GetType() == BodyTypes::Player || b->GetType() == BodyTypes::Player)
        if(a->GetClass() == "Platform" || b->GetClass() == "Platform")
            return true;
    return false;
}

bool MyContactListener::isPlayerOnBall(WorldBodyComponent *a, WorldBodyComponent *b) {
    if(a->GetType() == BodyTypes::Player || b->GetType() == BodyTypes::Player)
        if(a->GetClass() == "Ball" || b->GetClass() == "Ball")
            return true;
    return false;
}