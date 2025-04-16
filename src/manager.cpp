#include "manager.h"

bool TimerManager::loadNextTimer()
{
  if (!queue.pop(&currentTimer))
  {
    TockTimer *p = &currentTimer;
    p = nullptr;
    return false;
  };
  progressBar.lightIntervalInMs = currentTimer.initialTimeInMS / (progressBar._num_leds * progressBar.partialSteps);
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
    if (isExpired())
    {
      currentTimer.status = TimerStatus::EXPIRE;
      status = TimerStatus::EXPIRE;
      break;
    }
    currentTimer.remainingTimeInMS = currentTimer.initialTimeInMS - (currentMillis - startedAt);
    segmentDisplay.update();
    progressBar.update();

    break;
  case TimerStatus::EXPIRE:
    segmentDisplay.expireBlink();
    progressBar.expireBlink();
    break;
  }
};