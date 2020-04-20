#include "ER-TFTM0784-1.h"

_ts_event ts_event; 

inline void i2c_pause(void)
{
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
  asm("nop;nop;nop;nop;nop;nop;nop;nop;");
}

//IIC start
void TOUCH_Start(void)
{ 
  digitalWrite(SDA_911, 1);  
  i2c_pause();
  digitalWrite(SCL_911, 1);
  delayMicroseconds(3);
  digitalWrite(SDA_911, 0);  
  delayMicroseconds(3);
  digitalWrite(SCL_911, 0);
//  delayMicroseconds(1);
}   


//IIC stop
void TOUCH_Stop(void)
{
  digitalWrite(SDA_911, 0);
  i2c_pause();
  digitalWrite(SCL_911, 1);
  delayMicroseconds(3);
  digitalWrite(SDA_911, 1);
  delayMicroseconds(3);
  digitalWrite(SCL_911, 0);
//  delayMicroseconds(1);               
}


//Wait for an answer signal
uint8_t TOUCH_Wait_Ack(void)
{ uint8_t errtime=0;

  digitalWrite(SDA_911, 1);
//  delayMicroseconds(1);
  digitalWrite(SCL_911, 1);
//  delayMicroseconds(1);
  while(digitalRead(SDA_911))
  {
    errtime++;
    if(errtime>250)
      {
        TOUCH_Stop();
        return 1;
      } 
     ;
  }
  digitalWrite(SCL_911, 0);
//  delayMicroseconds(1);
  return 0;
}



//Acknowledge
void TOUCH_Ack(void)
{ digitalWrite(SCL_911, 0);
  i2c_pause();
  digitalWrite(SDA_911, 0);
  delayMicroseconds(2);
  digitalWrite(SCL_911, 1);
  delayMicroseconds(2);
  digitalWrite(SCL_911, 0);
  i2c_pause();
}



//NO Acknowledge        
void TOUCH_NAck(void)
{ digitalWrite(SCL_911, 0);
  delayMicroseconds(1);
  digitalWrite(SDA_911, 1);
  delayMicroseconds(2);
  digitalWrite(SCL_911, 1);
  delayMicroseconds(2);
  digitalWrite(SCL_911, 0);
  i2c_pause();
} 
  

//IIC send one byte     
void TOUCH_Send_Byte(uint8_t Byte)
{ uint8_t t;      
  digitalWrite(SCL_911, 0); 
//  delayus(5);
  for(t=0;t<8;t++)
  {           
    digitalWrite(SDA_911,(Byte & 0x80)!=0) ;
    Byte <<=1;
    delayMicroseconds(2);
    digitalWrite(SCL_911, 1);
    delayMicroseconds(2);
    digitalWrite(SCL_911, 0);
  //  delayus(5);
  } 

} 

//Read one byte£¬ack=0£¬Send Acknowledge£¬ack=1£¬NO Acknowledge   
uint8_t TOUCH_Read_Byte(uint8_t ack)
{ uint8_t t,receive=0;

  digitalWrite(SCL_911, 0);
//  delayus(5);
  digitalWrite(SDA_911, 1);
//  delayus(5);
  for(t = 0; t < 8; t++)
  { i2c_pause();
    digitalWrite(SCL_911, 0);
    delayMicroseconds(4);
    digitalWrite(SCL_911, 1);
    delayMicroseconds(4);
    receive<<=1;
    if(digitalRead(SDA_911)) 
      receive=receive|0x01;
  //  delayus(5);
  }

           
  if (ack)  TOUCH_NAck();//NO Acknowledge 
  else       TOUCH_Ack(); //Send Acknowledge   
  
   return receive;
}

uint8_t GT911_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
  uint8_t i;
  uint8_t ret=0;
  TOUCH_Start();  
  TOUCH_Send_Byte(WRITE_ADD);     //·¢ËÍÐ´ÃüÁî   
  TOUCH_Wait_Ack();
  TOUCH_Send_Byte(reg>>8);    //·¢ËÍ¸ß8Î»µØÖ·
  TOUCH_Wait_Ack();                              
  TOUCH_Send_Byte(reg&0xFF);    //·¢ËÍµÍ8Î»µØÖ·
  TOUCH_Wait_Ack(); 
  for(i=0;i<len;i++)
  {    
    TOUCH_Send_Byte(pgm_read_byte_near(buf+i));    //·¢Êý¾Ý
    ret=TOUCH_Wait_Ack();
    if(ret)break;  
  }
  TOUCH_Stop();         //²úÉúÒ»¸öÍ£Ö¹Ìõ¼þ      
  return ret; 
}


