#ifndef MANAGER_HEADER
#define MANAGER_HEADER

#include "typeDefs.h"
#include "cppQueue.h"

class SegmentDisplay;
class ProgressBar;
class Screen;
class cppQueue;


class TimerManager
{
private:
    SegmentDisplay &segmentDisplay;
    ProgressBar &progressBar;
    Screen &screen;
    cppQueue &queue;
    TockTimer* currentTimer;

public:
    TimerManager(SegmentDisplay &seg, ProgressBar &prog, Screen &scr, cppQueue &q) : segmentDisplay(seg),
                                                                                     progressBar(prog),
                                                                                     screen(scr),
                                                                                     queue(q),
                                                                                     currentTimer(nullptr)
    {

        // segmentDisplay.setManager(this);
        // progressBar.setManager(this);
        // screen.setManager(this);
        // queue.setManager(this);
    };

    bool isExpired()
    {
        return currentTimer->getElapsedPercentage() >= 1 &&
               queue.isEmpty();
    }

  TimerStatus getStatus(){
    return currentTimer->status;
  };
    
};

#endif