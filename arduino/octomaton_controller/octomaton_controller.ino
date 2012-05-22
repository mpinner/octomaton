/*
 AUTHOR: Matt Pinner
 
 */

#define DEBUG_RULE    0
#define DEBUG_SERIAL  0
#define ECHO_RULE     0

// HARDWARE CONFIG
const int ledCount = 8;
int ledPin[ledCount] = {
  12, 11, 10, 17, 16, 15, 14, 13};
//int ledPin[ledCount] = {13, 14, 15, 16, 17, 10, 11, 12};
const int buttonCount = 8;
int buttonPin[buttonCount] = {
  4, 3, 6, 7, 8, 9, 5, 2};
//int buttonPin[ledCount] = {2, 5, 9, 8, 7, 6, 3, 4};

#define ALLON_MSK    0x80
#define LEFT2_MSK    0x40
#define OUTSIDE_MSK  0x20
#define LEFT_MSK     0x10
#define RIGHT2_MSK   0x08
#define MIDDLE_MSK   0x04
#define RIGHT_MSK    0x02
#define ALLOFF_MSK   0x01

//STATE
byte rule = (LEFT2_MSK | LEFT_MSK | RIGHT2_MSK | MIDDLE_MSK | ALLOFF_MSK);
byte nextRule = rule;

boolean nextRulePending = false;

// DEBUG FLAG
boolean buttonDebug = true;


void setup()
{
  // start serial port at 9600 bps:
  Serial.begin(57600);

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttonPin[i], INPUT);
    digitalWrite(buttonPin[i], HIGH);
  }

  for (int i = 0; i < ledCount; i++) { 
    digitalWrite(ledPin[i], HIGH);
    pinMode(ledPin[i], OUTPUT);
  } 
}

void serialEvent(){
  char c;

  if (Serial.available() > 0) {
    // get incoming byte:
    c = Serial.read();
    if (c != '\n') {
      rule = c;
#if DEBUG_SERIAL
      Serial.print("s:  ");
      Serial.println(nextRule,HEX); 
#endif
    }

    // only taking first byte. flush remaining serial buffer.
    Serial.flush();
  }
}

void loop()
{
  // if we get a valid byte, read analog ins:
  // mirror changes rules
  if (nextRulePending) {
    //if (rule == nextRule) {
      // rule got there
      nextRulePending = false;
    //} 
    //else {
    // rule didnt get there. will resend at the end?
    //}
  } 
  else {
    if (rule != nextRule) { 
      // rule incremented on mirror side
      // test if increment it is by one?
      nextRule = rule;
      // echo rule back?
    } 
    else {
      // should do real debounce
      delay(50);
      for (int i = 0; i < buttonCount; i++) {
        boolean buttonState = digitalRead(buttonPin[i]);
        //Serial.print(buttonState, DEC);
        //digitalWrite(ledPin[i],  buttonState);
        //Serial.print(",");

        if (LOW == buttonState) {
          nextRule = rule ^ (1 << i);
        }
      }
    }
  }

  for (int i = 0; i < ledCount; i++) {
    boolean isOn = ((rule & (1 << i)) ? true : false); 
    digitalWrite(ledPin[i], (isOn));
  }

  if (nextRule != rule) {
    // something changed or didnt (on the mirror side)
    nextRulePending = true;    
    // send update
    //Serial.println(nextRule,HEX);
    Serial.print((char)nextRule);
  }

  delay(150);
}


