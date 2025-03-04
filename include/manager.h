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
    TockTimer *currentTimer;

    const unsigned long normalUpdateIntervalInMS = 1000;
    const unsigned long expireBlinkIntervalInMS = 420;

    bool isRunning = false;
    




public:
    TimerManager(SegmentDisplay &seg, ProgressBar &prog, Screen &scr, cppQueue &q) : segmentDisplay(seg),
                                                                                     progressBar(prog),
                                                                                     screen(scr),
                                                                                     queue(q),
                                                                                     currentTimer(nullptr)
    {

        segmentDisplay.setManager(this);
        progressBar.setManager(this);
        screen.setManager(this);
        // queue.setManager(this);
    };

    unsigned long startedAt = 0;

    void update();

    bool inline isExpired()
    {
        return currentTimer->getElapsedPercentage() >= 1 &&
               queue.isEmpty();
    }

    double inline getElapsedPercentage()
    {
        return currentTimer->getElapsedPercentage();
    }

    TimerStatus inline getStatus()
    {
        return currentTimer->status;
    };

    long inline getTimerColor(){
        return TimerColor[getStatus()];
    }

    long inline getRemainingTime()
    {
        return currentTimer->remainingTimeInMS;
    }

    bool inline loadNextTimer(){
        queue.pop(currentTimer);
    }

    void start(){
        isRunning = true;
        startedAt = currentMillis;

        segmentDisplay.updatedAt = currentMillis;
        progressBar.updatedAt = currentMillis;
        segmentDisplay.forceUpdate();
        progressBar.forceUpdate();
    }

    void stop(){
        isRunning = false;
    }

    TockTimer* const getCurrentTimer(){
        return currentTimer;
    }
};

#endif