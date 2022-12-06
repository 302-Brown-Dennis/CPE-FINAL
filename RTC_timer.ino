// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
void setup () {

Serial.begin(9600);
 rtc.begin();



rtc.adjust(DateTime(12, 22, 12, 1, 5, 1));
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//rtc.setTime();
}
void loop () {
 DateTime now = rtc.now();
  Serial.print(" Year: ");
 Serial.print(now.year(), DEC);
 Serial.print(" month: ");
 Serial.print(now.month(), DEC);
 Serial.print(" day: ");
 Serial.print(now.day(), DEC);
 Serial.print(" hour: ");
 
 int hour = now.hour();
Serial.print(hour);
 
 Serial.print("day of week: ");
 Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
 
 
 delay(3000);
}