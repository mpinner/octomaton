/*
  AUTHOR: MATT PINNER
 
 MOSTLY STOLEN FROM:
 Nathan Seidle, SparkFun Electronics 2011
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Controlling an LED strip with individually controllable RGB LEDs. This stuff is awesome.
 
 The SparkFun (individually controllable) RGB strip contains a bunch of WS2801 ICs. These
 are controlled over a simple data and clock setup. The WS2801 is really cool! Each IC has its
 own internal clock so that it can do all the PWM for that specific LED for you. Each IC
 requires 24 bits of 'greyscale' data. This means you can have 256 levels of red, 256 of blue,
 and 256 levels of green for each RGB LED. REALLY granular.
 
 To control the strip, you clock in data continually. Each IC automatically passes the data onto
 the next IC. Once you pause for more than 500us, each IC 'posts' or begins to output the color data
 you just clocked in. So, clock in (24bits * 32LEDs = ) 768 bits, then pause for 500us. Then
 repeat if you wish to display something new.
 
 This example code will display bright red, green, and blue, then 'trickle' random colors down 
 the LED strip.
 
 You will need to connect 5V/Gnd from the Arduino (USB power seems to be sufficient).
 
 For the data pins, please pay attention to the arrow printed on the strip. You will need to connect to
 the end that is the begining of the arrows (data connection)--->
 
 If you have a 4-pin connection:
 Blue = 5V
 Red = SDI
 Green = CKI
 Black = GND
 
 If you have a split 5-pin connection:
 2-pin Red+Black = 5V/GND
 Green = CKI
 Red = SDI
 */

#define DEBUG_RULE    0
#define DEBUG_SERIAL  0
#define ECHO_RULE     1

//
// config for hardware
//
int SDI = 2; //Red wire (not the red 5V wire!)
int CKI = 3; //Green wire
int ledPin = 13; //On board LED

#define STRIP_LENGTH 32 //32 LEDs on this strip
long strip_colors[STRIP_LENGTH];
long prev_strip_colors[STRIP_LENGTH];

//
// COLOR AND SEQUENCE CONFIGS
//
long OFF = 0x000000;
// how many interations per rule
int seqMaxSteps = 100;
// how long between interations
int stepDurationDelay = 130;


#define ALLON_MSK    0x80
#define LEFT2_MSK    0x40
#define OUTSIDE_MSK  0x20
#define LEFT_MSK     0x10
#define RIGHT2_MSK   0x08
#define MIDDLE_MSK   0x04
#define RIGHT_MSK    0x02
#define ALLOFF_MSK   0x01

// 
// STATE
//
int seqStep = 0;
//byte rule = 0x01;
byte rule = (LEFT2_MSK | OUTSIDE_MSK | LEFT_MSK | RIGHT2_MSK | RIGHT_MSK | ALLOFF_MSK);
byte nextRule = rule;

/// RULE DEFAULTS
// this is an ugly way to encode this stuff... but it makes some code more readable
boolean allOn;
boolean left2;
boolean outside;
boolean left;
boolean right2;
boolean middle;
boolean right;
boolean allOff;


void setup() {

  Serial.begin(57600);
  //Serial.println("Hello!");

  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  pinMode(ledPin, OUTPUT);

  setupRule();

  //Clear out the array
  for(int x = 0 ; x < STRIP_LENGTH ; x++) {
    strip_colors[x] = OFF;
  }

  randomSeed(analogRead(0));
}

void setupRule() {

  // overly verbose but very readable
  allOn   = (ALLON_MSK & rule);
  left2   = (LEFT2_MSK & rule);
  outside = (OUTSIDE_MSK & rule);
  left    = (LEFT_MSK & rule);
  right2  = (RIGHT2_MSK & rule);
  middle  = (MIDDLE_MSK & rule);
  right   = (RIGHT_MSK & rule);
  allOff  = (ALLOFF_MSK & rule);

#if DEBUG_RULE
  Serial.print("r:  ");
  Serial.println(rule,HEX); 
#else
#if ECHO_RULE
  //Serial.println(rule,HEX); 
  Serial.print((char)rule); 
#endif
#endif
}

void serialEvent(){
  char c;

  if (Serial.available() > 0) {
    // get incoming byte:
    c = Serial.read();
    if (c != '\n') {
      nextRule = c;
#if DEBUG_SERIAL
      Serial.print("s:  ");
      Serial.println(nextRule,HEX); 
#endif
    }

    // only taking first byte. flush remaining serial buffer.
    Serial.flush();
  }
}


void loop() {

  if ( seqStep > seqMaxSteps ) {
    incrementRule();
  }  

  if (rule != nextRule) { 
    rule = nextRule;
    setupRule();
    startSeq();
  }

  calcAutomaton();
  post_frame(); //Push the current color frame to the strip

  delay(stepDurationDelay);                  // wait for a little while

  seqStep++;

  return;
}


