#include "Item.h"
#include <cmath>
#include <algorithm>


static void Normalize(float& x, float& y)
{
    float len = std::sqrt(x * x + y * y);
    if (len == 0.0f) {
        x = 1.0f;
        y = 0.0f;
        return;
    }
    x /= len;
    y /= len;
}

void Item::Update(
    double delta,
    float camL, float camR,
    float camB, float camT
)
{
    HitX = HitY = false;

    float halfW = Width * 0.5f;
    float halfH = Height * 0.5f;

    // ---- Corner Assist Cooldown ----
    
    if (cornerCooldown > 0.0)
        cornerCooldown -= delta;

    // ---- Corner Assist ----
    if (cornerCooldown <= 0.0)
    {
        float distX = (XDir > 0) ? (camR - halfW - X) : (X - (camL + halfW));
        float distY = (YDir > 0) ? (camT - halfH - Y) : (Y - (camB + halfH));

        float timeX = distX / (Speed * std::fabs(XDir));
        float timeY = distY / (Speed * std::fabs(YDir));

        float norm = std::fabs(timeX - timeY) / std::max(timeX, timeY);
        const float cornerTolerance = 0.03f; // smaller and stable

        if (norm < cornerTolerance)
        {
            float targetX = (XDir > 0) ? camR - halfW : camL + halfW;
            float targetY = (YDir > 0) ? camT - halfH : camB + halfH;

            float vx = targetX - X;
            float vy = targetY - Y;

            Normalize(vx, vy);
            XDir = vx;
            YDir = vy;

            cornerCooldown = 0.250;
        }
    }

    // ---- Predict collisions ----
    float nextX = X + XDir * Speed * delta;
    float nextY = Y + YDir * Speed * delta;

    bool collideX = (nextX - halfW < camL) || (nextX + halfW > camR);
    bool collideY = (nextY - halfH < camB) || (nextY + halfH > camT);

    if (collideX) {
        XDir *= -1.0f;
        HitX = true;
    }
    if (collideY) {
        YDir *= -1.0f;
        HitY = true;
    }

    // ---- Move after collision adjustment ----
    X += XDir * Speed * delta;
    Y += YDir * Speed * delta;

    // ---- Clamp strictly to prevent "stuck" corners ----
    if (X - halfW < camL) X = camL + halfW;
    if (X + halfW > camR) X = camR - halfW;
    if (Y - halfH < camB) Y = camB + halfH;
    if (Y + halfH > camT) Y = camT - halfH;

    // ---- Normalize direction to keep constant speed ----
    //Normalize(XDir, YDir);

}
