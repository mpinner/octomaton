/*
 AUTHOR: Matt Pinner
 
 */



// HARDWARE CONFIG
int ledPin[8] = {12, 11, 10, 17, 16, 15, 14, 13};
//int ledPin[8] = {13, 14, 15, 16, 17, 10, 11, 12};
int ledCount = 8;
//int buttonPin[8] = {2, 5, 9, 8, 7, 6, 3, 4};
int buttonPin[8] = {4, 3, 6, 7, 8, 9, 5, 2};
int buttonCount = 8;

 
//STATE
byte rule = 93;         // incoming serial byte
byte nextRule = rule;
boolean LED_STATE = true;

// DEBUG FLAG
boolean buttonDebug = true;


void setup()
{
  // start serial port at 9600 bps:
  Serial.begin(57600);
  
  for (int i = 0; i < buttonCount; i++) {
    digitalWrite(buttonPin[i], HIGH);
    pinMode(buttonPin[2], INPUT);   // digital sensor is on digital pin 2
  }
   
   
  for (int i = 0; i < ledCount; i++) { 
    pinMode(ledPin[i], OUTPUT);   // digital sensor is on digital pin 2
    digitalWrite(ledPin[i], HIGH);
  } 
       

  

}

void loop()
{
  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    // get incoming byte:
    rule = Serial.read();
    nextRule = rule;
    // read first analog input, divide by 4 to make the range 0-255:
    Serial.flush();
  }

    // delay 10ms to let the ADC recover:
    delay(200);
   // flipLed();
   for (int i = 0; i < buttonCount; i++) {
     boolean buttonState = digitalRead(buttonPin[i]);
   //  Serial.print(buttonState, DEC);
   //  digitalWrite(ledPin[i],  buttonState);
   //  Serial.print(",");
     
     if (LOW == buttonState) {
       nextRule = rule ^ (1 << i);
     }
     
   }
    
    
  //  Serial.println(rule, DEC);
    
    if (nextRule != rule) {
      rule = nextRule;
      Serial.print(rule);

    }
    
   for (int i = 0; i < ledCount; i++) {
    boolean isOn = rule & (1 << i); 
    digitalWrite(ledPin[i], (isOn));
     
   }
          
  
}

void flipLed() {
 LED_STATE = (false == LED_STATE);
    digitalWrite(13, LED_STATE);
    
       for (int i = 0; i < ledCount; i++) { 

     digitalWrite(ledPin[i],  LED_STATE);
   } 
    return;

}


