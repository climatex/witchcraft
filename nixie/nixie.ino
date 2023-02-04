// Off-line nixie clock with a Mega2560, (c) J. Bogin

#define AC_LINE_HZ    50 // 50 or 60 Hz
#define DCF77_SUPPORT 1  // comment out to disable DCF module support
#define INVALID_TIME  60 // an invalid value for hours, minutes, seconds

// DCF support
#ifdef DCF77_SUPPORT
#include "src\DCF77\DCF77.h"
DCF77* dcf = NULL;
#endif

// current time
volatile byte hours = 0;
volatile byte minutes = 0;
volatile byte seconds = 0;

// ac cycles in a second and total
volatile byte cycles = 0;
volatile unsigned long cycles_total = 0;

// flags:
volatile byte is_setting_time = 1; // 1: setting hours (default from start), 2: minutes, 3: seconds, 0: clock runs
volatile unsigned long button_pressed = 0; // total cycles at which either button was pressed, 0: state re-set

// reset by null pointer function call
void (*reset)() = NULL;

// line clock ISR
void ac_line_isr()
{  
  // update cycles, but advance time only if not in time setting mode
  cycles_total++;
  cycles++;
  
  if (cycles == AC_LINE_HZ)
  {
    cycles = 0;
    
    if (!is_setting_time)
    {
      seconds++;  
    }   
  }
  else
  {
    return;
  }
  
  if (is_setting_time)
  {
    return;
  }  
  
  if (seconds == 60)
  {
    minutes++;
    seconds = 0;
  }
  
  if (minutes == 60)
  {
    hours++;
    minutes = 0;
  }
  
  if (hours == 24)
  {
    //increment days, weeks, months, years, decades, centuries, millenia :)
    hours = 0;
  }
}

// wait arbitrary clock cycles
void wait_cycles(byte towait)
{
  const unsigned long cycle = cycles_total;
  while (cycles_total - cycle < towait) {}
}

// common ISR for both buttons - store cycle count upon press
void button_isr()
{
  button_pressed = cycles_total;
}

// button action: increment time during setting
void button_action_increment()
{ 
  // hours
  if (is_setting_time == 1)
  {
    hours++;
    if (hours == 24)
    {
      hours = 0;
    }
  }
  
  // minutes      
  else if (is_setting_time == 2)
  {
    minutes++;
    if (minutes == 60)
    {
      // do not automatically increment hours as they have been already confirmed
      minutes = 0;
    }
  }
  
  // seconds
  else if (is_setting_time == 3)
  {
    seconds++;
    if (seconds == 60)
    {
      seconds = 0;
    }
  }
}

// button action: confirm time during setting
void button_action_confirm()
{ 
  // confirm current selection and advance to minutes, seconds
  if (is_setting_time < 3)
  {
    is_setting_time++;
  }
  
  // start clock
  else
  {
    is_setting_time = 0;
  }
  
  cycles = 0;
  cycles_total = 0;
}

void off_nixie(byte pins, byte startpin)
{
  // turn off all digits in a nixie
  for (byte n = 0; n <= pins; n++)
  {
    digitalWrite(startpin+n, HIGH);
  }
}

void set_nixie(byte pins, byte startpin, byte cathode)
{
  // make sure all cathodes are off and turn only one on (negative logic for common-base)
  off_nixie(pins, startpin);
  
  // show last digit if cathode index out of bounds
  if (cathode > pins)
  {
    digitalWrite(startpin+pins, LOW);
  }
  else
  {
    digitalWrite(startpin+cathode, LOW);
  }
}

