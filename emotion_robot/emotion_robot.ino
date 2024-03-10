/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  The 1.14" TFT breakout
  ----> https://www.adafruit.com/product/4383
  The 1.3" TFT breakout
  ----> https://www.adafruit.com/product/4313
  The 1.54" TFT breakout
    ----> https://www.adafruit.com/product/3787
  The 1.69" TFT breakout
    ----> https://www.adafruit.com/product/5206
  The 2.0" TFT breakout
    ----> https://www.adafruit.com/product/4311
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <TimerOne.h>
#include <SPI.h>
#include <Servo.h>
#include "SR04.h"

#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        10
  #define TFT_RST        8 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         9

#endif

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#define TRIG_PIN 4
#define ECHO_PIN 6
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position


float p = 3.1415926;
int light_val = 0;
int prev_light_val = 0;
int cnt_blink = 0;
int sleep_cnt = 0;
int digitalVal= 0;
int prevDigitalVal = 0;
int far_cnt = 0;

int timer=0;

const int ballSwitchPin = 2;

long dist = 0;
long prev_dist = 0;
bool scared_on = false;

unsigned long prev_time = 0;

void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));

  // OR use this initializer (uncomment) if using a 1.44" TFT:
  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  tft.setSPISpeed(40000000);

  Serial.println(F("Initialized"));
  tft.setRotation(0);  // Setează rotația ecranului, dacă este necesar
  tft.fillScreen(ST7735_BLACK);  // Umple ecranul cu culoarea de fundal inițială

  //initializare foto-rezistenta
  analogReference(DEFAULT); //setarea tensiunii de referinta la tensiunea default 
  pinMode(A0, INPUT); // setarea pinului analogic A1 ca si pin de input 
  digitalWrite(A0, HIGH); //activarea rezistorului pull up pentru pinul A

  //initializare senzor de inclinatie
  pinMode(ballSwitchPin,INPUT);
  digitalWrite(ballSwitchPin, HIGH);

  //initializare servo
  myservo.attach(5);  // attaches the servo on pin 9 to the servo object

  //blinking timer
  // initialize Timer1
  /*
  cli();
  TCCR0A=(1<<WGM01);    //Set the CTC mode   
  OCR0A=0xF9; //Value for ORC0A for 1ms 
  
  TIMSK0|=(1<<OCIE0A);   //Set  the interrupt request
  sei(); //Enable interrupt
  
  TCCR0B|=(1<<CS01);    //Set the prescale 1/64 clock
  TCCR0B|=(1<<CS00); 
  */
 // Timer1.initialize(1000000); //se initializeaza intervalul de timp la care temporizatorul va 
// declanşa evenimente (in microsecunde, 1000000 microsecunde = 1 secunda) 
 //Timer1.attachInterrupt(change_state); // funcŃia ShowMessage se va apela la intevalul 
// stabilit 
  Serial.println("done");
}

void dist_check(){
  if (dist < 15){
    if(prev_dist - dist >= 30){
      scared_on = true;
      scared();
      tft.fillScreen(ST77XX_BLACK);
      tft.fillRoundRect(25, 14, 55, 40, 15, ST77XX_WHITE);
      tft.fillRoundRect(25, 74, 55, 40, 15, ST77XX_WHITE);
      scared_on = false;
    }
    else if (prev_dist >=15){
      tft.fillScreen(ST77XX_BLACK);
      tft.fillRoundRect(25, 14, 55, 40, 15, ST77XX_WHITE);
      tft.fillRoundRect(25, 74, 55, 40, 15, ST77XX_WHITE);
    }
    happy();
  }
  else if(dist <= 100){
    far_cnt = 0;
  }
  else if(dist > 100){
    if(prev_dist <=100){
      far_cnt = 0;
    }
    if(far_cnt >= 20){
      if(far_cnt == 20){
        sad();
      }
      tft.fillTriangle(25, 14, 75, 14, 50, 54, ST77XX_BLACK);
      tft.fillRect(25, 14, 25, 40, ST77XX_BLACK);
      tft.fillTriangle(25, 114, 75, 114, 50, 74, ST77XX_BLACK);
      tft.fillRect(25, 74, 25, 40, ST77XX_BLACK);
    }
  }
}

