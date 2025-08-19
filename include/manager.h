#ifndef MANAGER_HEADER
#define MANAGER_HEADER

#include "typeDefs.h"
#include <cppQueue.h>
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"
#include "sensors.h"
#include "menu.h"

extern unsigned long currentMillis;
extern long TimerColor[4];
extern struct menuOptions menuOptions;
extern struct userPrefs uPrefs;

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
    TimerManager(SegmentDisplay &seg, ProgressBar &prog, Screen &scr, cppQueue &q) : segmentDisplay(seg),
                                                                                     progressBar(prog),
                                                                                     screen(scr),
                                                                                     queue(q)

    {

        segmentDisplay.setManager(this);
        progressBar.setManager(this);
        screen.setManager(this);
        // queue.setManager(this);
    };

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