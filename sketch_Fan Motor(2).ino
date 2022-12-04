
int i;

volatile unsigned char* ddr_e  = (unsigned char*) 0x2D; 
volatile unsigned char* port_e  = (unsigned char*) 0x2E; 


void setup() {
//---set pin direction
set_PB_as_output(5); //enable
set_PB_as_output(4); //DIRB
set_PB_as_output(3); //DIRA
Serial.begin(9600);
}
void loop() {
//---back and forth example
Serial.println("One way, then reverse");
write_pb(5, 1); // enable on
for (i=0;i<5;i++) {
write_pb(3, 1); //one way
write_pb(4, 0);
delay(500);
write_pb(3, 0); //reverse
write_pb(4, 1);
delay(500);
}
write_pb(5, 1); // disable
delay(2000);
Serial.println("fast Slow example");
}
void set_PB_as_output(unsigned char pin_num)
{
    *ddr_e |= 0x01 << pin_num;
}
void write_pb(unsigned char pin_num, unsigned char state)
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