void loop() {
  //normal_look();
  //if(scared_on == false){
  if(millis() - prev_time >= 1000){
    change_state();
    prev_time = millis();
  }
  if (light_val < 150) {
    if(prev_light_val >=150){
      sleep_cnt = 0;
    }
    if(sleep_cnt == 3){
      sleepy();
    }
    else if(sleep_cnt == 5){
      sleepy();
    }
    else if(sleep_cnt > 7){
      if(sleep_cnt == 8){
        sleep_mode();
      }
      tft.fillRect(50, 0, 5, 128, ST77XX_WHITE);
    }
  }
  else if (light_val < 450 ) {
  //else if (light_val < 450) { 
    if(LOW == digitalVal){
      if(HIGH == prevDigitalVal){
        tft.fillScreen(ST77XX_BLACK);
      }
      dist_check();
      if((dist <= 100 && dist >=15)|| (dist > 100 && far_cnt < 20)){
        if(prev_light_val >= 450){
          cnt_blink = 0; 
        }
        else if(prev_light_val < 150){
          tft.fillScreen(ST77XX_BLACK);
          wake_up();
          sleepy();
          cnt_blink = 0;
        }
        
        if(cnt_blink%5 == 0){
          eye_blink();
          delay(20);
          cnt_blink += 1;
          tft.fillScreen(ST77XX_BLACK);
        }
        if(cnt_blink > 15 && cnt_blink <= 20){
          look_right();
        }
        else if(cnt_blink > 20 && cnt_blink <= 25){
          look_left();
        }
        else{
          if(cnt_blink > 25){
            cnt_blink = 1;
          }
          normal_look();
        }
      }
    }
    else{
      if(LOW == prevDigitalVal){
        tft.fillScreen(ST77XX_BLACK);
        normal_look();
      }
      angry();
    }
    //normal_look();
  }
  else if (light_val < 600) {
  //else if (light_val < 600 || HIGH == digitalVal) {
    if(prev_light_val < 450 || prev_light_val >= 600){
      tft.fillScreen(ST77XX_BLACK); 
    }
    bright();
  }
  else{
    if(prev_light_val < 600){
      tft.fillScreen(ST77XX_BLACK); 
    }
    too_bright();
  }
  //}
}

