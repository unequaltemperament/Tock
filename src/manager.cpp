#include "manager.h"
#include <math.h>
#include "progressBar.h"
#include "segmentDisplay.h"
#include "screen.h"
#include "sensors.h"
#include "menu.h"

extern unsigned long currentMillis;
extern unsigned long TimerColor[5];
extern struct menuOptions menuOptions;
extern struct userPrefs uPrefs;


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
  
  //might start new timer with menu open
  if(screen.getMode() != Screen::Mode::MENU){
    screen.setMode(Screen::Mode::QUEUE);
    screen.dirty = true;
  }

  segmentDisplay.forceUpdate();
  progressBar.forceUpdate();
}

void TimerManager::update()
{

  if(uPrefs.brightness.autoSet){
    autoUpdateBrightness();
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
        currentTimer.status = TimerStatus::EXPIRE;
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
      if (screen.getMode() != Screen::Mode::EXPIRED){
      screen.setMode(Screen::Mode::EXPIRED);
      // setting both of these to true means that when a timer expires,
      // the first pass through expireBlink() will toggle it to false.
      // This means the first visible indicator of expiration is the display going dark.
      // Set these to false here if they should "snap" to the lit expired state
      segmentDisplay.expireLEDBlinkOn = true;
      progressBar.expireLEDBlinkOn = true;
      }


      segmentDisplay.expireBlink();
      progressBar.expireBlink();
    }
    break;
  }
};

bool TimerManager::queueTimer(TimerStatus status, long initialTimeInSeconds)
{
  bool result;
  TockTimer timer(status, initialTimeInSeconds);
  result = queue.push(&timer);
  return result;
}

int TimerManager::iterateNextInQueue(TockTimer *buffer)
{
  static int idx = 0;

  // result is 0 when idx goes off the end of the queue
  int result = queue.peekIdx(buffer, idx);
  idx++;
  idx *= result;
  return result;
}

void TimerManager::switchToRandomPalette()
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

void TimerManager::autoUpdateBrightness(){
//TODO: this probably looks like a strobe show
  //Adding some kind of smoothing and hysteresis would probably help
  static unsigned long previousLdrMillis = 0;
  static unsigned long previousAutoUpdateMillis = 0;
  static bool locked = false;

  if (currentMillis - previousLdrMillis >= ldrIntervalInMS && !locked)
  {
    previousLdrMillis = currentMillis;
    targetBrightness = getAmbientBrightness() / 1023.0f;
        
  }
  if (currentMillis - previousAutoUpdateMillis >= 50){
    previousAutoUpdateMillis = currentMillis;

    if(fabs(masterBrightness - targetBrightness) > 0.01){
      debugln("adjusting");
      locked = true;
      masterBrightness = .15 * targetBrightness + (.85) * masterBrightness;
    }
    else{
      debugln("stable");
      masterBrightness = targetBrightness;
      locked = false;
    }

    //TODO: don't run this every single update
    //TODO: also plagued by usual low-level brightness issues in the LEDS
    screen.setBrightness(static_cast<int>(masterBrightness * CAPPED_BACKLIGHT_BRIGHTNESS));
    segmentDisplay.setBrightness(static_cast<uint8_t>(masterBrightness * CAPPED_7SEG_BRIGHTNESS));
    progressBar.setBrightness(static_cast<uint8_t>(masterBrightness * CAPPED_BAR_BRIGHTNESS));

    segmentDisplay.show();
    progressBar.show();
  }
};