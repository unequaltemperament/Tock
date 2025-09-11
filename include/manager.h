#ifndef MANAGER_HEADER
#define MANAGER_HEADER

#include "typeDefs.h"
#include <cppQueue.h>

constexpr int CAPPED_7SEG_BRIGHTNESS = 90;
constexpr int CAPPED_BAR_BRIGHTNESS = 20 * .25;
constexpr int CAPPED_BACKLIGHT_BRIGHTNESS = 127;

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