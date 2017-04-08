#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

void dac_init()
{
  dac.begin(0x62);
}

void set_dac(float voltage)
{
        if(voltage < 0.0 || voltage > 5.0) {
          voltage = 2.0;
        }
  
        uint32_t scale = (uint32_t)((voltage * (4096.0 / 5.0)) - 1);
        dac.setVoltage(pgm_read_word(&(scale)), false);
}


