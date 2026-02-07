#include "Item.h"

#include <cmath>

void Item::Update(
    float delta,
    float camL, float camR,
    float camB, float camT
)
{
    HitX = HitY = false;

    float dX = XDir * Speed * delta;
    float dY = YDir * Speed * delta;

    X += dX;
    if (X - Width * 0.5f < camL || X + Width * 0.5f > camR)
    {
        XDir *= -1.0f;
        X += XDir * std::fabs(dX);
        HitX = true;
    }

    Y += dY;
    if (Y - Height * 0.5f < camB || Y + Height * 0.5f > camT)
    {
        YDir *= -1.0f;
        Y += YDir * std::fabs(dY);
        HitY = true;
    }
}