#include "manager.h"
#include <math.h>

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
  return true;
}

void TimerManager::start()
{
  currentMillis = millis();
  isRunning = true;
  startedAt = currentMillis;
  segmentDisplay.updatedAt = currentMillis;
  progressBar.updatedAt = currentMillis;
  screen.setMode(currentTimer.status);
  screen.dirty = true;
  segmentDisplay.forceUpdate();
  progressBar.forceUpdate();

}

void TimerManager::update()
{

  switch (currentTimer.status)
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
        screen.setMode(TimerStatus::EXPIRE);
        break;
      }
    }

    segmentDisplay.update();
    progressBar.update();

    break;

  case TimerStatus::EXPIRE:
    if(loadNextTimer()){
      start();}
    else{
    segmentDisplay.expireBlink();
    progressBar.expireBlink();
    }
    break;
  }
};

void TimerManager::updatePalette()
{
  int palleteIndex = -1;

  randomSeed(millis());
  int max = sizeof(menuOptions.palletes) / sizeof(menuOptions.palletes[0]);
  do{
  palleteIndex = random(0, max);
  }
  while(palleteIndex == uPrefs.selectedPalette);

  setPallete(palleteIndex);
  debug("Palette set to ");
  debugln(menuOptions.palletes[palleteIndex].palleteName);

  segmentDisplay.reColor(TimerColor[getStatus()]);
  progressBar.forceUpdate();
  screen.dirty = true;
};