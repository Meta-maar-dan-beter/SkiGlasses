#define SI1145_ADDR 0x60
#ifdef ARDUINO_AVR_UNO
#include <Wire.h>
#endif

#ifdef ARDUINO_attiny
#include <TinyWireM.h>
#define Wire TinyWireM
#endif

typedef struct {
    unsigned int vis;
    unsigned int uv;
} SI1145_value;

int SI1145_init_sensor();
SI1145_value SI1145_read_sensor();
