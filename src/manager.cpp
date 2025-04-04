#include "manager.h"

void TimerManager::update(){
    if (isRunning)
    {
      currentTimer.remainingTimeInMS = currentTimer.initialTimeInMS - (currentMillis - startedAt);
      segmentDisplay.update();
      if (getStatus() != TimerStatus::EXPIRE)
      {
        progressBar.update();
      }
    }

};