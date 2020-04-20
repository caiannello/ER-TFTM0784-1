#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <Arduino.h>
#include <stdint.h>
#include <avr/pgmspace.h>
/* touch panel interface define */

extern uint8_t SDA_911;
extern uint8_t SCL_911;
extern uint8_t PEN_911; 
extern uint8_t RESET_911;


//Touch Status	 
#define Key_Down 0x01
#define Key_Up   0x00 


typedef struct
{
    uint16_t    x1;
    uint16_t    y1;
    uint16_t    x2;
    uint16_t    y2;
    uint16_t    x3;
    uint16_t    y3;
    uint16_t    x4;
    uint16_t    y4;
    uint16_t    x5;
    uint16_t    y5;

	
    uint8_t     touch_point;
	uint8_t     Key_Sta;	
}  _ts_event;

#define WRITE_ADD	0xba 
#define READ_ADD	0xbb

#define CT_MAX_TOUCH    5		//���ݴ��������֧�ֵĵ���

// Registers define
#define GTP_READ_COOR_ADDR    0x814e
#define GTP_REG_SLEEP         0x8040
#define GTP_REG_SENSOR_ID     0x814a
#define GTP_REG_CONFIG_DATA   0x8047
#define GTP_REG_VERSION       0x8440

//#define CT_READ_XY_REG 	0x814E  	//��ȡ����Ĵ��� 
//#define CT_CONFIG_REG   0x8047	//���ò�����ʼ�Ĵ���

const uint8_t GTP_CFG_DATA[] PROGMEM =
{
/*
0x63,0x00,0x04,0x58,0x02,0x0A,0x3D,0x00,
0x01,0x08,0x28,0x0F,0x50,0x32,0x03,0x05,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x17,
0x19,0x1D,0x14,0x90,0x2F,0x89,0x23,0x25,
0xD3,0x07,0x00,0x00,0x00,0x02,0x03,0x1D,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x19,0x32,0x94,0xD5,0x02,
0x07,0x00,0x00,0x04,0xA2,0x1A,0x00,0x90,
0x1E,0x00,0x80,0x23,0x00,0x73,0x28,0x00,
0x68,0x2E,0x00,0x68,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x16,0x15,0x14,0x11,0x10,0x0F,0x0E,0x0D,
0x0C,0x09,0x08,0x07,0x06,0x05,0x04,0x01,
0x00,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x29,0x28,
0x27,0x26,0x25,0x24,0x23,0x22,0x21,0x20,
0x1F,0x1E,0x1C,0x1B,0x19,0x14,0x13,0x12,
0x11,0x10,0x0F,0x0E,0x0D,0x0C,0x0A,0x08,
0x07,0x06,0x04,0x02,0x00,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x71,0x01	 */

0x5A,0x00,0x05,0x90,0x01,0x05,0x0D,0x00,
0x01,0x08,0x28,0x05,0x50,0x32,0x03,0x05,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x8B,0x2B,0x0A,0x22,0x24,
0x31,0x0D,0x00,0x00,0x00,0x01,0x03,0x2D,
0x00,0x00,0x00,0x00,0x00,0x03,0x64,0x32,
0x00,0x00,0x00,0x1A,0x40,0x94,0xC5,0x02,
0x07,0x00,0x00,0x04,0x95,0x1C,0x00,0x7F,
0x22,0x00,0x71,0x28,0x00,0x62,0x31,0x00,
0x58,0x3A,0x00,0x58,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,
0x12,0x14,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x16,0x18,
0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x24,
0x26,0x13,0x12,0x10,0x0F,0x0C,0x0A,0x08,
0x06,0x04,0x02,0x00,0xFF,0xFF,0xFF,0xFF,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xE3,0x01
};


void TOUCH_Init(void);
void TOUCH_Start(void);
void TOUCH_Stop(void);
uint8_t   TOUCH_Wait_Ack(void);
void TOUCH_Ack(void);
void TOUCH_NAck(void);

void TOUCH_Send_Byte(uint8_t txd);
uint8_t TOUCH_Read_Byte(uint8_t ack);
void Draw_Big_Point(uint16_t x,uint16_t y,uint16_t colour);
uint8_t GT911_read_data(void);
void inttostr(uint16_t value,uint8_t *str);
void GT911_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len);
uint8_t GT911_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len);


#endif
