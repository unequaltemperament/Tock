#ifndef MANAGER_HEADER
#define MANAGER_HEADER

#include "typeDefs.h"
#include <cppQueue.h>
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"
#include "sensors.h"

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

    void update(){

        if (isRunning)
        {
          currentTimer->remainingTimeInMS = currentTimer->initialTimeInMS - (currentMillis - startedAt);
          segmentDisplay.update(currentMillis);
          if (getStatus() != EXPIRE)
          {
            progressBar.update();
          }
        }

    };

    bool isExpired()
    {
        return currentTimer->getElapsedPercentage() >= 1 &&
               queue.isEmpty();
    }

    double getElapsedPercentage()
    {
        return currentTimer->getElapsedPercentage();
    }

    TimerStatus getStatus()
    {
        return currentTimer->status;
    };

    long getRemainingTime()
    {
        return currentTimer->remainingTimeInMS;
    }
};

#endif