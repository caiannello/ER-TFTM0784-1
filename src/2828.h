#ifndef __2828_H__
#define __2828_H__

#include <Arduino.h>
#include <stdint.h>

extern uint8_t CS_2828;
extern uint8_t RST_2828;
extern uint8_t SDI_2828;
extern uint8_t SCLK_2828;

void SPI_3W_SET_Cmd(uint16_t Sdata);
void SPI_3W_SET_PAs(uint16_t Sdata);
uint8_t SPI_ReadData(void);
void SPI_WriteData(uint8_t value);
void SPI_WriteCmd(uint8_t value);
void GP_COMMAD_PA(uint16_t num);

void SSD2828_Reset(void);
void  SSD2828_initial(void);

#endif