#pragma once
#include <chrono>


class Time
{
public:
    static float DeltaTime;

    static void Update()
    {
        auto now = Clock::now();

        DeltaTime =
            std::chrono::duration<float>(now - LastTime).count();

        LastTime = now;
    }

private:
    using Clock = std::chrono::steady_clock;

    static inline Clock::time_point LastTime = Clock::now();
};

