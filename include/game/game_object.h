/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <math/Vectors.h>

#include <utility/texture.h>
#include <utility/sprite_renderer.h>


// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.
class GameObject
{
public:
    // object state
    Vector2   Position, Size, Velocity;
    Vector3   Color;
    float       Rotation;
    bool        IsSolid;
    bool        Destroyed;
    // render state
    Texture2D   Sprite;	
    // constructor(s)
    GameObject();
    GameObject(Vector2 pos, Vector2 size, Texture2D sprite, Vector3 color = Vector3(1.0f, 1.0f, 1.0f), Vector2 velocity = Vector2(0.0f, 0.0f));
    // draw sprite
    virtual void Draw(SpriteRenderer &renderer);
};

#endif