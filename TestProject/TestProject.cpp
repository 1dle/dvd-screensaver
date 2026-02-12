#include "pch.h"
#include "CppUnitTest.h"
#include "../dvd-screensaver/Item.h"
#include "../dvd-screensaver/Renderer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestProject
{
    TEST_CLASS(ItemTests)
    {
    public:
        struct Resolution
        {
            int w, h;
        };



        struct Stats
        {
            int wallHits = 0;
            int cornerHits = 0;
            int invalidCornerHits = 0;
        };

        Stats RunSimulation(double seconds)
        {
            float virtW = 2560;
            float virtH = 1440;
            float Aspect = virtW / virtH;

            float camB = -WORLD_HEIGHT * 0.5f;
            float camT = WORLD_HEIGHT * 0.5f;
            float camL = camB * Aspect;
            float camR = camT * Aspect;

            double delta = TARGET_FRAME_TIME;

            Item item;

            Stats stats;

            double time = 0.0;
            bool prev_coll_corner = false;

            while (time < seconds)
            {
                item.Update(delta, camL, camR, camB, camT);

                bool collision = (item.HitX || item.HitY);
                bool corner = (item.HitX && item.HitY);

                if (collision)
                {
                    stats.wallHits++;

                    if (corner)
                    {
                        if (prev_coll_corner)
                            stats.invalidCornerHits++;

                        stats.cornerHits++;
                        prev_coll_corner = true;
                    }
                    else
                    {
                        prev_coll_corner = false;
                    }
                }

                time += delta;
            }

            return stats;
        }

        TEST_METHOD(HitCorner_ReflectsBoth)
        {
            Item item;
            item.X = 9.0f;
            item.Y = 9.0f;
            item.XDir = 1.0f;
            item.YDir = 1.0f;

            float camL = -10.0f, camR = 10.0f, camB = -10.0f, camT = 10.0f;
            item.Update(TARGET_FRAME_TIME, camL, camR, camB, camT);

            Assert::IsTrue(item.HitX && item.HitY);
        }


        TEST_METHOD(MultiMonitor_Stability)
        {
            auto res = Resolution{ 3840, 1080 };
            float Aspect = float(res.w) / float(res.h);

            float camB = -WORLD_HEIGHT * 0.5f;
            float camT = WORLD_HEIGHT * 0.5f;
            float camL = camB * Aspect;
            float camR = camT * Aspect;

            Item item;
            double delta = TARGET_FRAME_TIME;

            int hits = 0;
            for (int i = 0; i < 500000; ++i)
            {
                item.Update(delta, camL, camR, camB, camT);
                if (item.HitX || item.HitY) hits++;
            }

            Assert::IsTrue(hits > 0);
        }



        TEST_METHOD(Item_movement)
        {
            Stats stats = RunSimulation(3600.0); // 1 hour

            wchar_t msg[128];
            swprintf_s(msg, L"wall hit %d times\n", stats.wallHits);
            Logger::WriteMessage(msg);

            swprintf_s(msg, L"corner hit %d times\n", stats.cornerHits);
            Logger::WriteMessage(msg);

            swprintf_s(msg, L"invalid corner hits: %d times\n", stats.invalidCornerHits);
            Logger::WriteMessage(msg);

            Assert::AreEqual(0, stats.invalidCornerHits,
                L"Corner hit must not occur on the next collision after any other corner hit.");
        }
    };
}
