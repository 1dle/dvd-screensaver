#pragma once
class Item
{
public:
    float X = 2.0f;
    float Y = 0.0f;

    float Width = 5.0f;
    float Height = 5.0f;

    float Speed = 2.5f;
    float XDir = 1.0f;
    float YDir = 1.0f;

    bool HitX = false;
    bool HitY = false;

    void Update(
        float deltaSeconds,
        float camL, float camR,
        float camB, float camT
    );
};