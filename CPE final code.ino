/*
Team: Dennis Brown, Dominic Bacci, Josh Rhoades
Team Name: JDD
Date: 12/12/2022
*/
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include "RTClib.h"
#include <DHT.h>
#include <DHT_U.h>
#define RDA 0x80
#define TBE 0x20 
#include <Stepper.h> // Include the header file
#define STEPS 64
Stepper stepper(STEPS, 25, 29, 27, 31);




#define DHTPIN 23
#define DHTTYPE DHT11
#define RDA 0x80
#define TBE 0x20 
LiquidCrystal lcd(2, 3, 9, 10, 11, 12);
DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 rtc;

volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;
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
volatile unsigned char* pin_l  = (unsigned char*) 0x109; 
unsigned int water_lvl = 0;
unsigned int water_lvl_threshold = 300;
int ledState = 1;
float temp_threshold = 81;
float temp;
float humid;

int previous = 0;
int potVal = 0;

bool disable_stat = true;
bool running = false;
bool state_change = false;
bool lcd_flag = false;
bool idle = false;
bool error = false;

int d_cur_state;
int err_state_pin;
int prev_err_state;
int d_prev_state;
/*
* SETUP
*/
void setup() {

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  U0init(9600);
  
  adc_init();
  lcd.begin(16, 2);
  dht.begin();
  stepper.setSpeed(200);
  set_PH_as_input(3);
  set_PL_as_input(1);
  set_PL_as_output(7);
  set_PD_as_output(7);
  set_PG_as_output(1);
  set_PG_as_output(5);
  set_PH_as_output(4); 
  set_PH_as_output(5);
  set_PE_as_output(3); 

  //blue light
  write_ph(5,0);
   //green light
  write_pe(3,0);
    //red light
  write_pg(5,0);
    //yellow light
  write_ph(4,ledState);
    
  d_cur_state = (*pin_h & 0x108);
  err_state_pin = (*pin_l & 0x10B);


}
/*
* MAIN LOOP
*/
void loop() {

if(idle == true && lcd_flag == false)
  set_lcd();


  else if(disable_stat == true)
  {
    if(state_change == false)
   print_time();

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
 else if(error == true){
   
     if(state_change == false)
   print_time();

   error_state();
 }
  
}
/*
* RUNNING STATE
*/
void running_state(){

read_stepper();
 write_pl(7,1);
 write_pd(7,1);
  
 write_ph(5,1);
 lcd_display();
 water_sense();
 if(temp <= temp_threshold){
   idle = true;
    running = false;
    state_change = false;
    write_ph(5,0);
     write_pl(7,0);
      write_pg(1,0);
  }
  if(water_lvl < water_lvl_threshold){
    idle = false;
    running = false;
    state_change = false;
    error = true;    
    write_pl(7,0);
    write_ph(5,0);
    write_pg(5,1);
    lcd.clear();
    lcd.write("error");
    lcd.setCursor(0, 2);
    lcd.write("Water lvl low");
  }
  if(start_stop()){
  running = false;
  disable_stat =  true;
  write_pl(7,0);
    write_ph(5,0);
  return;
}
}

void set_lcd(){
  lcd.clear();
lcd.print("Temp:  Humidity:");
lcd_flag = true;
}
/*
* IDLE STATE
*/
void idle_state(){
  //Serial.println("idleing! ");
 read_stepper();
 
lcd_display();
water_sense();

  if(temp > temp_threshold){
    write_pe(3,0);
    running = true;
    state_change = false;
  }
    if(water_lvl <= water_lvl_threshold){
    idle = false;
    running = false;
    state_change = false;
    error = true;    
    write_pl(7,0);
     write_pe(3,0);
    write_pg(5,1);
    lcd.clear();
    lcd.write("error");
    lcd.setCursor(0, 2);
    lcd.write("Water lvl low");
  }
  start_stop();
}
/*
* DISABLED STATE
*/
void disable_state(){
//Serial.println("DISABLED! ");
lcd.clear();
lcd_flag = false;
 d_prev_state = d_cur_state;
  d_cur_state = (*pin_h & 0x108);
  write_ph(4,1);

  if(d_prev_state >= 1 && d_cur_state == 0) {
    write_pe(3,1);
    write_ph(4,0);
    idle = true;
    disable_stat = false;
    state_change = false;
  }


}
/*
* ERROR STATE
*/
void error_state(){
  start_stop();
  read_stepper();
  prev_err_state = err_state_pin;
  err_state_pin = (*pin_l & 0x10B);

  if(prev_err_state >= 1 && err_state_pin == 0) {

  
  write_ph(4,1);
  write_pg(5,0);
    // toggle state of LED
    idle = true;
    lcd_flag = false;
    state_change = false;
    disable_stat = false;
    running = false;
    
  }
  //Serial.println("Error state! ");
}
/*
* LCD DISPLAY
*/
void lcd_display(){
  lcd.setCursor(0, 1);

  float h = dht.readHumidity();
  float t = dht.readTemperature(true);
   if (isnan(h) || isnan(t)) {
    //lcd.print("ERROR");
    return;
  }
  temp = t;
  humid = h;
  lcd.print(temp);
  lcd.setCursor(7,1);
  lcd.print(humid);
}
/*
* READ STEPPER MOTOR
*/
void read_stepper(){
  unsigned int adc_reading = adc_read(7);
potVal = map(adc_reading,0,1024,0,500);
if (potVal>previous)
  stepper.step(64);
if (potVal<previous)
  stepper.step(-64);
 previous = potVal;
 
}
/*
* WATER SENSOR
*/
void water_sense(){

  unsigned int adc_reading = adc_read(8);
  water_lvl = adc_reading;
  
}
/*
* START STOP BUTTON
*/
bool start_stop()
{
   d_prev_state = d_cur_state;
  d_cur_state = (*pin_h & 0x108);

  if(d_prev_state >= 1 && d_cur_state == 0) {

  if(idle == true){
    write_ph(4,1);
    write_pe(3,0);
    state_change = false;
    disable_stat = true;
    idle = false;
    return true;
  }
  else if(running == true){
    write_ph(4,1);
    write_ph(5,0);
     write_pl(7,0);
     write_pd(7,0);
    
    state_change = false;
    disable_stat = true;
    running = false;
    return true;
  }
  else if(error == true){
    write_ph(4,1);
    write_pg(5,0);
    state_change = false;
    disable_stat = true;
    error = false;
    return true;
  }
  }
  return false;
}
/*
* BELOW:
* PORT SETTING I/O
* PORT WRITING
* PRINT INTS
* PRINT CHARS
* PRINT TIME
*/
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
void write_pd(unsigned char pin_num, unsigned char state)
{
  if(state == 0)
  {
    *port_d &= ~(0x01 << pin_num);
  }
  else
  {
    *port_d |= 0x01 << pin_num;
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
void set_PL_as_input(unsigned char pin_num)
{
    *pin_l |= 0x01 << pin_num;
}
void print_time(){
  DateTime now = rtc.now();

  unsigned int year = (now.year());
  print_int(year);
   
   U0putchar('/');
    
    
    unsigned int month = now.month();
    print_int(month);
    U0putchar('/');
    
    unsigned int day = now.day();
    print_int(day);
     U0putchar('  ');
    
    unsigned int hour = now.hour();
    print_int(hour);
    U0putchar(':');
    
    unsigned int minute = now.minute();
    print_int(minute);
    U0putchar(':');
    
    unsigned int second = now.second();  
    print_int(second);                   
        U0putchar('\n');

    state_change = true;
}
void U0init(int U0baud)
{
 unsigned long FCPU = 16000000;
 unsigned int tbaud;
 tbaud = (FCPU / 16 / U0baud - 1);
 // Same as (FCPU / (16 * U0baud)) - 1;
 *myUCSR0A = 0x20;
 *myUCSR0B = 0x18;
 *myUCSR0C = 0x06;
 *myUBRR0  = tbaud;
}
void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b11011111; // clear bit 6 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11110111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11111000; // clear bit 0-2 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}
unsigned int adc_read(unsigned char adc_channel_num)
{
  // clear the channel selection bits (MUX 4:0)
  *my_ADMUX  &= 0b11100000;
  // clear the channel selection bits (MUX 5)
  *my_ADCSRB &= 0b11110111;
  // set the channel number
  if(adc_channel_num > 7)
  {
    // set the channel selection bits, but remove the most significant bit (bit 3)
    adc_channel_num -= 8;
    // set MUX bit 5
    *my_ADCSRB |= 0b00001000;
  }
  // set the channel selection bits
  *my_ADMUX  += adc_channel_num;
  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0x40;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register
  return *my_ADC_DATA;
}
void print_int(unsigned int out_num)
{
  // clear a flag (for printing 0's in the middle of numbers)
  unsigned char print_flag = 0;
  // if its greater than 1000
  if(out_num >= 1000)
  {
    // get the 1000's digit, add to '0' 
    U0putchar(out_num / 1000 + '0');
    // set the print flag
    print_flag = 1;
    // mod the out num by 1000
    out_num = out_num % 1000;
  }
  // if its greater than 100 or we've already printed the 1000's
  if(out_num >= 100 || print_flag)
  {
    // get the 100's digit, add to '0'
    U0putchar(out_num / 100 + '0');
    // set the print flag
    print_flag = 1;
    // mod the output num by 100
    out_num = out_num % 100;
  } 
  // if its greater than 10, or we've already printed the 10's
  if(out_num >= 10 || print_flag)
  {
    U0putchar(out_num / 10 + '0');
    print_flag = 1;
    out_num = out_num % 10;
  } 
  // always print the last digit (in case it's 0)
  U0putchar(out_num + '0');
  // print a newline
  //U0putchar('\n');
}
unsigned char U0kbhit()
{
  return *myUCSR0A & RDA;
}
unsigned char U0getchar()
{
  return *myUDR0;
}
void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A & TBE)==0);
  *myUDR0 = U0pdata;
}
