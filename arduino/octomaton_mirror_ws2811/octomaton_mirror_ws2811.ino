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
 
 
 */

#include <Adafruit_NeoPixel.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
#define STRIP_LENGTH 60 
#define STRIP_DATA_PIN 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LENGTH, STRIP_DATA_PIN, NEO_GRB + NEO_KHZ800);

// a value from 0x0F to 0xFF 
#define BRIGHTNESS 0x0F
#define DELAY_MS 130
#define STEPS_PER_RULE 100



#define DEBUG_RULE    1
#define DEBUG_SERIAL  0
#define ECHO_RULE     0


boolean strip_colors[STRIP_LENGTH];
boolean prev_strip_colors[STRIP_LENGTH];

uint32_t OFF = strip.Color(0,0,0);


//
// COLOR AND SEQUENCE CONFIGS
//
// how many interations per rule
// how long between interations


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
//byte rule = (ALLOFF_MSK);
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
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(57600);
  //Serial.println("Hello!");

  setupRule();

  //Clear out the array
  for(int x = 0 ; x < STRIP_LENGTH ; x++) {
    strip_colors[x] = false;
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

  /*
 for (int i = 0; i < STRIP_LENGTH; i++) {
   
   strip.setPixelColor(i, getPixelColor());
   
   }
   */
  ///*
  if ( seqStep > STEPS_PER_RULE ) {
    incrementRule();
  }  

  if (rule != nextRule) { 
    rule = nextRule;
    setupRule();
    startSeq();
  }

  calcAutomaton();
  //*/
  post_frame();

  delay(DELAY_MS);                  // wait for a little while

  seqStep++;

  return;
}


void post_frame () {
  uint32_t new_color = getPixelColor();
  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    if (  strip_colors[LED_number]) {
      strip.setPixelColor(LED_number, new_color);
    } 
    else {
      strip.setPixelColor(LED_number, OFF);
    }
    strip.show(); // Initialize all pixels to 'off'
  }
  return;
}


void startSeq() {
  seqStep = 0;
  //Pre-fill the color array with known values
  //Bright Red
  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    strip_colors[LED_number] = false; //Bright Red

  }


  strip_colors[random(60)] = true;

  post_frame();
  delay(DELAY_MS);

  return;
}

//Throws random colors down the strip array
void addRandom(void) {
  int x;

  int swapOutLed = random(STRIP_LENGTH);

  //Now form a new RGB color
  strip_colors[swapOutLed] = true; //Add the new random color to the strip
}


void incrementRule() {
  nextRule = 0xff & (rule+1);
  return;
}



uint32_t getPixelColor() {

  //Now form a new RGB color
  uint16_t r_color = 0;
  r_color <<= 8;    
  r_color |= random(BRIGHTNESS); //Give me a number from 0 to 0xFF

  //Now form a new RGB color
  uint16_t g_color = 0;
  g_color <<= 8;    
  g_color |= random(BRIGHTNESS); //Give me a number from 0 to 0xFF

  //Now form a new RGB color
  uint16_t b_color = 0;
  b_color <<= 8;    
  b_color |= random(BRIGHTNESS); //Give me a number from 0 to 0xFF

  return strip.Color(r_color, g_color, b_color);

}






void calcAutomaton() {

  for (int i = 0; i < STRIP_LENGTH; i++) {
    prev_strip_colors[i] = strip_colors[i]; 
  }

  uint32_t newColor = getPixelColor();
  for (int i = 0; i < STRIP_LENGTH; i++) {

    boolean selfVal= getSelf(i);
    boolean rightVal = getRight(i);
    boolean leftVal = getLeft(i);

    if (DEBUG_SERIAL) {

      Serial.print(i, DEC);
      Serial.print(", ");
      Serial.print(rightVal, BIN);
      Serial.print(", ");
      Serial.print(selfVal, BIN);
      Serial.print(", ");
      Serial.print(leftVal, BIN);
      Serial.println();
    }

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
      strip_colors[i] = true;
      // strip.setPixelColor(i, newColor);
      strip.setPixelColor(i, newColor);

    } 
    else {
      strip_colors[i] = false;

    }
  }
}


boolean isOn(boolean color) {
  return color;
}


boolean getRight(int index) {
  if (index == 0) {
    return prev_strip_colors[STRIP_LENGTH-1];
  }
  return prev_strip_colors[index-1];
}

boolean getLeft(int index) {
  if (index == STRIP_LENGTH-1){
    return prev_strip_colors[0];
  }
  return prev_strip_colors[index+1];
}

boolean getSelf(int index) {
  return prev_strip_colors[index];
}

















