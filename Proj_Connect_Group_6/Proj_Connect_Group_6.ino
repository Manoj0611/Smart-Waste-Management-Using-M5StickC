/* PROJECT CONNECT GROUP-6 SMART WASTE MANAGEMENT SYSTEM
* This is the code implementation of smart waste management system integrating with the adafruit io cloud 
* and as well as ifttt for trigerring mail / message when the garbage is 85% filled.
*/

#include "Ultrasonic.h"
#include "config.h"
#include <M5StickC.h>

const float homeTrashHeight = 50; 
const float commercialTrashHeight = 100;

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
double xAngle ;
long RangeInCentimeters;
float distanceValue;
float garbageFilledPercentage;
String garbageFilledStatus;
int delayCount = 0;
static bool openedLid = false;
// function to detect whether garbage lid is opened or not and triggering the data to adafruit accordingly.
void accelerometerTriggerDetection(double); 
// function to read the ultrasonic sensor value.
float readUltrasonicvalue();


// set up the 'group6' feed
AdafruitIO_Feed *group6 = io.feed("group6");    
// set up the ultrasonic sensor
Ultrasonic ultrasonic(33);

void setup() {
  
    M5.begin();
    Serial.begin(115200);
    M5.IMU.Init();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(0, 10);
    M5.Lcd.println("Angle - X ");
    
    Serial.print("Connecting to Adafruit IO");
    // connect to io.adafruit.com
    io.connect();
    // wait for a connection
    while(io.status() < AIO_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    // we are connected
    Serial.println();
    Serial.println(io.statusText());
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
}

void loop() {

    io.run();
    M5.IMU.getAccelData(&accX,&accY,&accZ);  
    xAngle = atan( accX / (sqrt(sq(accY) + sq(accZ)))); // got the angle calcuation from https://wizmoz.blogspot.com/2013/01/simple-accelerometer-data-conversion-to.html
    xAngle *= 180.00;
    xAngle /= 3.141592;
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf(" %5.2f ",xAngle);
    // The accelerometer angle determines whether the lid is opened or not.
    accelerometerTriggerDetection(xAngle);
    delay(1000);
}

void accelerometerTriggerDetection(double xAngle){
    
    if(xAngle >= 25 || xAngle <= -25){
        Serial.print("Angle - ");Serial.println(xAngle);
        openedLid = true;
        digitalWrite(10, LOW); // Switching the LED on when the lid is opened.
      }
      
    if(openedLid) { 
      delay(5000);
      delayCount ++ ; 
      // (i) After opening the lid and delay of 10 seconds , checking whether the lid is closed .
      // If lid is closed , then reading the ultrasonic value and converting to the percentage of garbage filled.
      // Then sending the filled % to adafruit feed to analyze the values and viewing in the dashboard. 
      if(xAngle <= 25 && xAngle >= -25) {
        delayCount = 0;
        openedLid = false;
        digitalWrite(10, HIGH); // Switching the LED on when the lid is closed.
        Serial.println("Calculating garbage status after the dustbin is closed");
        distanceValue = homeTrashHeight - readUltrasonicvalue();
        garbageFilledPercentage = (distanceValue / homeTrashHeight ) * 100;
        if(garbageFilledPercentage < 0){
          return ;
          }
        garbageFilledStatus = String(garbageFilledPercentage); 
        Serial.print(" % of Garbage Filled - "); Serial.println(garbageFilledStatus);
        group6->save(garbageFilledStatus);
        
      } 
      //(ii) After opening the lid and delay of 25 secs ,if the lid is still not closed (Handling Error condition).
      // Then sending the error trigger value "Garbage Lid is not closed" to adafruit feed.
      else if((xAngle >= 25 || xAngle <= -25) && delayCount > 4) {
        delayCount = 0;
        // sending garabage value for Errant Condition ( The lid is not closed ) 
        garbageFilledStatus = "Garbage Lid is not closed" ;
        Serial.println(garbageFilledStatus);
        group6->save(garbageFilledStatus);       
      }
    }
}

float readUltrasonicvalue(){
    RangeInCentimeters = ultrasonic.MeasureInCentimeters(); 
    Serial.print("Ultrasonic Distance -  "); Serial.print(RangeInCentimeters); Serial.println(" cm");
    M5.Lcd.setCursor(0, 40);  M5.Lcd.print(" Ultrasonic Sensor Value(in cm) - ");
    M5.Lcd.setCursor(0, 50);  M5.Lcd.printf(" %ld " ,RangeInCentimeters);
    return float(RangeInCentimeters);
}