void update()
{
  // only update what needs to be updated
  // initial values are invalid, marked for update
  static byte previous_hour = INVALID_TIME;
  static byte previous_minute = INVALID_TIME;
  static byte previous_second = INVALID_TIME;
  
  // button states: time increment, time confirm/reset
  byte increment_button = (digitalRead(18) == LOW);
  byte confirm_button = (digitalRead(19) == LOW);
  
  // DCF
#ifdef DCF77_SUPPORT
  time_t t = dcf->getTime();
  
  // DCF time information available, update our counts
  if (t)
  {
    hours = hour(t);
    minutes = minute(t);
    seconds = second(t);
    
    // reset clock count, mark as clock running (if still in time setting mode, leave)
    cycles = 0;
    cycles_total = 0;
    is_setting_time = 0;
  }
#endif
    
  // is setting time? - default action right after start or reset
  if (is_setting_time)
  {   
    static unsigned long timeout_counter = 0;
    static byte increment_button_held = 0;

    // with DCF support in time setting mode, use the dot indicators for verifying DCF pulses
#ifdef DCF77_SUPPORT
    digitalWrite(22, digitalRead(03));
#endif
    
    // if pushbuttons actuated, perform debounce by waiting and checking again
    if (increment_button)
    {
      wait_cycles(2);
      increment_button = (digitalRead(18) == LOW);
    }
    if (confirm_button)
    {
      wait_cycles(2);
      confirm_button = (digitalRead(19) == LOW);
    }
    
    // handle increment button action
    if (increment_button && !confirm_button && button_pressed)
    {      
      // clear timeout
      timeout_counter = 0;
      
      // first keypress? commence keyrepeat after some half a second
      if (!increment_button_held)
      {
        button_action_increment();
        button_pressed = cycles_total + AC_LINE_HZ/2;
        increment_button_held = 1;
      }
      
      // button already held? do fast keyrepeat once possible
      else if (cycles_total >= button_pressed)
      {
        button_action_increment();
        button_pressed = cycles_total + AC_LINE_HZ/10;
      }
    }
    
    // handle confirm button action
    else if (confirm_button && !increment_button && button_pressed)
    {
      // clear timeout
      timeout_counter = 0;
      
      // no keyrepeat here
      button_action_confirm();
      button_pressed = 0;
      
      // clock just started? force update the dot indicators
      if (!is_setting_time)
      {
        previous_second = INVALID_TIME;
        return;
      }
    }
    
    // no current buttons or invalid state (both pressed etc)
    else
    {     
      // clear keyrepeat state      
      button_pressed = 0;
      increment_button_held = 0;
      
      // signal idle from now on
      if (!timeout_counter)
      {
        timeout_counter = cycles_total;
      }
    }
    
    // power down all cathodes if in time setting mode for over 10 minutes without doing anything
    if (!increment_button_held && (cycles_total - timeout_counter > AC_LINE_HZ*60*10))
    {
      // turn all bulbs off
      off_nixie(2, 23);
      off_nixie(9, 26);
      off_nixie(5, 36);
      off_nixie(9, 42);
      off_nixie(5, 52);
      off_nixie(9, 58);
      
      // mark for update on resume        
      previous_hour = INVALID_TIME;
      previous_minute = INVALID_TIME;
      previous_second = INVALID_TIME;
      
      return;
    }

    // blinking effect during time setting; not if incrementing time on keyrepeat
    if (!increment_button_held && (cycles < AC_LINE_HZ/6))
    {
      // hours - blink H1, H2, mark update hours
      if (is_setting_time == 1)
      {
        off_nixie(2, 23);
        off_nixie(9, 26);
        previous_hour = INVALID_TIME;
      }
      
      // minutes - blink M1, M2, mark update minutes
      else if (is_setting_time == 2)
      {
        off_nixie(5, 36);
        off_nixie(9, 42);
        previous_minute = INVALID_TIME;
      }
      
      // seconds - blink S1, S2, mark update seconds
      else
      {
        off_nixie(5, 52);
        off_nixie(9, 58);
        previous_second = INVALID_TIME;
      }
      
      return;
    }
  }
  
  // not setting time: detect if confirm button is pressed
  else if (confirm_button && !increment_button && button_pressed)
  {
    // and held for two seconds? reset the board
    if (cycles_total - button_pressed >= AC_LINE_HZ*2)
    {
      reset();
    }
  }
  
  // not setting time or resetting: clear button state
  else
  {
    button_pressed = 0;
  }

  // update hours, minutes or seconds, as required
  if (previous_hour != hours)
  {  
    const byte h1 = hours / 10;    
    if (h1 != (previous_hour / 10))
    {
      // update H1
      set_nixie(2, 23, h1);
    }
    
    // update H2
    set_nixie(9, 26, hours % 10);    
    previous_hour = hours;
  }
  
  if (previous_minute != minutes)
  { 
    const byte m1 = minutes / 10;    
    if (m1 != (previous_minute / 10))
    {
      // update M1
      set_nixie(5, 36, m1);
    }
    
    // update M2
    set_nixie(9, 42, minutes % 10);
    previous_minute = minutes;
  }
  
  if (previous_second != seconds)
  {
    const byte s1 = seconds / 10;
    if (s1 != (previous_second / 10))
    {
      // update S1
      set_nixie(5, 52, s1);
    }
    
    // update S2
    set_nixie(9, 58, seconds % 10);
    previous_second = seconds;
    
    // blink the dots: off every odd second
    if (!is_setting_time)
    {
      digitalWrite(22, (seconds % 2) ? HIGH : LOW); // negative logic for the glow lamps
      digitalWrite(13, (seconds % 2) ? LOW : HIGH); // do it also for the internal LED (positive logic)
    }
  }  
}

void setup()
{ 
  // onboard LED flashes together with HH-MM-SS separators once time is set
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  // inputs: D02 (AC clock), D03 (DCF77 decoder, optional)
  // buttons: D18 (increment time button), D19 (confirm/reset button) - with pullups
  pinMode(02, INPUT);
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  
  // DCF
#ifdef DCF77_SUPPORT
  dcf = new DCF77(03, digitalPinToInterrupt(03));
  dcf->Start();
#endif
   
  //D22 to D67 (D54-D67 as A0-A13) outputs with negative logic 
  for (byte pin = 22; pin <= 67; pin++)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
   
  // set interrupt handlers: AC line clock, increment and confirm button
  attachInterrupt(digitalPinToInterrupt(02), ac_line_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(18), button_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(19), button_isr, FALLING);
}

void loop()
{
  while(true)
  {
    update();
  }
}