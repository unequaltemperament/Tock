#include "manager.h"

void TimerManager::update(){

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