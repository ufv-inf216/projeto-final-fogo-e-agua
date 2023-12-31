//
// Created by guilh on 01/12/2023.
//

#include "WorldBodyComponent.h"
#include <iostream>
#include "../CSV.h"

WorldBodyComponent::WorldBodyComponent(const std::string &line, b2World* world, Transform* transform, Actor* owner, float runVelocity, float jumpVelocity)
    :   mOwner(owner),  tf(transform), mRunVelocity(runVelocity), mJumpVelocity(jumpVelocity), mIsOnGround(false), mWorld(world), cont_Collision(0)
{
    auto tiles = CSVHelper::Split(line);
    float x = std::stof(tiles[2]);
    float y = std::stof(tiles[3]);
    Vector2 pos(x, y);

    mClass = tiles[0];
    if(tiles[1] == "WaterGirl" || tiles[1] == "FireBoy") mType = BodyTypes::Player;
    if(tiles[1] == "Wall") mType = BodyTypes::Wall;
    if(tiles[1] == "Floor" || tiles[1] == "Block") mType = BodyTypes::Floor;
    if(tiles[1] == "Ceiling") mType = BodyTypes::Ceiling;
    if(tiles[0] == "Platform") mType = BodyTypes::Platform;
    if(tiles[1] == "Portal") mType = BodyTypes::Sensor;

    if(tiles[0] == "Ramp") {
        b2Vec2 vertices[3];
        vertices[0] = tf->pointMapToWorld(std::stof(tiles[4])+x, std::stof(tiles[5])+y);
        vertices[1] = tf->pointMapToWorld(std::stof(tiles[6])+x, std::stof(tiles[7])+y);
        vertices[2] = tf->pointMapToWorld(std::stof(tiles[8])+x, std::stof(tiles[9])+y);

        b2BodyDef rampBodyDef;
        mBody = GetWorld()->CreateBody(&rampBodyDef);
        b2PolygonShape shape;
        shape.Set(vertices, 3);
        mBody->CreateFixture(&shape, 1.0f);
        mBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

        auto fixtureDef = new b2FixtureDef();
        fixtureDef->shape = &shape;
        fixtureDef->density = 1.0f;
        fixtureDef->friction = 0.0f;
        fixtureDef->restitution = 0.0f;
        fixtureDef->filter.categoryBits = BodyTypes::Floor;
        fixtureDef->filter.maskBits = BodyTypes::Player;
        mBody->CreateFixture(fixtureDef);
    } else {
        float width = std::stof(tiles[4]);
        float height = std::stof(tiles[5]);
        Vector2 size(width, height);

        if(tiles[0] == "Platform") {
            b2BodyDef bodyDef;
            bodyDef.type = b2_kinematicBody;
            bodyDef.position = tf->posMapToWorld(pos, size);
            b2Body* kinematicBody = GetWorld()->CreateBody(&bodyDef);
            kinematicBody->SetLinearVelocity(b2Vec2(0.0f, 1.0f));

            b2PolygonShape kinematicBox;
            b2Vec2 worldSize(tf->sizeMapToWorld(size.x), tf->sizeMapToWorld(size.y));
            kinematicBox.SetAsBox(worldSize.x, worldSize.y);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &kinematicBox;
            fixtureDef.filter.categoryBits = BodyTypes::Player | BodyTypes::Floor;
            kinematicBody->CreateFixture(&fixtureDef);
            mBody = kinematicBody;
            mBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
        }
        else if(tiles[0] == "Ball") {
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.fixedRotation = true;
            bodyDef.position = tf->posMapToWorld(pos, size);
            b2Body* dynamicBody = GetWorld()->CreateBody(&bodyDef);

            b2CircleShape dynamicCircle;
            float radius = size.x/64.0f;
            dynamicCircle.m_radius = radius;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &dynamicCircle;
            fixtureDef.density = 3.0f;
            fixtureDef.friction = 0.8f;
            fixtureDef.restitution = 0.0f;
            fixtureDef.filter.categoryBits = BodyTypes::Player | BodyTypes::Floor;
            dynamicBody->CreateFixture(&fixtureDef);
            mBody = dynamicBody;
            mBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
        }
        else if(tiles[0] == "Block")
        {
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.fixedRotation = true;
            bodyDef.position = tf->posMapToWorld(pos, size);
            b2Body* dynamicBody = GetWorld()->CreateBody(&bodyDef);

            // Adicionando uma forma ao corpo (um retângulo)
            b2PolygonShape dynamicBox;
            b2Vec2 worldSize(tf->sizeMapToWorld(size.x), tf->sizeMapToWorld(size.y));
            dynamicBox.SetAsBox(worldSize.x, worldSize.y);

            // Configurando a densidade, fricção e restituição
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &dynamicBox;
            fixtureDef.density = 10.0f;
            fixtureDef.friction = 0.8f;
            fixtureDef.restitution = 0.0f;
            fixtureDef.filter.categoryBits = BodyTypes::Player | BodyTypes::Floor;
            dynamicBody->CreateFixture(&fixtureDef);
            mBody = dynamicBody;
            mBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);
        }
        else if(tiles[0] == "Sensor")
        {
            mBody = CreateBody(pos, size, false, BodyTypes::Floor, BodyTypes::Player, true, 0.0f, 1.0f);
            mBody->GetFixtureList()->SetSensor(true);
        }
        else if(tiles[0] == "Floor" || tiles[0] == "Box")
        {
            mBody = CreateBody(pos, size, false, BodyTypes::Floor, BodyTypes::Player, true, 0.0f, 1.0f);
        }
        else if(tiles[0] == "Player")
        {
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.fixedRotation = false;
            b2Vec2 worldPos = tf->posMapToWorld(pos, size);
            bodyDef.position = worldPos;
            b2Body* body = GetWorld()->CreateBody(&bodyDef);
            body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

            b2CircleShape shape;
            float radius = size.x/64.0f;
            shape.m_radius = radius;

            auto fixtureDef = new b2FixtureDef();
            fixtureDef->shape = &shape;
            fixtureDef->density = 1.2f;
            fixtureDef->friction = 0.0f;
            fixtureDef->restitution = 0.0f;
            fixtureDef->filter.categoryBits = BodyTypes::Player;
            fixtureDef->filter.maskBits = BodyTypes::Floor;
            body->CreateFixture(fixtureDef);
            mBody = body;
        }

    }
    Update();
}

