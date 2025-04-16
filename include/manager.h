#ifndef MANAGER_HEADER
#define MANAGER_HEADER

#include "typeDefs.h"
#include <cppQueue.h>
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"
#include "sensors.h"

extern unsigned long currentMillis;

class TimerManager
{
private:
    SegmentDisplay &segmentDisplay;
    ProgressBar &progressBar;
    Screen &screen;
    cppQueue &queue;
    TockTimer currentTimer;
    TimerStatus status = TimerStatus::STOPPED;

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

    bool inline isExpired()
    {
        return currentTimer.getElapsedPercentageNormalized() >= 1;
    }

    double inline getElapsedPercentageNormalized()
    {
        return currentTimer.getElapsedPercentageNormalized();
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

    bool inline isQueueEmpty(){
        return queue.isEmpty();
    }
};

#endif