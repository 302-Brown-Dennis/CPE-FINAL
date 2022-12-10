#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include "RTClib.h"
#include <DHT.h>
#include <DHT_U.h>
#include <Stepper.h> // Include the header file
#define STEPS 32
Stepper stepper(STEPS, 25, 27, 29, 31);


int Pval = 0;

int potVal = 0;

#define DHTPIN 23
#define DHTTYPE DHT11
#define RDA 0x80
#define TBE 0x20 
LiquidCrystal lcd(2, 3, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;
//enable 42 pL7
//dira 40 pg1
//dirb 38 pd7
volatile unsigned char* ddr_l  = (unsigned char*) 0x10A; 
volatile unsigned char* port_l  = (unsigned char*) 0x10B;
volatile unsigned char* ddr_d  = (unsigned char*) 0x2A; 
volatile unsigned char* port_d  = (unsigned char*) 0x2B;
volatile unsigned char* ddr_h  = (unsigned char*) 0x101; 
volatile unsigned char* port_h  = (unsigned char*) 0x102; 
volatile unsigned char* ddr_e  = (unsigned char*) 0x2D; 
volatile unsigned char* port_e  = (unsigned char*) 0x2E; 
volatile unsigned char* ddr_g  = (unsigned char*) 0x33; 
volatile unsigned char* port_g  = (unsigned char*) 0x34; 
volatile unsigned char* pin_h  = (unsigned char*) 0x100; 
int ledState = 1;
int previous = 0;
bool disable_stat = true;
bool running = false;
bool state_change = false;
bool lcd_flag = false;
bool idle = false;
int d_cur_state;
int d_prev_state;
void setup() {
rtc.begin();
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // put your setup code here, to run once:
  Serial.begin(9600);
    lcd.begin(16, 2);
  dht.begin();
  stepper.setSpeed(200);
   set_PH_as_input(3);

  set_PL_as_output(7);
  set_PD_as_output(7);
  set_PG_as_output(1);

  set_PH_as_output(4); 
  set_PH_as_output(5);
  // set_PG_as_output(5); 
   set_PE_as_output(3); 
 
  //set_PEas_output(5);
  //set_PE_as_output(4); 
  //blue light
   write_ph(5,0);
   //green light
    write_pe(3,0);
    //yellow light
    write_ph(4,ledState);
    d_cur_state = (*pin_h & 0x108);



}

void loop() {
 //digitalWrite(5, LOW);
potVal = map(analogRead(A7),0,1024,0,500);

 stepper.step(potVal - previous);
 Serial.print(potVal);
Serial.print(" - ");
Serial.print(previous);
Serial.println();
  previous = potVal;



 
if(idle == true && lcd_flag == false)
  set_lcd();


  else if(disable_stat == true)
  {
     disable_state();
    
  }

 else if(running == true)
 {
   if(state_change == false)
   print_time();

   running_state();
 }
 else if (idle == true){
   write_ph(4,0);
   write_pe(3,1);
    if(state_change == false)
   print_time();

   idle_state();
 }
  
 
  // put your main code here, to run repeatedly:
  
}
void running_state(){
//print_time();
//Serial.println("RUNNING! ");
 write_pl(7,1);
 write_pg(1,1);  
 write_ph(5,1);
 float f = dht.readTemperature(true);
 if(f <= 82){
   idle = true;
    running = false;
    state_change = false;
    write_ph(5,0);
     write_pl(7,0);
      write_pg(1,0);
  }
}

void set_lcd(){
lcd.print("Temp:  Humidity:");
lcd_flag = true;
}
void idle_state(){
  Serial.println("idleing! ");
 // delay(200);
 // write_pe(3,1);
    lcd.setCursor(0, 1);

  float h = dht.readHumidity();
  float f = dht.readTemperature(true);
   if (isnan(h) || isnan(f)) {
    lcd.print("ERROR");
    return;
  }
   lcd.print(f);
  lcd.setCursor(7,1);
  lcd.print(h);
 // water_sense();

  if(f > 82){
    write_pe(3,0);
    running = true;
    state_change = false;
  }
  

  //idle_state();
}
void disable_state(){
//Serial.println("DISABLED! ");
 d_prev_state = d_cur_state;
 //Serial.print(d_prev_state);
  d_cur_state = (*pin_h & 0x108);
   //Serial.print(d_cur_state);

  if(d_prev_state >= 1 && d_cur_state == 0) {

  //print_time();
  write_ph(4,0);
    // toggle state of LED
    idle = true;
    //digitalWrite(5, HIGH);
   // ledState = !ledState;
disable_stat = false;
    // control LED arccoding to the toggled state
    //write_ph(4,ledState);
    
  }

}
void set_PH_as_output(unsigned char pin_num)
{
    *ddr_h |= 0x01 << pin_num;
}
void set_PL_as_output(unsigned char pin_num)
{
    *ddr_l |= 0x01 << pin_num;
}
void set_PD_as_output(unsigned char pin_num)
{
    *ddr_d |= 0x01 << pin_num;
}
void set_PG_as_output(unsigned char pin_num)
{
    *ddr_g |= 0x01 << pin_num;
}
void set_PE_as_output(unsigned char pin_num)
{
    *ddr_e |= 0x01 << pin_num;
}
void write_pl(unsigned char pin_num, unsigned char state)
{
  if(state == 0)
  {
    *port_l &= ~(0x01 << pin_num);
  }
  else
  {
    *port_l |= 0x01 << pin_num;
  }
}
void write_pg(unsigned char pin_num, unsigned char state)
{
  if(state == 0)
  {
    *port_g &= ~(0x01 << pin_num);
  }
  else
  {
    *port_g |= 0x01 << pin_num;
  }
}
void write_pe(unsigned char pin_num, unsigned char state)
{
  if(state == 0)
  {
    *port_e &= ~(0x01 << pin_num);
  }
  else
  {
    *port_e |= 0x01 << pin_num;
  }
}
void write_ph(unsigned char pin_num, unsigned char state)
{
  if(state == 0)
  {
    *port_h &= ~(0x01 << pin_num);
  }
  else
  {
    *port_h |= 0x01 << pin_num;
  }
}
void set_PH_as_input(unsigned char pin_num)
{
    *pin_h |= 0x01 << pin_num;
}
void print_time(){
  DateTime now = rtc.now();
   Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
     Serial.print('/');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    state_change = true;
}