class b2Body *WorldBodyComponent::CreateBody(const Vector2 &position, const Vector2 &size, bool isDynamic, BodyTypes type, BodyTypes collidesWith, bool fixedRotation, float density, float friction) {
    b2BodyDef bodyDef;
    if(isDynamic)
        bodyDef.type = b2_dynamicBody;
    bodyDef.fixedRotation = fixedRotation;
    b2Vec2 worldPos = tf->posMapToWorld(position, size);
    bodyDef.position = worldPos;
    b2Body* body = GetWorld()->CreateBody(&bodyDef);
    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    b2PolygonShape shape;
    b2Vec2 worldSize(tf->sizeMapToWorld(size.x), tf->sizeMapToWorld(size.y));
    shape.SetAsBox(worldSize.x, worldSize.y);

    auto fixtureDef = new b2FixtureDef();
    fixtureDef->shape = &shape;
    fixtureDef->density = density;
    fixtureDef->friction = friction;
    fixtureDef->restitution = 0.0f;
    fixtureDef->filter.categoryBits = type;
    fixtureDef->filter.maskBits = collidesWith;
    body->CreateFixture(fixtureDef);
    return body;
}

void WorldBodyComponent::Update(){
    if(GetType() == BodyTypes::Player)
        mBody->SetLinearVelocity(b2Vec2(0, mBody->GetLinearVelocity().y));
}

Vector2 WorldBodyComponent::GetVelocity() {
    return Vector2(mBody->GetLinearVelocity().x, mBody->GetLinearVelocity().y);;
}

Vector2 WorldBodyComponent::GetPosition() {
    if(GetClass() == "Ball" || GetClass() == "Player"){
        auto shape = dynamic_cast<b2CircleShape*>(mBody->GetFixtureList()->GetShape());
        float radius = shape->m_radius;
        return tf->posWorldToMap(mBody->GetPosition(), b2Vec2(2 * radius, 2 * radius));
    } else {
        auto shape = dynamic_cast<b2PolygonShape*>(mBody->GetFixtureList()->GetShape());
        b2Vec2 worldSize = shape->m_vertices[2] - shape->m_vertices[0];
        return tf->posWorldToMap(mBody->GetPosition(), worldSize);
    }
}

Vector2 WorldBodyComponent::GetSize() {
    if(GetClass() == "Ball" || GetClass() == "Player"){
        auto shape = dynamic_cast<b2CircleShape*>(mBody->GetFixtureList()->GetShape());
        float radius = shape->m_radius;
        return Vector2(radius*64, radius*64);
    } else {
        auto shape = dynamic_cast<b2PolygonShape*>(mBody->GetFixtureList()->GetShape());
        b2Vec2 worldSize = shape->m_vertices[2] - shape->m_vertices[0];
        return Vector2(worldSize.x*32, worldSize.y*32);
    }

}

float WorldBodyComponent::GetAngle() {
    return mBody->GetAngle();
}

void WorldBodyComponent::Jump(){
    mBody->SetLinearVelocity(b2Vec2(mBody->GetLinearVelocity().x, mJumpVelocity));
}

void WorldBodyComponent::Run(bool toTheRight){
    mBody->SetLinearVelocity(b2Vec2(mRunVelocity*((toTheRight)?1.0f:-1.0f), mBody->GetLinearVelocity().y));
}