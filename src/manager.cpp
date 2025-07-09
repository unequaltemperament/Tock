#include "manager.h"

bool TimerManager::loadNextTimer()
{
  TockTimer *p = &currentTimer;
  if (!queue.pop(p))
  {
    p = nullptr;
    return false;
  };
  progressBar.clear();
  progressBar.lightIntervalInMs = currentTimer.initialTimeInMS / (progressBar._num_leds * progressBar.partialSteps);
  screen.dirty = true;
  return true;
}

void TimerManager::start()
{
  currentMillis = millis();
  isRunning = true;
  status = currentTimer.status;
  startedAt = currentMillis;
  segmentDisplay.updatedAt = currentMillis;
  progressBar.updatedAt = currentMillis;
  segmentDisplay.forceUpdate();
  progressBar.forceUpdate();
}

void TimerManager::update()
{

  switch (status)
  {
  case TimerStatus::STOPPED:
    break;

  case TimerStatus::WORK:
  case TimerStatus::BREAK:
    // fallthrough
    currentTimer.remainingTimeInMS = currentTimer.initialTimeInMS - (currentMillis - startedAt);

    if (isElapsed())
    {
      if (loadNextTimer())
      {
        start();
      }
      else
      {
        currentTimer.status = TimerStatus::EXPIRE;
        status = TimerStatus::EXPIRE;
        screen.setMode(TimerStatus::EXPIRE);
        break;
      }
    }

    segmentDisplay.update();
    progressBar.update();

    break;

  case TimerStatus::EXPIRE:
    
    segmentDisplay.expireBlink();
    progressBar.expireBlink();
    break;
  }
};