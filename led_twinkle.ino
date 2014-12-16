/***********************************************************************
LED twinkle with LilyTiny

By Jenny Lindberg

Randomized LED fade rates and sleep (off) times.

For use on awesome, blinged-out dog collar.

***********************************************************************/

#define NUM_LEDS    4

#define TICK_PERIOD 200 // loop speed, in microseconds
                        // can't go much faster

#define PWM_PERIOD  50  // ticks in a PWM period
                        // PWM period in microseconds = MAX_TICKS * TICK_PERIOD
                        
#define MAX_DUTY    25  // max led brightness, brightness = MAX_DUTY / PWM_PERIOD
                        // 50% brightness not noticeably dimmer than 100%
                        // save some battery!

// dwell is the number of PWM periods to spend at a given brightness level
// bounds for randomizing dwell
#define MIN_ACTIVE_DWELL 1
#define MAX_ACTIVE_DWELL 6
#define MIN_SLEEP_DWELL  10
#define MAX_SLEEP_DWELL  50

long startTime = 0;    // counter for loop iterations
long ticks = 0;        // counter for ticks

// state variables for each LED
byte duty[NUM_LEDS];         // current duty factor
byte dwell_count[NUM_LEDS];  // count of dwell periods at current duty
byte dwell_target[NUM_LEDS]; // number of dwell periods before we change duty
char increment[NUM_LEDS];    // current duty increment to apply at next dwell target (-1, 1, 0)

void setup()
{ 
  randomSeed(analogRead(3));
  for (byte iLed = 0; iLed < NUM_LEDS; iLed++) {
    pinMode(iLed, OUTPUT);
    duty[iLed] = 0;
    increment[iLed] = 1; // start by fading on
    dwell_count[iLed] = 0;
    dwell_target[iLed] = random(MIN_ACTIVE_DWELL, MAX_ACTIVE_DWELL);
  }
  startTime = micros();
} 

void loop()
{
  long currTime = micros();
  
  // speed limit
  if ((currTime - startTime) > TICK_PERIOD)
  {
    startTime = currTime;   // save last time we entered the loop
    
    // turn LEDs on/off as needed
    for (byte iLed = 0; iLed < NUM_LEDS; iLed++) {
      // turn on LED at start of PWM period, if it's not supposed to sleep
      if ((0 == ticks) && (duty[iLed] > 0)) {
        digitalWrite(iLed, HIGH);
      }
      // turn off LED when we reach duty factor
      if (ticks == duty[iLed]) {
        digitalWrite(iLed, LOW);
      }
    }
   
    // another one bites the dust
    ticks++;
    
    if (ticks > PWM_PERIOD) {
      ticks = 0; // start a new PWM period
      
      // update state of each LED as needed
      for (byte iLed = 0; iLed < NUM_LEDS; iLed++) {
        dwell_count[iLed]++;
        
        // have we dwelt long enough?
        if (dwell_count[iLed] > dwell_target[iLed]) {
          dwell_count[iLed] = 0; // start a new count
          duty[iLed] += increment[iLed]; // adjust brightness
          
          // if we've hit max brightness, reverse direction and start dimming
          if (duty[iLed] > MAX_DUTY) {
            increment[iLed] = -1;
          }
          
          // have we turned off?
          if (duty[iLed] == 0) {
            // have we been off for a while?
            if (increment[iLed] == 0) {
              increment[iLed] = 1; // turn on!
              dwell_target[iLed] = random(MIN_ACTIVE_DWELL, MAX_ACTIVE_DWELL); // randomly pick fade rate
            // no, we just turned off
            } else {
              increment[iLed] = 0; // stay off for a bit
              dwell_target[iLed] = random(MIN_SLEEP_DWELL, MAX_SLEEP_DWELL); // randomly pick an off duration
            }
          }
        }
      }
    } 
  }
}
