/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include <game/2.game.h>
#include <utility/sprite_renderer.h>
#include <utility/resource_manager.h>
#include <game/5.1.ball_object_collisions.h>

// Game-related State data
SpriteRenderer *Renderer;
GameObject *Player;
BallObject *Ball;

// Initial size of the player paddle
const Vector2 PLAYER_SIZE(100.0f, 20.0f);
// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);
// Initial velocity of the Ball
const Vector2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;

bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
float clamp(float value, float min, float max);
Direction VectorDirection(Vector2 target);

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    delete Player;
    delete Renderer;
}

void Game::Init()
{
    // load shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    // configure shaders
    Matrix4 proj = proj.orthographic(0.0f, static_cast<float>(Width), 0.0f, static_cast<float>(Height), 1.0f, -1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", proj);
    // set render-specific controls
    Shader spriteShader = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(spriteShader);
    // load textures
    ResourceManager::LoadTexture("textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
    // load levels
    GameLevel one;
    one.Load("levels/one.lvl", Width, Height / 2);
    GameLevel two;
    two.Load("levels/two.lvl", Width, Height / 2);
    GameLevel three;
    three.Load("levels/three.lvl", Width, Height / 2);
    GameLevel four;
    four.Load("levels/four.lvl", Width, Height / 2);
    Levels.push_back(one);
    Levels.push_back(two);
    Levels.push_back(three);
    Levels.push_back(four);
    Level = 0;
    // Player init
    Vector2 playerPos = Vector2(Width / 2.0f - PLAYER_SIZE.x / 2.0f, Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    //Ball init
    Vector2 ballPos = playerPos + Vector2(PLAYER_SIZE.x / 2.0f -
    BALL_RADIUS, -BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::ResetLevel()
{
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2);
    else if (this->Level == 3)
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = Vector2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + Vector2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

void Game::Update(float dt)
{
    Ball->Move(dt, Width);
    DoCollisions();
    if (Ball->Position.y >= Height) // did ball reach bottom edge?
    {
        ResetLevel();
        ResetPlayer();
    }
}

void Game::ProcessInput(float dt)
{
    float velocity = PLAYER_VELOCITY * dt;
    // move playerboard
    if (Keys[GLFW_KEY_A])
    {
        if (Player->Position.x >= 0.0f)
        {
            Player->Position.x -= velocity;
         if (Ball->Stuck)
             Ball->Position.x -= velocity;
        }
    }
    if (Keys[GLFW_KEY_D])
    {
        if (Player->Position.x <= Width - Player->Size.x)
        {
            Player->Position.x += velocity;
            if (Ball->Stuck)
                Ball->Position.x += velocity;
        }
    }
    if (Keys[GLFW_KEY_SPACE])
        Ball->Stuck = false;
}

void Game::Render()
{
    if (State == GAME_ACTIVE)
    {
        // draw background
        Texture2D background = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(background, Vector2(0.0f, 0.0f), Vector2(Width, Height), 0.0f);
        // draw level
        Levels[Level].Draw(*Renderer);
        // draw the player
        Player->Draw(*Renderer);
        // draw ball
        Ball->Draw(*Renderer);
    }
}

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.IsSolid)
                {
                    box.Destroyed = true;
                }
                // collision resolution
                Direction dir = std::get<1>(collision);
                Vector2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT) // horizontal collision
                {
                    Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                    // relocate
                    float penetration = Ball->Radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                    {
                        Ball->Position.x += penetration; // move ball to right
                    }
                    else
                    {
                        Ball->Position.x -= penetration; // move ball to left;
                    }
                }
                else // vertical collision
                {
                    Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                    // relocate
                    float penetration = Ball->Radius - std::abs(diff_vector.y);
                    if (dir == UP)
                    {
                        Ball->Position.y -= penetration; // move ball bback up
                    }
                    else
                    {
                        Ball->Position.y += penetration; // move ball back down
                    }
                }               
            }
        }    
    }

    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        Vector2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = Ball->Velocity.normalize() * oldVelocity.length();
    }
}

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
    two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
    two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}
Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle
{
    // get center point circle first
    Vector2 center = Vector2(one.Position.x  + one.Radius, one.Position.y + one.Radius);
    // calculate AABB info (center, half-extents)
    Vector2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    Vector2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    Vector2 difference = center - aabb_center;
    Vector2 clamped = Vector2(clamp(difference.x, -aabb_half_extents.x, aabb_half_extents.x), 
    clamp(difference.y, -aabb_half_extents.y, aabb_half_extents.y));
    // add clamped value to AABB_center and get the value closest to circle
    Vector2 closest = aabb_center + clamped;
    // vector between center circle and closest point AABB
    difference = closest - center;
    if (difference.length() <= one.Radius)
    {
        return std::make_tuple(true, VectorDirection(difference), difference);
    }
    else
    {
        return std::make_tuple(false, UP, Vector2(0.0f, 0.0f));
    }
}
float clamp(float value, float min, float max) 
{
    return std::max(min, std::min(max, value));
}
Direction VectorDirection(Vector2 target)
{
    Vector2 compass[] = 
    {
        Vector2(0.0f, 1.0f), // up
        Vector2(1.0f, 0.0f), // right
        Vector2(0.0f, -1.0f), // down
        Vector2(-1.0f, 0.0f) // left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        target.normalize();
        float dot_product = target.dot(compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}