void GT911_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
  uint8_t i;
  TOUCH_Start();  
  TOUCH_Send_Byte(WRITE_ADD);     //·¢ËÍÐ´ÃüÁî   
  TOUCH_Wait_Ack(); 
  TOUCH_Send_Byte(reg>>8);    //·¢ËÍ¸ß8Î»µØÖ·
  TOUCH_Wait_Ack();                              
  TOUCH_Send_Byte(reg&0xFF);    //·¢ËÍµÍ8Î»µØÖ·
  TOUCH_Wait_Ack();
  
  TOUCH_Start();       
  TOUCH_Send_Byte(READ_ADD);    //·¢ËÍ¶ÁÃüÁî       
  TOUCH_Wait_Ack();    
  for(i=0;i<len;i++)
  {    
    buf[i]=TOUCH_Read_Byte(i==(len-1)?1:0); //·¢Êý¾Ý
    
    
  //  if(i==(len-1))  buf[i]=TOUCH_Read_Byte(1);
  //  else            buf[i]=TOUCH_Read_Byte(0);   
    
  } 
   TOUCH_Stop();//²úÉúÒ»¸öÍ£Ö¹Ìõ¼þ      
}

uint8_t GT911_Send_Cfg(uint8_t * buf,uint16_t cfg_len)
{
  uint8_t ret=0;
  uint8_t retry=0;
  for(retry=0;retry<5;retry++)
  {
    ret=GT911_WR_Reg(GTP_REG_CONFIG_DATA,buf,cfg_len);
    if(ret==0)break;
    delay(10);   
  }
  return ret;
}

/******************************************************************************************
*Function name£ºDraw_Big_Point(u16 x,u16 y)
* Parameter£ºuint16_t x,uint16_t y xy
* Return Value£ºvoid
* Function£ºDraw touch pen nib point 3 * 3
*********************************************************************************************/       
void Draw_Big_Point(uint16_t x,uint16_t y,uint16_t colour)
{    Graphic_Mode();
   Goto_Pixel_XY(x-1,y-1); 
   LCD_CmdWrite(0x04);  
  LCD_DataWrite(colour);
  LCD_DataWrite(colour);
  LCD_DataWrite(colour);
  Goto_Pixel_XY(x,y-1);//Memory write position
  LCD_CmdWrite(0x04);              
  LCD_DataWrite(colour);
  LCD_DataWrite(colour);
  LCD_DataWrite(colour);
  Goto_Pixel_XY(x+1,y-1);//Memory write position
  LCD_CmdWrite(0x04);              
  LCD_DataWrite(colour);
  LCD_DataWrite(colour);
  LCD_DataWrite(colour);
}

uint8_t GT911_read_data(void)
{ uint8_t buf[80] = {0xff}; uint8_t ret = 0;  uint8_t ss[4];


  GT911_RD_Reg(GTP_READ_COOR_ADDR,buf,80);    //Ò»´Î¶ÁÈ¡80¸ö×Ö½Ú

  ss[0]=0;    
  GT911_WR_Reg(GTP_READ_COOR_ADDR,ss,1);    //Must clear  Buffer status
 
  ts_event.touch_point = buf[0] & 0x0f;


 

  if (ts_event.touch_point == 0) 
    {       

      return 0;
    }   
   

          switch (ts_event.touch_point) 
          {
          /*    case 10:
                  ts_event.x10 = (uint16_t)(buf[75] )<<8 | (uint16_t)buf[74];
                  ts_event.y10 = (uint16_t)(buf[77] )<<8 | (uint16_t)buf[76];

              case 9:
                  ts_event.x9 = (uint16_t)(buf[67] )<<8 | (uint16_t)buf[66];
                  ts_event.y9 = (uint16_t)(buf[69] )<<8 | (uint16_t)buf[68];

              case 8:
                  ts_event.x8 = (uint16_t)(buf[59] )<<8 | (uint16_t)buf[58];
                  ts_event.y8 = (uint16_t)(buf[61] )<<8 | (uint16_t)buf[60];

              case 7:
                  ts_event.x7 = (uint16_t)(buf[51] )<<8 | (uint16_t)buf[50];
                  ts_event.y7 = (uint16_t)(buf[53] )<<8 | (uint16_t)buf[52];

              case 6:
                  ts_event.x6 = (uint16_t)(buf[43] )<<8 | (uint16_t)buf[42];
                  ts_event.y6 = (uint16_t)(buf[45] )<<8 | (uint16_t)buf[44];      */

              case 5:
                  ts_event.y5 = (uint16_t)(buf[35] )<<8 | (uint16_t)buf[34];
                  ts_event.x5 = (uint16_t)(buf[37] )<<8 | (uint16_t)buf[36];
        
              case 4:
                  ts_event.y4 = (uint16_t)(buf[27] )<<8 | (uint16_t)buf[26];
                  ts_event.x4 = (uint16_t)(buf[29] )<<8 | (uint16_t)buf[28];
            
              case 3:
                  ts_event.y3 = (uint16_t)(buf[19] )<<8 | (uint16_t)buf[18];
                  ts_event.x3 = (uint16_t)(buf[21] )<<8 | (uint16_t)buf[20];
            
              case 2:
                  ts_event.y2 = (uint16_t)(buf[11] )<<8 | (uint16_t)buf[10];
                  ts_event.x2 = (uint16_t)(buf[13] )<<8 | (uint16_t)buf[12];
            
              case 1:
                  ts_event.y1 = (uint16_t)(buf[3] )<<8 | (uint16_t)buf[2];
                  ts_event.x1 = (uint16_t)(buf[5] )<<8 | (uint16_t)buf[4];
        
          break;
              default:
              return 0;
          }


     
  return ret;
}


