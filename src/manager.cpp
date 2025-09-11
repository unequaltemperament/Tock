#include "manager.h"
#include <math.h>
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"
#include "sensors.h"
#include "menu.h"

extern unsigned long currentMillis;
extern long TimerColor[5];
extern struct menuOptions menuOptions;
extern struct userPrefs uPrefs;
const char ldrIntervalInMS = 250;

TimerManager::TimerManager(SegmentDisplay &seg, ProgressBar &prog, Screen &scr, cppQueue &q) : segmentDisplay(seg),
                                                                                               progressBar(prog),
                                                                                               screen(scr),
                                                                                               queue(q)

{

  segmentDisplay.setManager(this);
  progressBar.setManager(this);
  screen.setManager(this);
  // queue.setManager(this);
};

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

  //TODO: this probably looks like a strobe show
  //Adding some kind of smoothing and hysteresis would probably help
  static unsigned long previousLdrMillis = 0;

  if (uPrefs.brightness.autoSet && (currentMillis - previousLdrMillis >= ldrIntervalInMS))
  {
    previousLdrMillis = currentMillis;
    
    segmentDisplay.setBrightness(map(getAmbientBrightness(), 0, 1023, 0, CAPPED_7SEG_BRIGHTNESS));
    progressBar.setBrightness(map(getAmbientBrightness(), 0, 1023, 0, CAPPED_BAR_BRIGHTNESS));

    segmentDisplay.show();
    progressBar.show();


    screen.setBrightness(map(getAmbientBrightness(), 0, 1023, 0, CAPPED_BACKLIGHT_BRIGHTNESS));    
  }




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
        // setting both of these to true means that when a timer expires,
        // the first pass through expireBlink() will toggle it to false.
        // This means the first visible indicator of expiration is the display going dark.
        // Set these to false here if they should "snap" to the lit expired state
        segmentDisplay.expireLEDBlinkOn = true;
        progressBar.expireLEDBlinkOn = true;

        currentTimer.status = TimerStatus::EXPIRE;
        screen.setMode(currentTimer.status);
        break;
      }
    }

    segmentDisplay.update();
    progressBar.update();

    break;

  case TimerStatus::EXPIRE:
    if (loadNextTimer())
    {
      start();
    }
    else
    {
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
  do
  {
    palleteIndex = random(0, max);
  } while (palleteIndex == uPrefs.selectedPalette);

  setPallete(palleteIndex);
  debug("Palette set to ");
  debugln(menuOptions.palletes[palleteIndex].palleteName);

  segmentDisplay.reColor(TimerColor[getStatus()]);
  progressBar.forceUpdate();
  screen.dirty = true;
};