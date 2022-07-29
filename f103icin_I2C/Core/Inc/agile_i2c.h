/*
 * agile_i2c.h
 *      Author: yahya
 */
#include "stdint.h"

#ifndef INC_AGILE_I2C_H_
#define INC_AGILE_I2C_H_


enum mod_sec{SM_mod, FM_mod};

void i2c_baslat(enum mod_sec secilenMod, int8_t PCLK1);
void i2c_yaz(int8_t slaveAdd, int8_t regAdd, int8_t veri );
void i2c_veriAl(int8_t slaveAdd, int8_t regAdd, int8_t byteSayisi, char *veri);






#endif /* INC_AGILE_I2C_H_ */