void inttostr(uint16_t value,uint8_t *str)
{
  str[0]=value/1000+48;
  str[1]=value%1000/100+48;
  str[2]=value%1000%100/10+48;
  str[3]=value%1000%100%10+48;
}



void TOUCH_Init(void)
{ 
 
  /*  digitalWrite(RESET_911, 0);
    PEN_911=0;
    Delay1ms(30);
    PEN_911=1;
    Delay1ms(5);
    digitalWrite(RESET_911, 1);;
    Delay1ms(20);
    Delay1ms(200);   */

    pinMode(SDA_911,OUTPUT);
    pinMode(SCL_911,OUTPUT);
    pinMode(PEN_911,OUTPUT);
    pinMode(RESET_911,OUTPUT);

    digitalWrite(RESET_911, 0);               
    digitalWrite(PEN_911, 0);      //CTP INT  SET CTP IIC ADDRESS
    delay(50);
    digitalWrite(RESET_911, 1);
    delay(200);
    //ACC=PEN_INT;        //CTP INT  SET CTP IIC ADDRESS
    //ACC=PEN_INT;       //CTP INT  SET CTP IIC ADDRESS
    delay(200);



     
}             


void TPTEST(void)
{
  uint8_t ss[4]; uint8_t re=1;// uint8_t i;  uint8_t bb[187];
  ts_event.Key_Sta=Key_Up;
    
            ts_event.x1=0;
            ts_event.y1=0;
            ts_event.x2=0;
            ts_event.y2=0;
            ts_event.x3=0;
            ts_event.y3=0;
            ts_event.x4=0;
            ts_event.y4=0;
            ts_event.x5=0;
            ts_event.y5=0;

  //  Font_90_degree();
  //  VSCAN_B_to_T();

    Select_Main_Window_16bpp();
    Main_Image_Start_Address(0);        
    Main_Image_Width(400);              
    Main_Window_Start_XY(0,0);  

    Canvas_Image_Start_address(0);//Layer 1
    Canvas_image_width(400);//
    Active_Window_XY(0,0);
    Active_Window_WH(400,1280);

    Foreground_color_65k(Black);
    Line_Start_XY(0,0);
    Line_End_XY(399,1279);
    Start_Square_Fill();

     Foreground_color_65k(Black);
  Background_color_65k(Blue2);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,450);
  Show_String("www.buydisplay.ocm  CTP test");

  Goto_Text_XY(0,1230);
  Show_String("exit");
  Goto_Text_XY(376,1215);
  Show_String("clear");



  Foreground_color_65k(White);
  Background_color_65k(Black);
  CGROM_Select_Internal_CGROM();
  Font_Select_8x16_16x16();
  

    //    TOUCH_Init();

  re=GT911_Send_Cfg((uint8_t*)GTP_CFG_DATA,sizeof(GTP_CFG_DATA));
  if(re==0)
  {
    Goto_Text_XY(0,0);Show_String("Send CFG OK");
  }
  else  
  {
    Goto_Text_XY(0,0);Show_String("Send CFG NOK");
  }
      
      
 /*    
    GT911_RD_Reg(GTP_REG_CONFIG_DATA,bb,sizeof(GTP_CFG_DATA));

      
    i=0;
      Text_Mode();
     Goto_Text_XY(0,100);
   while(i<186)       
    { inttostr(bb[i],ss);
     LCD_CmdWrite(0x04);
    LCD_DataWrite(ss[0]); delayMicroseconds(2); Check_Mem_WR_FIFO_not_Full(); 
    LCD_DataWrite(ss[1]); delayMicroseconds(2); Check_Mem_WR_FIFO_not_Full();
    LCD_DataWrite(ss[2]); delayMicroseconds(2);  Check_Mem_WR_FIFO_not_Full();
    LCD_DataWrite(ss[3]); delayMicroseconds(2);  Check_Mem_WR_FIFO_not_Full();
    LCD_DataWrite(','); delayMicroseconds(2);   Check_Mem_WR_FIFO_not_Full();
    i+=1;
    }
*/       

    ss[0]=0;    
  GT911_WR_Reg(GTP_READ_COOR_ADDR,ss,1);    //Must clear  Buffer status


  while(digitalRead(NEXT_8876))
  {     Foreground_color_65k(White);
      Background_color_65k(Black);
      CGROM_Select_Internal_CGROM();
      Font_Select_8x16_16x16();
  

      Goto_Text_XY(70,465);
      Show_String("Coordinate");



     GT911_RD_Reg(GTP_READ_COOR_ADDR,ss,1);
     if(ss[0]&0x80)
      {
          GT911_read_data();
          ts_event.Key_Sta=Key_Up;

          if(ts_event.x1<=40&&ts_event.x1>5&&ts_event.y1<=1280&&ts_event.y1>1230)return;
          if(ts_event.x1>=365&&ts_event.y1>=1215)
          { 
            Select_Main_Window_16bpp();
            Main_Image_Start_Address(0);        
            Main_Image_Width(400);              
            Main_Window_Start_XY(0,0);  
        
            Canvas_Image_Start_address(0);//Layer 1
            Canvas_image_width(400);//
            Active_Window_XY(0,0);
            Active_Window_WH(400,1280);
        
            Foreground_color_65k(Black);
            Line_Start_XY(0,0);
            Line_End_XY(399,1279);
            Start_Square_Fill();
          
            Foreground_color_65k(Black);
            Background_color_65k(Blue2);
            CGROM_Select_Internal_CGROM();
            Font_Select_12x24_24x24();
            Goto_Text_XY(0,450);
            Show_String("www.buydisplay.ocm  CTP test");
          
            Goto_Text_XY(0,1230);
            Show_String("exit");
            Goto_Text_XY(376,1215);
            Show_String("clear");
          }                      

           else{
    
          inttostr(ts_event.x1,ss); 
      
           
          Goto_Text_XY(100,465);   //Set the display position
          
          Show_String("X=");
          Text_Mode();   
          LCD_CmdWrite(0x04);
          LCD_DataWrite(ss[0]);
           Check_Mem_WR_FIFO_not_Full();
          LCD_DataWrite(ss[1]);
           Check_Mem_WR_FIFO_not_Full();
          LCD_DataWrite(ss[2]);
           Check_Mem_WR_FIFO_not_Full();
          LCD_DataWrite(ss[3]);
           Check_Mem_WR_FIFO_not_Full();

          inttostr(ts_event.y1,ss);
          Goto_Text_XY(130, 465);   //Set the display position
          
          Show_String("Y=");
          Text_Mode();     
          LCD_CmdWrite(0x04);
          LCD_DataWrite(ss[0]);
           Check_Mem_WR_FIFO_not_Full();
          LCD_DataWrite(ss[1]);
           Check_Mem_WR_FIFO_not_Full();
          LCD_DataWrite(ss[2]);
           Check_Mem_WR_FIFO_not_Full();
          LCD_DataWrite(ss[3]);
            Check_Mem_WR_FIFO_not_Full();



          Draw_Big_Point(ts_event.x1,ts_event.y1,color65k_red);
          Draw_Big_Point(ts_event.x2,ts_event.y2,color65k_green); 
          Draw_Big_Point(ts_event.x3,ts_event.y3,color65k_blue);
          Draw_Big_Point(ts_event.x4,ts_event.y4,color65k_cyan);  
          Draw_Big_Point(ts_event.x5,ts_event.y5,color65k_purple);




          }
          
            ts_event.x1=0;
            ts_event.y1=0;
            ts_event.x2=0;
            ts_event.y2=0;
            ts_event.x3=0;
            ts_event.y3=0;
            ts_event.x4=0;
            ts_event.y4=0;
            ts_event.x5=0;
            ts_event.y5=0;

           
      }


  }

   //   Font_0_degree();
  //  VSCAN_T_to_B();

}
