#include "Item.h"

#include <cmath>

void Item::Update(
    double delta,
    float camL, float camR,
    float camB, float camT
)
{
    HitX = HitY = false;

    double dX = XDir * Speed * delta;
    double dY = YDir * Speed * delta;

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