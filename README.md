# tock

Utility-driven time management clock designed to help deal with time blindness.

Featuring simple inputs, queueable timers, and an aesthetic interface, Tock makes it easy to track your work time and stay focused by fostering a sense of urgency across large periods of time. The basic idea is that, for people who deal with time blindness as a regular obstacle, managing and using one hour of time is abstract and very open-ended, but managing 600 seconds is more tangible. Tock takes your long periods of time, interprets them as much shorter ones, and shows you the time passing much more often.

This repo is the driver code for the larger project, which includes the physical clock asssembly and custom PCB to display countdown, progress bar, and HUD screen for information like upcoming blocks, total time worked, and length of current timer. 

Basic usage converts hard-coded button inputs to seconds-displayed countdown timers, for common timespans from 5 minutes to 4 hours, as well as "add time" options. Displayed time is color-coded according to the type of work (ie, working blocks are yellow, breaks are red, timer expiration is flashing red) to offer extra sensory cues. Progress through the timer is shown with a custom LED progress glow as well as on the HUD screen. Queued timers segue immediately upon the expiration of the current block, allowing breaks to be built seamlessly into your workday. Also supports common time-management schemes such as Pomodoro Technique and 52/17.

Wishlist items include app integration over bluetooth or wifi to allow for more customizable features such as custom queue templates, customizable colors and types, mood adjustment (cooler colors for morning, warmer ones for afternoon/evening, auto-brightness based on ambient room light), battery power.
