#include "UV-sensor.h"

void SI1145_setRegister(uint8_t reg, uint8_t val) {
    TinyWireM.beginTransmission(SI1145_ADDR);
    TinyWireM.write(reg);
    TinyWireM.write(val);
    TinyWireM.endTransmission();   
}

uint8_t SI1145_getRegister(uint8_t reg) {
    TinyWireM.beginTransmission(SI1145_ADDR);
    TinyWireM.write(reg);
    TinyWireM.endTransmission();
    TinyWireM.requestFrom(SI1145_ADDR, 1);
    return TinyWireM.read();
}

uint16_t SI1145_getRegister16(uint8_t reg) {
    TinyWireM.beginTransmission(SI1145_ADDR);
    TinyWireM.write(reg);
    TinyWireM.endTransmission();
    TinyWireM.requestFrom(SI1145_ADDR, 2);
    return TinyWireM.read() | (TinyWireM.read() << 8);
}

void SI1145_sendCommand(uint8_t command) {
    SI1145_setRegister(0x18, command);
}

void SI1145_setParameter(uint8_t par, uint8_t val) {
    SI1145_setRegister(0x17, val); // PARAM_WR
    SI1145_sendCommand(0b10100000 | par); // PARAM_SET with parameter
}

int SI1145_init_sensor() {
    TinyWireM.begin();
  
    uint8_t id = SI1145_getRegister(0); // check ID, should be 0x45
    if (id != 0x45) return -1;

    SI1145_sendCommand(1); // reset
    
    SI1145_setRegister(0x07, 0x17); // must write 0x17 to HW_KEY for proper operation

    SI1145_setParameter(1, 0b10010000); // set CHLIST to EN_UV and EN_ALS_VIS

    SI1145_setRegister(0x13, 0x29); // UCOEF0
    SI1145_setRegister(0x14, 0x89); // UCOEF1
    SI1145_setRegister(0x15, 0x02); // UCOEF2
    SI1145_setRegister(0x16, 0x00); // UCOEF3

    return 0;
}

SI1145_value SI1145_read_sensor() {
    SI1145_value v;
    SI1145_sendCommand(0x06); // ALS_FORCE
    v.vis = SI1145_getRegister16(0x22) / 100; // ALS_VISDATA0
    v.uv = SI1145_getRegister16(0x2c) / 100; // UVINDEX0
    if (v.vis > 100) v.vis = 100;
    return v;
}