void startSeq() {
  seqStep = 0;
  //Pre-fill the color array with known values
  //Bright Red
  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    strip_colors[LED_number] = OFF; //Bright Red
  }

  strip_colors[12] = getRandomColor ();

  post_frame(); //Push the current color frame to the strip

  delay(stepDurationDelay);

  return;
}

//Throws random colors down the strip array
void addRandom(void) {
  int x;

  int swapOutLed = random(STRIP_LENGTH);

  //First, shuffle all the current colors down one spot on the strip
  // for(x = (STRIP_LENGTH - 1) ; x > 0 ; x--)
  //   strip_colors[x] = strip_colors[x - 1];

  //Now form a new RGB color
  long new_color = getRandomColor ();
  strip_colors[swapOutLed] = new_color; //Add the new random color to the strip
}


void incrementRule() {
  nextRule = 0xff & (rule+1);

  return;
}


long getRandomColor () {
  //Now form a new RGB color
  long new_color = 0;
  for(int x = 0 ; x < 3 ; x++){
    new_color <<= 8;    
    //new_color <<= 8;
    new_color |= random(0xFF); //Give me a number from 0 to 0xFF
    //new_color &= 0xFFFFF0; //Force the random number to just the upper brightness levels. It sort of works.
  }
  return new_color;
}



//Takes the current strip color array and pushes it out
void post_frame (void) {
  //Each LED requires 24 bits of data
  //MSB: R7, R6, R5..., G7, G6..., B7, B6... B0 
  //Once the 24 bits have been delivered, the IC immediately relays these bits to its neighbor
  //Pulling the clock low for 500us or more causes the IC to post the data.

  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    long this_led_color = strip_colors[LED_number]; //24 bits of color data

    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      //Feed color bit 23 first (red data MSB)

      digitalWrite(CKI, LOW); //Only change data when clock is low

      long mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.

      if(this_led_color & mask) {
        digitalWrite(SDI, HIGH);
      }
      else {
        digitalWrite(SDI, LOW);
      }

      digitalWrite(CKI, HIGH); //Data is latched when clock goes high
    }
  }

  //Pull clock low to put strip into reset/post mode
  digitalWrite(CKI, LOW);

  int delay = random(10,100);
  delayMicroseconds(delay); //Wait for 500us to go into reset
}


void calcAutomaton() {

  for (int i = 0; i < STRIP_LENGTH; i++) {
    prev_strip_colors[i] = strip_colors[i]; 
  }

  long newColor = getRandomColor();
  for (int i = 0; i < STRIP_LENGTH; i++) {

    long selfVal= getSelf(i);
    long rightVal = getRight(i);
    long leftVal = getLeft(i);

    /*
     Serial.print(i, DEC);
     Serial.print(", ");
     Serial.print(self, DEC);
     Serial.print(", ");
     Serial.print(right, DEC);
     Serial.print(", ");
     Serial.print(left, DEC);
     Serial.println();
     */

    boolean isSelf = isOn(selfVal);
    boolean isRight = isOn(rightVal);
    boolean isLeft = isOn(leftVal);

    //rule 30
    /*  if ( isLeft) {
     
     strip_colors[i] = OFF;
     if ((!isSelf) && (!isRight)) {
     strip_colors[i] = getRandomColor();
     }
     
     } else if (isSelf) {
     strip_colors[i] = getRandomColor();
     } else if (isRight) {
     strip_colors[i] = getRandomColor();
     } else {
     strip_colors[i] = OFF;
     }  */

    //rule consts
    /* 
     /// RULE
     boolean allOn = true;
     boolean left2 = false;
     boolean outside = true;
     boolean left    = true;
     boolean right2  = false;
     boolean middle = true;
     boolean right = true;
     boolean allOff   = false;
     */

    boolean selfState = true;

    if ( isLeft) {
      if (isSelf) {
        if (isRight) {
          selfState = allOn;
        } 
        else {
          selfState = left2;  
        }
      } 
      else {
        if (isRight) {
          selfState = outside;
        } 
        else {
          selfState = left;  
        }
      }
    } 
    else {
      if (isSelf) {
        if (isRight) {
          selfState = right2;
        } 
        else {
          selfState = middle;  
        }
      } 
      else {
        if (isRight) {
          selfState = right;
        } 
        else {
          selfState = allOff;  
        }
      }  
    }

    if (selfState) {
      //strip_colors[i] = getRandomColor();
      strip_colors[i] = newColor;
    } 
    else {
      strip_colors[i] = OFF;
    }
  }
}


boolean isOn(long color) {
  if (OFF == color)  {
    return false;
  }

  return true;
}


long getRight(int index) {
  if (index == 0) {
    return prev_strip_colors[STRIP_LENGTH-1];
  }
  return prev_strip_colors[index-1];
}

long getLeft(int index) {
  if (index == STRIP_LENGTH-1){
    return prev_strip_colors[0];
  }
  return prev_strip_colors[index+1];
}

long getSelf(int index) {
  return prev_strip_colors[index];
}