void normal_look() {
  // center look
  tft.fillRoundRect(25, 14, 55, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(25, 74, 55, 40, 8, ST77XX_WHITE);
}

void look_left() {
  // left look
  tft.fillRoundRect(25, 21, 55, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(25, 81, 55, 40, 8, ST77XX_WHITE);
}

void look_right() {
  // right look
  tft.fillRoundRect(25, 7, 55, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(25, 67, 55, 40, 8, ST77XX_WHITE);
}

//ISR(TIMER0_COMPA_vect){    //This  is the interrupt request
//  timer++;
//  if(timer>=1000){
//    change_state();
//    timer=0;
//  }
//}

void change_state()
{
  prev_dist = dist;
  dist=sr04.Distance();
  prevDigitalVal = digitalVal;
  digitalVal = digitalRead(ballSwitchPin);
  prev_light_val = light_val;
  light_val = analogRead(A0); //citirea valorii analogice 
  cnt_blink += 1;
  if(light_val < 150){
    sleep_cnt += 1;
  }
  if(dist > 100){
    far_cnt += 1;
  }
  Serial.println(light_val);
  Serial.print(dist);
  Serial.println("cm");
}

void eye_blink()
{
  //blink
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(50, 14, 10, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(50, 74, 10, 40, 8, ST77XX_WHITE);
}

void bright()
{
  //blink
  tft.fillRoundRect(50, 14, 10, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(50, 74, 10, 40, 8, ST77XX_WHITE);
  delay(1000);
}

void too_bright()
{
  //blink
  tft.fillTriangle(35, 14, 65, 14, 50, 54, ST77XX_WHITE);
  tft.fillTriangle(35, 0, 65, 0, 50, 40, ST77XX_BLACK);
  tft.fillRoundRect(50, 74, 10, 40, 8, ST77XX_WHITE);
  delay(1000);
}

void sleepy()
{
  //blink
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 14, 55, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(25, 74, 55, 40, 8, ST77XX_WHITE);
  for(int i = 0; i <=20; i++)
  {
    tft.fillTriangle(25, 14, 25+2*i, 14, 25+i, 55, ST77XX_BLACK);
    tft.fillRect(25, 14, i, 40, ST77XX_BLACK);
    tft.fillTriangle(25, 114, 25+2*i, 114, 25+i, 75, ST77XX_BLACK);
    tft.fillRect(25, 74, i, 40, ST77XX_BLACK);
    delay(5);
  }
  for(int i = 20; i >=0; i--)
  {
    tft.fillRoundRect(25+i, 14, 55-i, 40, 8, ST77XX_WHITE);
    tft.fillTriangle(25, 14, 25+2*i, 14, 25+i, 55, ST77XX_BLACK);
    tft.fillRect(25, 14, i, 40, ST77XX_BLACK);
    tft.fillRoundRect(25+i, 74, 55-i, 40, 8, ST77XX_WHITE);
    tft.fillTriangle(25, 114, 25+2*i, 114, 25+i, 75, ST77XX_BLACK);
    tft.fillRect(25, 74, i, 40, ST77XX_BLACK);
  }
}

void sleep_mode()
{
  for(int i = 0; i <=25; i++)
  {
    tft.fillRect(25, 0, i, 128, ST77XX_BLACK);
    tft.fillRect(80 - i, 0, i, 128, ST77XX_BLACK);
    delay(5);
  }
}

void wake_up()
{
  tft.fillScreen(ST77XX_BLACK);
  for(int i = 25; i >=0; i--)
  {
    tft.fillRoundRect(25+i, 14, 5+2*(25-i), 40, 8, ST77XX_WHITE);
    tft.fillRoundRect(25+i, 74, 5+2*(25-i), 40, 8, ST77XX_WHITE);
  }
}

void angry()
{
  //tft.fillRoundRect(25, 14, 55, 40, 8, ST77XX_WHITE);
  //tft.fillRoundRect(25, 74, 55, 40, 8, ST77XX_WHITE);
  tft.fillTriangle(15, 54, 35, 14, 55, 54, ST77XX_BLACK);
  tft.fillTriangle(15, 74, 35, 114, 55, 74, ST77XX_BLACK);
  tft.fillRect(15, 14, 20, 100, ST77XX_BLACK);
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(2);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(2);                       // waits 15ms for the servo to reach the position
  }
}

void scared()
{
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 14, 55, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(25, 74, 55, 40, 8, ST77XX_WHITE);
  tft.fillTriangle(15, 54, 25, 19, 55, 54, ST77XX_WHITE);
  tft.fillTriangle(15, 74, 25, 109, 55, 74, ST77XX_WHITE);
  tft.fillRect(50, 14, 40, 100, ST77XX_BLACK);
  delay(2000);
}

void sad()
{
  //blink
  tft.fillScreen(ST77XX_BLACK);
  tft.fillRoundRect(25, 14, 55, 40, 8, ST77XX_WHITE);
  tft.fillRoundRect(25, 74, 55, 40, 8, ST77XX_WHITE);
  for(int i = 0; i <=25; i++)
  {
    tft.fillTriangle(25, 14, 25+2*i, 14, 25+i, 54, ST77XX_BLACK);
    tft.fillRect(25, 14, i, 40, ST77XX_BLACK);
    tft.fillTriangle(25, 114, 25+2*i, 114, 25+i, 74, ST77XX_BLACK);
    tft.fillRect(25, 74, i, 40, ST77XX_BLACK);
    delay(5);
  }
  delay(1000);
}

void happy()
{
  //blink
  tft.fillRoundRect(35, 14, 55, 40, 15, ST77XX_BLACK);
  tft.fillRoundRect(35, 74, 55, 40, 15, ST77XX_BLACK);
  delay(1000);
}
