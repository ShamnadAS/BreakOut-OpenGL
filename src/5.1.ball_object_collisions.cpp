/******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <game/5.1.ball_object_collisions.h>


BallObject::BallObject() 
    : GameObject(), Radius(12.5f), Stuck(true) { }

BallObject::BallObject(Vector2 pos, float radius, Vector2 velocity, Texture2D sprite)
    : GameObject(pos, Vector2(radius * 2.0f, radius * 2.0f), sprite, Vector3(1.0f, 1.0f, 1.0f), velocity), Radius(radius), Stuck(true) { }

Vector2 BallObject::Move(float dt, unsigned int window_width)
{
    // if not stuck to player board
    if (!this->Stuck)
    {
        // move the ball
        this->Position += this->Velocity * dt;
        // then check if outside window bounds and if so, reverse velocity and restore at correct position
        if (this->Position.x <= 0.0f)
        {
            this->Velocity.x = -this->Velocity.x;
            this->Position.x = 0.0f;
        }
        else if (this->Position.x + this->Size.x >= window_width)
        {
            this->Velocity.x = -this->Velocity.x;
            this->Position.x = window_width - this->Size.x;
        }
        if (this->Position.y <= 0.0f)
        {
            this->Velocity.y = -this->Velocity.y;
            this->Position.y = 0.0f;
        }
    }
    return this->Position;
}

// resets the ball to initial Stuck Position (if ball is outside window bounds)
void BallObject::Reset(Vector2 position, Vector2 velocity)
{
    this->Position = position;
    this->Velocity = velocity;
    this->Stuck = true;
}