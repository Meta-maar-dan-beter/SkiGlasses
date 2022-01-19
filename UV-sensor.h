#include <Wire.h>
#define SI1145_ADDR 0x60

typedef struct {
    unsigned int vis;
    unsigned int uv;
} SI1145_value;

int SI1145_init_sensor();
SI1145_value SI1145_read_sensor();
