
int i;

volatile unsigned char* ddr_e  = (unsigned char*) 0x2D; 
volatile unsigned char* port_e  = (unsigned char*) 0x2E; 


void setup() {
//---set pin direction
set_PE_as_output(6); //enable
set_PEas_output(5); //DIRB
set_PE_as_output(4); //DIRA
Serial.begin(9600);
}
void loop() {
//---back and forth example
Serial.println("One way, then reverse");
write_pe(6, 1); // enable on
/*for (i=0;i<5;i++) {
write_pe(4, 1); //one way
write_pe(5, 0);
delay(500);
write_pe(4, 0); //reverse
write_pe(5, 1);
delay(500);
}
write_pe(6, 1); // disable
delay(2000);
Serial.println("fast Slow example");
*/
}
void set_PE_as_output(unsigned char pin_num)
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
