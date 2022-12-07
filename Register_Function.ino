
volatile unsigned char* ddr_e  = (unsigned char*) 0x2D; 
volatile unsigned char* port_e  = (unsigned char*) 0x2E; 

void setup() {
//---set pin direction
set_PE_as_output(6); 
set_PEas_output(5);
set_PE_as_output(4); 
Serial.begin(9600);
}
void loop() {

write_pe(, ); 

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


