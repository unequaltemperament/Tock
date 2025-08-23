#ifndef MANAGER_HEADER
#define MANAGER_HEADER

#include "typeDefs.h"
#include <cppQueue.h>

extern unsigned long currentMillis;
extern long TimerColor[5];
extern struct menuOptions menuOptions;
extern struct userPrefs uPrefs;

constexpr int CAPPED_NEOPIXEL_BRIGHTNESS = 90;

class SegmentDisplay;
class ProgressBar;
class Screen;

class TimerManager
{
private:
    SegmentDisplay &segmentDisplay;
    ProgressBar &progressBar;
    Screen &screen;
    cppQueue &queue;
    TockTimer currentTimer;

    const unsigned long normalUpdateIntervalInMS = 1000;
    const unsigned long expireBlinkIntervalInMS = 420;

    bool isRunning = false;

public:
    TimerManager(SegmentDisplay &seg, ProgressBar &prog, Screen &scr, cppQueue &q);

    unsigned long startedAt = 0;

    bool loadNextTimer();
    void start();
    void stop()
    {
        isRunning = false;
    }
    void update();

    double inline getElapsedPercentageNormalized()
    {
        return currentTimer.getElapsedPercentageNormalized();
    }

    bool inline isElapsed()
    {
        return currentTimer.getElapsedPercentageNormalized() >= 1;
    }

    bool inline isExpired()
    {
        return isElapsed() && queue.isEmpty();
    }

    TimerStatus inline getStatus()
    {
        return currentTimer.status;
    };

    long inline getTimerColor()
    {
        return TimerColor[getStatus()];
    }

    long inline getRemainingTime()
    {
        return currentTimer.remainingTimeInMS;
    }

    TockTimer *const getCurrentTimer()
    {
        return &currentTimer;
    }

    bool inline isQueueEmpty()
    {
        return queue.isEmpty();
    }

    void updatePalette();
};

#endif