///////////////////////////////////////////////////////////////////////////////
//
// ER-TFTM0784-1  7.84 INCH TFT LCD  1280*400  ( RA8876 + SSD2828 + GT911 )
//
// 2020-04-14 : Ported to Arduino by Craig Iannello 
//              Original by Javen Liu (Original heading next section)
//
// The demo code is pretty much verbatim of Javen's original.
//
// Devices accessed and pin usage:
//
//    CHIP    PURPOSE           INTERFACE
//    RA8876  Graphics Driver   SPI 4-Wire OR 8/16 bit Parallel
//    GT911   Touch             I2C + Interrupt flag output
//    SSD2828 MIPI bridge       SPI 4-Wire
//
// Note the two seperate SPI's.  I read that the RA8876 doesn't properly
// tri-state (release) the bus while it's unselected. I was too lazy to test
// that, and just left the SSD2828 as software serial on some pins,
// and hooked the HW SPI up to the RA8876. The 2828 only needs to be spoken 
// to on powerup and doesnt need to be fast.
//
// The touch panel code has been left as bit-bang too out of laziness.
//
///////////////////////////////////////////////////////////////////////////////
//
// EASTRISING TECHNOLOGY CO,.LTD.
//
// Module    : ER-TFTM0784-1  7.84 INCH TFT LCD  1280*400
// Lanuage   : C++
// Create    : JAVEN LIU
// Date      : 2018-12-23
// Drive IC  : RA8876     FLASH:W25Q128FV  128M BIT   FONT CHIP: 30L24T3Y   CTP:GT911
// INTERFACE : 16BIT 8080      CTP:IIC    SSD2828: 3SPI 
// MCU     : STC12LE5C60S2     1T MCU    
// MCU VDD   : 3.3V
// MODULE VDD : 5V OR 3.3V 
//
///////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>
#include <ER-TFTM0784-1.h>

#include "demo.h"

//-----------------------------------------------------------------------------
// the following global variables are declared as externs in the library and
// are defined below.  Sorry, this is probably poor style.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Setup communications method for RA8876 display controller
//-----------------------------------------------------------------------------

uint8_t mode_8876 = 2;  // 0: Parallel 16 bits 8080
                        // 1: Parallel 8 bits 8080
                        // 2: SPI 4-Wire

//-----------------------------------------------------------------------------
// pins for display controller used in all modes
//-----------------------------------------------------------------------------

uint8_t RST_8876 = 34;

//-----------------------------------------------------------------------------
// pins for display controller only when in 4-wire SPI mode (mode 2)
//-----------------------------------------------------------------------------

uint32_t SPI_SPEED_8876 = 42000000;

uint8_t CS_SER_8876 = 10;
uint8_t SDI_8876 = 109;
uint8_t SDO_8876 = 108;
uint8_t SCLK_8876 = 110;

//-----------------------------------------------------------------------------
// pins for display controller only when in 8/16-Bit parallel mode (modes 0,1)
//
// (all of these are unused when in mode 2 but must be declared anyway)
//
// The parallel code in the Library is very much degraded from Javen's
// original because I don't know a portable way of using IO lines in parallel
// in Arduino, e.g.
//
// PORTA = 0xff
// foo = PORTB;
//
// I'm ashamedly doing it bit-wise. 
//
//-----------------------------------------------------------------------------

uint8_t RS_8876 = 33;
uint8_t CS_PAR_8876 = 34;
uint8_t WR_8876 = 35;
uint8_t RD_8876 = 36;
uint8_t NEXT_8876 = 37;
uint8_t D0 = 38;
uint8_t D1 = 39;
uint8_t D2 = 40;
uint8_t D3 = 41;
uint8_t D4 = 42;
uint8_t D5 = 43;
uint8_t D6 = 44;
uint8_t D7 = 45;
uint8_t D8 = 46;
uint8_t D9 = 47;
uint8_t D10 = 48;
uint8_t D11 = 49;
uint8_t D12 = 50;
uint8_t D13 = 51;
uint8_t D14 = 52;
uint8_t D15 = 53;

//-----------------------------------------------------------------------------
// SPI pins for the 2828 LCD panel driver (used in all modes)
//
// No attempt was made to share this SPI with the RA8876 SPI because I read
// that there are problems with the RA8876 not properly 
// tri-stating (disconnecting) its bus pins when it is not selected.
//
// Note that this interface is only used once on powerup to setup the panel
// driver, so it doesnt need to be fast. It uses software (bit bang) SPI.
//-----------------------------------------------------------------------------

uint8_t CS_2828 = 24;
uint8_t RST_2828 = 25;
uint8_t SDI_2828 = 26;
uint8_t SCLK_2828 = 27;

//-----------------------------------------------------------------------------
// I2C pins for GT911 touch controller (used in all modes)
//
// This one is also implemented as bit-bang. Thanks, Javen!
//-----------------------------------------------------------------------------

uint8_t SDA_911       =  28;
uint8_t SCL_911       =  29;
uint8_t PEN_911       =  30; 
uint8_t RESET_911     =  31;

//-----------------------------------------------------------------------------
// The demo code is pretty much verbatim of Javen's original.
//-----------------------------------------------------------------------------
void setup(void)
{
  Serial.begin(9600);
  Serial.println("Starting in one second...");
  delay(1000);
  Serial.println("Starting.");
  Serial.println("GT911   touch init");  
  TOUCH_Init();
  Serial.println("SSD2828 reset");  
  SSD2828_Reset();       
  Serial.println("RA8876  IO init");
  RA8876_IO_Init(); 
  Serial.println("RA8876  HW reset");
  RA8876_HW_Reset();
  Serial.println("SSD2828 init");
  SSD2828_initial();
  Serial.println("RA8876  init");
  RA8876_initial();
  Serial.println("Display ON");  
  Display_ON();
  /*   Select_Main_Window_16bpp();
  Main_Image_Start_Address(layer1_start_addr);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);
  Foreground_color_65k(White);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();   */
}
//-----------------------------------------------------------------------------
void loop(void)
{  
  
  // BackLight brightness control : RA8876's PWM0
  Serial.println("RA8876  init backlight pwm");  
  Enable_PWM0_Interrupt();
  Clear_PWM0_Interrupt_Flag();
  Mask_PWM0_Interrupt_Flag();
  Select_PWM0_Clock_Divided_By_2();
  Select_PWM0();
  Enable_PWM0_Dead_Zone();
  Auto_Reload_PWM0();
  Start_PWM0();
  Set_Timer0_Compare_Buffer(0xffff);
  
  //Serial.println("TpTest");  
  //TPTEST();
  Serial.println("Geometric");  
  Geometric();
  Serial.println("Text");  
  Text_Demo();
  Serial.println("Mono");    
  mono_Demo();     
  Serial.println("Gray"); 
  gray();
  Serial.println("DMA");      
  DMA_Demo();
  Serial.println("Pattern fill");
  BTE_Pattern_Fill();
  Serial.println("Color expansion");
  BTE_Color_Expansion();
  Serial.println("Waveform");
  App_Demo_Waveform();
  Serial.println("Scroller");  
  App_Demo_Scrolling_Text();
  Serial.println("Slide frame buffer");        
  App_Demo_slide_frame_buffer();
  Serial.println("Multi frame buffer");        
  App_Demo_multi_frame_buffer();
}
//-----------------------------------------------------------------------------
void NextStep(void)
{ 
  delay(500);
  /*  
  while(next)
    { 
      Delay100ms(1);
    }
  Delay100ms(10);
  */
}
//-----------------------------------------------------------------------------
void BTE_Color_Expansion(void)
{    
  uint16_t i,j;
    uint32_t im=1;

  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

/*  Canvas_Image_Start_address(layer1_start_addr);//Layer 1
  Canvas_image_width(400);//
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);

  Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();



    //DMA initial setting
  Enable_SFlash_SPI();
    Select_SFI_1();
    Select_SFI_DMA_Mode();
    Select_SFI_24bit_Address();

    //Select_SFI_Waveform_Mode_0();
    Select_SFI_Waveform_Mode_3();

    //Select_SFI_0_DummyRead(); //normal read mode
    Select_SFI_8_DummyRead(); //1byte dummy cycle
    //Select_SFI_16_DummyRead();
    //Select_SFI_24_DummyRead();

    Select_SFI_Single_Mode();
    //Select_SFI_Dual_Mode0();
    //Select_SFI_Dual_Mode1();

    SPI_Clock_Period(0);


  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//

  SFI_DMA_Source_Start_Address(im*400*1280*3);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();  */
                 


  //color expansion and bte memory copy(move) 

  j=0;
 do
 {
   for(i=0;i<3;i++)
   {
   //switch to layer2 update screen, and execute color expansion and copy to layer1
   Canvas_Image_Start_address(layer2_start_addr);//
   Foreground_color_65k(Black);
   Line_Start_XY(0,0);
   Line_End_XY(399,1279);
   Start_Square_Fill();
   SFI_DMA_Source_Start_Address(0);//
   Start_SFI_DMA();
     Check_Busy_SFI_DMA();

   BTE_S0_Color_16bpp();
 
     BTE_S1_Color_16bpp();

     BTE_Destination_Color_16bpp();  
     BTE_Destination_Memory_Start_Address(layer2_start_addr);
     BTE_Destination_Image_Width(400);
     BTE_Destination_Window_Start_XY(0+70,0+70);  
     BTE_Window_Size(160,160);
   Foreground_color_65k(color65k_blue);
     Background_color_65k(color65k_red);
   BTE_ROP_Code(15);
     BTE_Operation_Code(8); //BTE color expansion

   BTE_Enable();
   LCD_CmdWrite(0x04);

   switch(i)
   {
    case 0 :               
        Show_picture(10*160,f1); 
        Check_Mem_WR_FIFO_Empty();      
            Check_BTE_Busy();
        break;
    case 1 :
        Show_picture(10*160,f2); 
        Check_Mem_WR_FIFO_Empty();      
            Check_BTE_Busy();
        break;
    case 2 :
        Show_picture(10*160,f3); 
        Check_Mem_WR_FIFO_Empty();      
            Check_BTE_Busy();
        break;
     default:
        break;
   }

  
    Foreground_color_65k(White);
  Background_color_65k(Black);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(274,70);
  Show_String("Color Expansion");



   Foreground_color_65k(color65k_blue);
     Background_color_65k(color65k_red);

   BTE_Operation_Code(9); //BTE color expansion with chroma key
   BTE_Destination_Window_Start_XY(70,460);  
   BTE_Enable();
   LCD_CmdWrite(0x04);

   switch(i)
   {
    case 0 :              
        Show_picture(10*160,f1); 
        Check_Mem_WR_FIFO_Empty();      
            Check_BTE_Busy();
        break;
    case 1 :
        Show_picture(10*160,f2); 
        Check_Mem_WR_FIFO_Empty();      
            Check_BTE_Busy();
        break;
    case 2 :
        Show_picture(10*160,f3); 
        Check_Mem_WR_FIFO_Empty();      
            Check_BTE_Busy();
        break;
     default:
        break;
   }
   

  Foreground_color_65k(White);
  Background_color_65k(Black);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(250,450);
  Show_String("Color Expansion");
  Goto_Text_XY(274,450);
  Show_String("Color With chroma key");

     Foreground_color_65k(color65k_blue);
     Background_color_65k(color65k_red);

     //BTE memory(move) layer2 to layer1
     //BTE_S0_Color_16bpp();
       BTE_S0_Memory_Start_Address(layer2_start_addr);
       BTE_S0_Image_Width(400);
       BTE_S0_Window_Start_XY(0,0);

       //BTE_Destination_Color_16bpp();  
       BTE_Destination_Memory_Start_Address(layer1_start_addr);
       BTE_Destination_Image_Width(400);
       BTE_Destination_Window_Start_XY(0,0);  
       BTE_Window_Size(400,1280);

       BTE_ROP_Code(12); 
       BTE_Operation_Code(2); //BTE move
       BTE_Enable();      //memory copy s0(layer2) to layer1  
       Check_BTE_Busy();

   
    j++;
  }

   }while(j<8);

      NextStep();

}
//-----------------------------------------------------------------------------
void BTE_Pattern_Fill(void)
{     uint32_t im=1;

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



  Foreground_color_65k(White);
  Background_color_65k(Blue2);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,800);
  Show_String("Demo BTE Pattern Fill");


    //DMA initial setting
  Enable_SFlash_SPI();
    Select_SFI_1();
    Select_SFI_DMA_Mode();
    Select_SFI_24bit_Address();

    //Select_SFI_Waveform_Mode_0();
    Select_SFI_Waveform_Mode_3();

    //Select_SFI_0_DummyRead(); //normal read mode
    Select_SFI_8_DummyRead(); //1byte dummy cycle
    //Select_SFI_16_DummyRead();
    //Select_SFI_24_DummyRead();

    Select_SFI_Single_Mode();
    //Select_SFI_Dual_Mode0();
    //Select_SFI_Dual_Mode1();

    SPI_Clock_Period(0);


  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//

  SFI_DMA_Source_Start_Address(im*400*1280*2*1);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();


  //write 16x16 pattern to sdram
  Pattern_Format_16X16(); 
  Canvas_Image_Start_address(layer6_start_addr);//any layer 
    Canvas_image_width(16);
    Active_Window_XY(0,0);
    Active_Window_WH(16,16);
    Goto_Pixel_XY(0,0);
    Show_picture(16*16,pattern16x16_16bpp); 

  Canvas_Image_Start_address(layer1_start_addr);//
    Canvas_image_width(400);
    Active_Window_XY(0,0);
    Active_Window_WH(400,1280);

    
  BTE_S0_Color_16bpp();
    BTE_S0_Memory_Start_Address(layer6_start_addr);
    BTE_S0_Image_Width(16);
 
    BTE_S1_Color_16bpp();
    BTE_S1_Memory_Start_Address(0);
    BTE_S1_Image_Width(400);

    BTE_Destination_Color_16bpp();  
    BTE_Destination_Memory_Start_Address(layer1_start_addr);
    BTE_Destination_Image_Width(400);

    BTE_ROP_Code(0xc);
    BTE_Operation_Code(0x06);//pattern fill 

  BTE_S1_Window_Start_XY(0,0);      
    BTE_Destination_Window_Start_XY(0,40);
    BTE_Window_Size(300,300);

    BTE_Enable();   
    Check_BTE_Busy();
  
  Foreground_color_65k(Black);
  Background_color_65k(White);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(330,100);
  Show_String("Pattern Fill");
     
  Background_color_65k(color65k_red);
  BTE_S1_Window_Start_XY(0,0);      
    BTE_Destination_Window_Start_XY(0,440);
    BTE_Window_Size(300,300);
  BTE_Operation_Code(0x07);//pattern fill with chroma key

  BTE_Enable();   
    Check_BTE_Busy();

  Foreground_color_65k(Black);
  Background_color_65k(White);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(330,450);
  Show_String("Pattern Fill With");

  Foreground_color_65k(Black);
  Background_color_65k(White);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(354,450);
  Show_String("Chroma Key");
    delay(1000);
    NextStep();
}
//-----------------------------------------------------------------------------
void App_Demo_Waveform(void)
{
    uint16_t i,h=0;

  uint16_t point1y,point2y;
  uint16_t point21y,point22y;
  uint16_t point31y,point32y;
  point2y = 0; //initial value
  point22y = 0; //initial value
  point32y = 0; //initial value

  #define grid_width 400
  #define grid_high  1280
  #define grid_gap 50
    
  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

  Canvas_Image_Start_address(0);//Layer 1
  Canvas_image_width(400);//
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);

  Foreground_color_65k(Blue);
  Line_Start_XY(0,0);
  Line_End_XY(399,1247);
  Start_Square_Fill();

  Foreground_color_65k(Blue2);
  Line_Start_XY(0,1248);
  Line_End_XY(399,1279);
  Start_Square_Fill();

do{     

  Canvas_Image_Start_address(layer2_start_addr);//Layer 2


    Foreground_color_65k(Black);  //clear layer2 to color black
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();




    for(i=0;i<=grid_width;i+=grid_gap)
  {
   Foreground_color_65k(color65k_grayscale12);
   Line_Start_XY(i,0);
     Line_End_XY(i,grid_high-1);
   Start_Line();
  }

   for(i=0;i<=grid_high;i+=grid_gap)
  {
   Foreground_color_65k(color65k_grayscale12);
   Line_Start_XY(0,i);
     Line_End_XY(grid_width-1,i);
   Start_Line();
  }

  Foreground_color_65k(Red);
  Background_color_65k(Black);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,0);
  Show_String("Application Demo Waveform    www.buydisplay.com");


  //BTE memory(move) grid to layer1
     BTE_S0_Color_16bpp();
       BTE_S0_Memory_Start_Address(layer2_start_addr);
       BTE_S0_Image_Width(400);
       BTE_S0_Window_Start_XY(0,0);


       BTE_Destination_Color_16bpp();  
       BTE_Destination_Memory_Start_Address(0);
       BTE_Destination_Image_Width(400);
       BTE_Destination_Window_Start_XY(0,0);  
       BTE_Window_Size(400,1280);

     //move with ROP 0 
       BTE_ROP_Code(12); //memory copy s0(layer2)grid to layer1  
       BTE_Operation_Code(2); //BTE move
       BTE_Enable();
       Check_BTE_Busy();


     Canvas_Image_Start_address(0);//Layer 1
       


      for(i=0;i<1280;i+=2)
     {
    // copy layer2 grid column to layer1
     BTE_S0_Window_Start_XY(i,0);

     BTE_Destination_Window_Start_XY(0+i,0);  
         BTE_Window_Size(0,400);
     BTE_Enable();
         Check_BTE_Busy();

  
     point1y = point2y;
         point2y = rand()%90;//
    
     point21y = point22y;
         point22y = rand()%99;//
//    
     point31y = point32y;
         point32y = rand()%67;//
     
     Foreground_color_65k(color65k_yellow);//
//     Line_Start_XY(i+10,point1y+80);
//     Line_End_XY(i+1+10,point2y+80);
     Line_Start_XY(point1y+80,i+10);
     Line_End_XY(point2y+80,i+1+10);
       Start_Line();

     Foreground_color_65k(color65k_purple);//
//     Line_Start_XY(i+10,point21y+200);
//     Line_End_XY(i+1+10,point22y+200);
     Line_Start_XY(point21y+200,i+10);
     Line_End_XY(point22y+200,i+1+10);
       Start_Line();
//
     Foreground_color_65k(color65k_green);//
//     Line_Start_XY(i+10,point31y+300);
//     Line_End_XY(i+1+10,point32y+300);

     Line_Start_XY(point31y+300,i+10);
     Line_End_XY(point32y+300,i+1+10);
       Start_Line();


     //delay(1000); 
      }
     

     h++;
    }
   while(h<7);  

    NextStep();

}
//-----------------------------------------------------------------------------
void App_Demo_Scrolling_Text(void)
{
    uint16_t i; 

 

  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

  Canvas_Image_Start_address(layer2_start_addr);
  Canvas_image_width(400);//
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);

  Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();


/*
  Foreground_color_65k(White);
  Background_color_65k(Blue2);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,Line52);
  Show_String("Demo Scrolling Text");
  Foreground_color_65k(White);
  Background_color_65k(Black);
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,Line51);
  Show_String("Used Move BTE with Chroma Key ");
   */

    //DMA initial setting
  Enable_SFlash_SPI();
    Select_SFI_1();
    Select_SFI_DMA_Mode();
    Select_SFI_24bit_Address();

    //Select_SFI_Waveform_Mode_0();
    Select_SFI_Waveform_Mode_3();

    //Select_SFI_0_DummyRead(); //normal read mode
    Select_SFI_8_DummyRead(); //1byte dummy cycle
    //Select_SFI_16_DummyRead();
    //Select_SFI_24_DummyRead();

    Select_SFI_Single_Mode();
    //Select_SFI_Dual_Mode0();
    //Select_SFI_Dual_Mode1();

    SPI_Clock_Period(0);


  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//
  SFI_DMA_Source_Start_Address(0);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();


  //BTE memory(move) layer2 to layer1
     BTE_S0_Color_16bpp();
       BTE_S0_Memory_Start_Address(layer2_start_addr);
       BTE_S0_Image_Width(400);
       BTE_S0_Window_Start_XY(0,0);



       BTE_Destination_Color_16bpp();  
       BTE_Destination_Memory_Start_Address(layer1_start_addr);
       BTE_Destination_Image_Width(400);
       BTE_Destination_Window_Start_XY(0,0);  
       BTE_Window_Size(400,1280);

       BTE_ROP_Code(12); 
       BTE_Operation_Code(2); //BTE move
       BTE_Enable();      //memory copy s0(layer3) to layer1  
       Check_BTE_Busy();


    //write text to layer3
    Canvas_Image_Start_address(layer3_start_addr);
    Canvas_image_width(400);//
      Active_Window_XY(0,0);
    Active_Window_WH(400,1280);

    Foreground_color_65k(Red);
    Line_Start_XY(0,0);
    Line_End_XY(399,1279);
    Start_Square_Fill();

  Foreground_color_65k(Green);
  Background_color_65k(Red);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(24,10);
  Show_String("Demo Scrolling Text");

  Foreground_color_65k(Yellow);
  Background_color_65k(Red);
  Goto_Text_XY(48,10);
  Show_String("Demo Scrolling Text");
  Foreground_color_65k(Magenta);
  Background_color_65k(Red);
  Goto_Text_XY(72,10);
  Show_String("Demo Scrolling Text");
  Foreground_color_65k(Grey);
  Background_color_65k(Red);
  Goto_Text_XY(96,10);
  Show_String("Demo Scrolling Text");


    //Move BTE with chroma key layer3 to layer2 then move layer2 to layer1 to display

    for(i=0;i<200;i+=8)
    {      
      Canvas_Image_Start_address(layer2_start_addr);
      Canvas_image_width(400);//
        Active_Window_XY(0,0);
      Active_Window_WH(400,1280);

       Foreground_color_65k(Black);
       Line_Start_XY(0,0);
       Line_End_XY(399,1279);
       Start_Square_Fill();

     SFI_DMA_Destination_Upper_Left_Corner(0,0);
         SFI_DMA_Transfer_Width_Height(400,1280);
         SFI_DMA_Source_Width(400);//
       SFI_DMA_Source_Start_Address(0);//
       Start_SFI_DMA();
         Check_Busy_SFI_DMA();

    //BTE memory(move) layer3 to layer2
     //BTE_S0_Color_16bpp();
       BTE_S0_Memory_Start_Address(layer3_start_addr);
       BTE_S0_Image_Width(400);
       BTE_S0_Window_Start_XY(0,0);



       //BTE_Destination_Color_16bpp();  
       BTE_Destination_Memory_Start_Address(layer2_start_addr);
       BTE_Destination_Image_Width(400);
       BTE_Destination_Window_Start_XY(391-i,72);  
       BTE_Window_Size(0+i,24*4);

     Background_color_65k(Red);//

       BTE_ROP_Code(12); 
       BTE_Operation_Code(5); //BTE move with chroma key
       BTE_Enable();      //memory copy s0(layer3) to layer1  
       Check_BTE_Busy();




      //BTE memory(move) layer2 to layer1
     //BTE_S0_Color_16bpp();
       BTE_S0_Memory_Start_Address(layer2_start_addr);
       BTE_S0_Image_Width(400);
       BTE_S0_Window_Start_XY(0,0);

       //BTE_Destination_Color_16bpp();  
       BTE_Destination_Memory_Start_Address(layer1_start_addr);
       BTE_Destination_Image_Width(400);
       BTE_Destination_Window_Start_XY(0,0);  
       BTE_Window_Size(400,1280);

       BTE_ROP_Code(12); 
       BTE_Operation_Code(2); //BTE move
       BTE_Enable();      //memory copy s0(layer2) to layer1  
       Check_BTE_Busy();
    }
      
    for(i=0;i<100;i+=8)
    {      
      Canvas_Image_Start_address(layer2_start_addr);
      Canvas_image_width(400);//
        Active_Window_XY(0,0);
      Active_Window_WH(400,1280);

       Foreground_color_65k(Black);
       Line_Start_XY(0,0);
       Line_End_XY(399,1279);
       Start_Square_Fill();

     SFI_DMA_Destination_Upper_Left_Corner(0,0);
         SFI_DMA_Transfer_Width_Height(400,1280);
         SFI_DMA_Source_Width(400);//
       SFI_DMA_Source_Start_Address(layer4_start_addr);// ??
       Start_SFI_DMA();
         Check_Busy_SFI_DMA();

    //BTE memory(move) layer3 to layer2
     //BTE_S0_Color_16bpp();
       BTE_S0_Memory_Start_Address(layer3_start_addr);
       BTE_S0_Image_Width(400);
       BTE_S0_Window_Start_XY(i,0);



       //BTE_Destination_Color_16bpp();  
       BTE_Destination_Memory_Start_Address(layer2_start_addr);
       BTE_Destination_Image_Width(400);
       BTE_Destination_Window_Start_XY(0,72);  
       BTE_Window_Size(399-i,24*4);

     Background_color_65k(Red);//

       BTE_ROP_Code(12); 
       BTE_Operation_Code(5); //BTE move with chroma key
       BTE_Enable();      //memory copy s0(layer3) to layer1  
       Check_BTE_Busy();

//     Main_Image_Width(800);             
//       Main_Window_Start_XY(0,0);
//     Main_Image_Start_Address(layer2_start_addr);//switch display windows to      
//     delay(20);


      //BTE memory(move) layer2 to layer1
     //BTE_S0_Color_16bpp();
       BTE_S0_Memory_Start_Address(layer2_start_addr);
       BTE_S0_Image_Width(400);
       BTE_S0_Window_Start_XY(0,0);

       //BTE_Destination_Color_16bpp();  
       BTE_Destination_Memory_Start_Address(layer1_start_addr);
       BTE_Destination_Image_Width(400);
       BTE_Destination_Window_Start_XY(0,0);  
       BTE_Window_Size(400,1280);

       BTE_ROP_Code(12); 
       BTE_Operation_Code(2); //BTE move
       BTE_Enable();      //memory copy s0(layer2) to layer1  
       Check_BTE_Busy();
    }

      NextStep();

}
//-----------------------------------------------------------------------------
void App_Demo_multi_frame_buffer(void)
{    uint16_t i,j;
      uint32_t im=1;
    Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

      //DMA initial setting
  Enable_SFlash_SPI();
    Select_SFI_1();
    Select_SFI_DMA_Mode();
    Select_SFI_24bit_Address();

    //Select_SFI_Waveform_Mode_0();
    Select_SFI_Waveform_Mode_3();

    //Select_SFI_0_DummyRead(); //normal read mode
    Select_SFI_8_DummyRead(); //1byte dummy cycle
    //Select_SFI_16_DummyRead();
    //Select_SFI_24_DummyRead();

    Select_SFI_Single_Mode();
    //Select_SFI_Dual_Mode0();
    //Select_SFI_Dual_Mode1();

    SPI_Clock_Period(0);

  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//


 for(i=0;i<7;i++)
  {

  Canvas_Image_Start_address(im*400*1280*2*i);//Layer1~6
  Canvas_image_width(400);//
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);

 /* Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1247);
  Start_Square_Fill();

  Foreground_color_65k(Blue2);
  Line_Start_XY(0,1248);
  Line_End_XY(399,1279);
  Start_Square_Fill();


  Foreground_color_65k(White);
  Background_color_65k(Blue2);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,Line52);
  Show_String("Demo Multi Frame Buffer");

  //LCD_DisplayString(0,Line23 ,"  Demo Mulit Frame Buffer",Black,White);

   */
  

    SFI_DMA_Source_Start_Address(im*400*1280*2*i);//
    Start_SFI_DMA();
      Check_Busy_SFI_DMA();
    //delay(500);
    delay(100);
    Main_Image_Width(400);              
      Main_Window_Start_XY(0,0);
    Main_Image_Start_Address(im*400*1280*2*i);//switch display windows to 
   }
  //delay(2000);
//  Color_Bar_ON();
    
  //set canvas to 8188 
    Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(3200);             
  Main_Window_Start_XY(0,0);  
  Canvas_Image_Start_address(0);
  Canvas_image_width(3200);//
  Active_Window_XY(0,0);
  Active_Window_WH(3200,1280);
  
  for(im=0;im<5;im++)
    {
  Canvas_Image_Start_address(0);//Layer1~6

  Foreground_color_65k(Black);
  Line_Start_XY(0+im*400,0);
  Line_End_XY(399+im*400,1255);
  Start_Square_Fill();

  Foreground_color_65k(Blue2);
  Line_Start_XY(0+im*400,1256);
  Line_End_XY(399+im*400,1279);
  Start_Square_Fill();


    SFI_DMA_Destination_Upper_Left_Corner(0+im*400,0);
  SFI_DMA_Source_Start_Address(im*1280*400*2);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();
     delay(5);

/*  Foreground_color_65k(White);
  Background_color_65k(Blue2);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(i*1024,Line52);
  Show_String("Demo Mulit Frame Buffer"); 
   */
  }
//  Color_Bar_OFF();

   for(j=0;j<4;j++)
   {
  for(i=0;i<400;i++)
  {
   Main_Window_Start_XY(i+j*400,0);
   delay(5);
  }
    delay(30);  
   }

   for(j=4;j>0;j--)
   {
  for(i=0;i<400;i++)
  {
   Main_Window_Start_XY(j*400-i,0);
   delay(5);
  }

   }

   delay(500);

        NextStep();
}
//-----------------------------------------------------------------------------
void App_Demo_slide_frame_buffer(void)
{   uint32_t im=1;
     uint16_t i,j;
    
    Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

      //DMA initial setting
  Enable_SFlash_SPI();
    Select_SFI_1();
    Select_SFI_DMA_Mode();
    Select_SFI_24bit_Address();

    //Select_SFI_Waveform_Mode_0();
    Select_SFI_Waveform_Mode_3();

    //Select_SFI_0_DummyRead(); //normal read mode
    Select_SFI_8_DummyRead(); //1byte dummy cycle
    //Select_SFI_16_DummyRead();
    //Select_SFI_24_DummyRead();

    Select_SFI_Single_Mode();
    //Select_SFI_Dual_Mode0();
    //Select_SFI_Dual_Mode1();

    SPI_Clock_Period(0);

  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//


  //Color_Bar_ON();
    
  //set canvas to 1600
    Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(800);              
  Main_Window_Start_XY(0,0);  
  Canvas_Image_Start_address(0);
  Canvas_image_width(800);//
  Active_Window_XY(0,0);
  Active_Window_WH(800,2560);
  
  Canvas_Image_Start_address(0);//

  Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(799,2559);
  Start_Square_Fill();

    SFI_DMA_Destination_Upper_Left_Corner(0,0);
  SFI_DMA_Source_Start_Address(0);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();
 
    SFI_DMA_Destination_Upper_Left_Corner(400,0);
  SFI_DMA_Source_Start_Address(im*400*1280*2);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();

  SFI_DMA_Destination_Upper_Left_Corner(0,1280);
  SFI_DMA_Source_Start_Address(im*400*1280*4);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();

  SFI_DMA_Destination_Upper_Left_Corner(400,1280);
  SFI_DMA_Source_Start_Address(im*400*1280*6);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();


  
  //Color_Bar_OFF();


  for(i=0;i<800-400+1;i++)
  {
   Main_Window_Start_XY(i,0);
   delay(5);
  }
    delay(1); 
 
  for(j=0;j<2560-1280+1;j++)
  {
   Main_Window_Start_XY(400,j);
   delay(5);
  } 
  delay(1);


  for(i=800-400;i>0;i--)
  {
   Main_Window_Start_XY(i,1280);
   delay(5);
  }
  Main_Window_Start_XY(0,1280);
  delay(1); 
 
  for(j=2560-1280;j>0;j--)
  {
   Main_Window_Start_XY(0,j);
   delay(5);
  }
  Main_Window_Start_XY(0,0);
  delay(5); 


    NextStep();
}
//-----------------------------------------------------------------------------
void App_Demo_Alpha_Blending(void)
{    uint32_t im=1;
      uint16_t i,j;

    Select_Main_Window_16bpp();
  Main_Image_Start_Address(layer1_start_addr);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

  Canvas_Image_Start_address(layer1_start_addr);//
  Main_Image_Width(400);  
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);

 
  Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1247);
  Start_Square_Fill();

  Foreground_color_65k(Blue2);
  Line_Start_XY(0,1248);
  Line_End_XY(399,1279);
  Start_Square_Fill();



  Foreground_color_65k(White);
  Background_color_65k(Blue2);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,Line52);
  Show_String(" Demo Alpha Blending");
  Foreground_color_65k(White);
  Background_color_65k(Black);
  Font_Select_12x24_24x24();
  Goto_Text_XY(0,Line51);
  Show_String("Fade in and fade out");

  //DMA initial setting
  Enable_SFlash_SPI();
    Select_SFI_1();
    Select_SFI_DMA_Mode();
    Select_SFI_24bit_Address();

    //Select_SFI_Waveform_Mode_0();
    Select_SFI_Waveform_Mode_3();

    //Select_SFI_0_DummyRead(); //normal read mode
    Select_SFI_8_DummyRead(); //1byte dummy cycle
    //Select_SFI_16_DummyRead();
    //Select_SFI_24_DummyRead();

    Select_SFI_Single_Mode();
    //Select_SFI_Dual_Mode0();
    //Select_SFI_Dual_Mode1();
  
  SPI_Clock_Period(0);


  //Clear layer2 to color black
  Canvas_Image_Start_address(layer2_start_addr);//
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

    //DMA picture to layer2
  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//
  SFI_DMA_Source_Start_Address(0);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();


  //Clear layer3 to color black
  Canvas_Image_Start_address(layer3_start_addr);//
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

    //DMA picture to layer3
  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//
  SFI_DMA_Source_Start_Address(im*400*1280*2);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();

  //BTE move alpha blending by picture 
   BTE_Destination_Color_16bpp();  
     BTE_Destination_Memory_Start_Address(layer1_start_addr);
     BTE_Destination_Image_Width(400);
     BTE_Destination_Window_Start_XY(0,0);  
     BTE_Window_Size(400,1280);

     BTE_S0_Color_16bpp();
     BTE_S0_Memory_Start_Address(layer2_start_addr);
     BTE_S0_Image_Width(400);
     BTE_S0_Window_Start_XY(0,0);

     BTE_S1_Color_16bpp();
     BTE_S1_Memory_Start_Address(layer3_start_addr);
     BTE_S1_Image_Width(400);
     BTE_S1_Window_Start_XY(0,0);


  BTE_ROP_Code(15);
    BTE_Operation_Code(10); //BTE move  

  for(j=0;j<4;j+=2)
  {
  //DMA picture to layer2
  Canvas_Image_Start_address(layer2_start_addr);//
    SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//
  SFI_DMA_Source_Start_Address(im*j*400*1280*2);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();
  //DMA picture to layer3
  Canvas_Image_Start_address(layer3_start_addr);//
  SFI_DMA_Source_Start_Address(im*(j+1)*400*1280*2);//
  Start_SFI_DMA();
    Check_Busy_SFI_DMA();

    for(i=0;i<32;i++)
    {
   BTE_Alpha_Blending_Effect(i);
   BTE_Enable();
   delay(200);
    }

    for(i=32;i>0;i--)
    {
   BTE_Alpha_Blending_Effect(i);
   BTE_Enable();
   delay(200);
    }
   }

  NextStep();

} 
//-----------------------------------------------------------------------------
void mono_Demo(void)
{


  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);  
  Canvas_Image_Start_address(0);
  Canvas_image_width(400);//
  Active_Window_XY(0,0);
  Active_Window_WH(400,1280);


  Foreground_color_65k(Red);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
  Foreground_color_65k(Green);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
  Foreground_color_65k(Blue);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
  Foreground_color_65k(Cyan);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
  Foreground_color_65k(Yellow);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
    Foreground_color_65k(Magenta);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
  Foreground_color_65k(White);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();
  NextStep();
}
//-----------------------------------------------------------------------------
void Text_Demo(void)
{
 //   uint16_t i,j;
  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

  Canvas_Image_Start_address(0);//Layer 1
  Canvas_image_width(400);//
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);


 //   Color_Bar_ON();
 //   NextStep();
  //  Color_Bar_OFF();



  Foreground_color_65k(White);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();


  /////////////////////////////////////////////////////////////////////////////    
//  LCD_DisplayString(0,Line48 ,"EastRising Technology",Blue2,White);
//  Foreground_color_65k(White);
//  Background_color_65k(Blue2);
  CGROM_Select_Internal_CGROM();
  Font_Select_12x24_24x24();
  Goto_Text_XY(20,1000);
  Show_String("www.buydisplay.ocm");
  Foreground_color_65k(Blue2);
  Background_color_65k(White);
  Goto_Text_XY(0,850);
  Show_String("7.84 inch TFT Module 400*1280 Dots");
  delay(1000);
//  NextStep();

  


  Foreground_color_65k(Black);
  Background_color_65k(White);
  CGROM_Select_Internal_CGROM();
  Font_Select_8x16_16x16();
  Goto_Text_XY(0,0);
  Show_String("Embedded 8x16 ASCII Character");

  Font_Select_12x24_24x24();
  Goto_Text_XY(16,0);
  Show_String("Embedded 12x24 ASCII Character");

  Font_Select_16x32_32x32();
  Goto_Text_XY(40,0);
  Show_String("Embedded 16x32 ASCII ");

    /*
  Font_Width_X2();
  Goto_Text_XY(0,16);
  Show_String("Character Width Enlarge x2");
  Font_Width_X1();
  Font_Height_X2();
  Goto_Text_XY(0,32);
  Show_String("Character High Enlarge x2");
  Font_Width_X2();
  Font_Height_X2();
  Goto_Text_XY(0,64);
  Show_String("Character Width & High Enlarge x2");
  */

  Font_Select_8x16_16x16();
  Font_Width_X1();
  Font_Height_X1();
  Goto_Text_XY(100,0);
  Show_String("Supporting Genitop Inc. UNICODE/BIG5/GB etc. Serial Character ROM with 16x16/24x24/32X32 dots Font.");
  Goto_Text_XY(132,0);
  Show_String("The supporting product numbers are GT21L16TW/GT21H16T1W, GT23L16U2W, GT23L24T3Y/GT23H24T3Y, GT23L24M1Z, and GT23L32S4W/GT23H32S4W, GT23L24F6Y, GT23L24S1W.");
  


    //Foreground_color_65k(color65k_yellow);
      //Background_color_65k(color65k_purple);

     //Font_Select_UserDefine_Mode();
       //CGROM_Select_Internal_CGROM();
        CGROM_Select_Genitop_FontROM();

   //GTFont_Select_GT21L16TW_GT21H16T1W();
     //GTFont_Select_GT23L16U2W();
     GTFont_Select_GT23L24T3Y_GT23H24T3Y();
     //GTFont_Select_GT23L24M1Z();
     //GTFont_Select_GT23L32S4W_GT23H32S4W();
     //GTFont_Select_GT20L24F6Y();
     //GTFont_Select_GT21L24S1W();
     //GTFont_Select_GT22L16A1Y();

   
    
    Font_Width_X1();
      Font_Height_X1();
    CGROM_Select_Genitop_FontROM();
      Font_Select_12x24_24x24();
    Select_SFI_0();
      Select_SFI_Font_Mode();
      Select_SFI_24bit_Address();
      Select_SFI_Waveform_Mode_0();
      Select_SFI_0_DummyRead();
      Select_SFI_Single_Mode();
      SPI_Clock_Period(4);   // Freq must setting <=20MHZ
   
    Enable_SFlash_SPI();

    Foreground_color_65k(color65k_green);
    Background_color_65k(color65k_blue);

    Set_GTFont_Decoder(0x11);  //BIG5  

    Goto_Text_XY(200,0);
      Show_String("Demo GT23L24T3Y BIG5:");
    Font_Select_8x16_16x16();
    Goto_Text_XY(230,0);
      Show_String("¶°³q¤¤¤åÁcÅé16x16:RA8876 TFT ¹Ï§Î±±¨î¾¹");
    Font_Select_12x24_24x24();
    Goto_Text_XY(254,0);
      Show_String("¶°³q¤¤¤åÁcÅé24x24:RA8876 TFT ¹Ï§Î±±¨î¾¹");


    Foreground_color_65k(color65k_purple);
    Background_color_65k(color65k_yellow);
    Set_GTFont_Decoder(0x01);  //GB2312  
    Goto_Text_XY(280,0);
      Show_String("Demo GT23L24T3Y GB2312:");
    Font_Select_8x16_16x16();
    Goto_Text_XY(304,0);
      Show_String("¼¯Í¨ÖÐÎÄ¼òÌå16x16:RA8876 TFT Í¼ÐÎ¿ØÖÆÆ÷");
    Font_Select_12x24_24x24();
    Goto_Text_XY(328,0);
      Show_String("¼¯Í¨ÖÐÎÄ¼òÌå16x16:RA8876 TFT Í¼ÐÎ¿ØÖÆÆ÷");

    Set_GTFont_Decoder(0x11);  //BIG5


    Foreground_color_65k(color65k_black); 
    Font_Background_select_Transparency();//³]©w­I´º³z©ú¦â
    

        Active_Window_XY(0,600);
    Active_Window_WH(80,80);
    Goto_Pixel_XY(0,600);
    Show_picture(80*80,pic_80x80); 
    Active_Window_XY(0,0);
    Active_Window_WH(400,1280);

    Goto_Text_XY(0,580);
      Show_String("text transparent");



    Font_Background_select_Color();  //³]©w­I´º¨Ï¥Î³]©wÃC¦â
    Foreground_color_65k(color65k_black); 
    Background_color_65k(color65k_white);
    delay(10000);

/*    Goto_Text_XY(0,700);
      Show_String("Demo text cursor:");
       
    Goto_Text_XY(0,724);
    Show_String("0123456789");
    Text_cursor_initial();
   delay(1000);

    for(i=0;i<14;i++)
    {
     delay(100);
     Text_Cursor_H_V(1+i,15-i);    
    }
       delay(2000);

    Disable_Text_Cursor();

    CGROM_Select_Internal_CGROM();
    Font_Select_8x16_16x16();

    Foreground_color_65k(color65k_blue); 
    Goto_Text_XY(0,484);
      Show_String("Demo graphic cursor:");

    Set_Graphic_Cursor_Color_1(0xff);
      Set_Graphic_Cursor_Color_2(0x00);

    Graphic_cursor_initial();
    Graphic_Cursor_XY(0,508);
    Select_Graphic_Cursor_1();  
       delay(2000);
    Select_Graphic_Cursor_2();
    delay(2000);
    Select_Graphic_Cursor_3();
    delay(2000);
    Select_Graphic_Cursor_4();
    delay(2000);
    Select_Graphic_Cursor_2(); 

    for(j=0;j<2;j++)
    {
     for(i=0;i<400;i++)
     {
      Graphic_Cursor_XY(i,508+j*20);  
    delay(2);   
     }
    }
     Graphic_Cursor_XY(0,508);  

   delay(200);
   Disable_Graphic_Cursor();   */

  NextStep();


 }

//-----------------------------------------------------------------------------

 void DMA_Demo(void)
{
    uint32_t i;

 ///////////////////////////////////////////////////////////////

  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

  Canvas_Image_Start_address(0);//Layer 1
  Canvas_image_width(400);//
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);





    //DMA initial setting
  Enable_SFlash_SPI();
    Select_SFI_1();
    Select_SFI_DMA_Mode();
    Select_SFI_24bit_Address();

    //Select_SFI_Waveform_Mode_0();
    Select_SFI_Waveform_Mode_3();

    //Select_SFI_0_DummyRead(); //normal read mode
    Select_SFI_8_DummyRead(); //1byte dummy cycle
    //Select_SFI_16_DummyRead();
   //Select_SFI_24_DummyRead();

   // Select_SFI_Single_Mode();
    Select_SFI_Dual_Mode0();
   // Select_SFI_Dual_Mode1();

    SPI_Clock_Period(0);



//DMA initail normally command setting
//  LCD_CmdWrite(0x01);
//  LCD_DataWrite(0x83);  
//  LCD_CmdWrite(0xB7);
//  LCD_DataWrite(0xd4);
//  LCD_CmdWrite(0xBB);
//  LCD_DataWrite(0x00);

   /*
   //if used 32bit address Flash ex.256Mbit,512Mbit
   //must be executed following in 24bit address mode to switch to 32bit address mode

   //Select_nSS_drive_on_xnsfcs0();
   Select_nSS_drive_on_xnsfcs1();

   Reset_CPOL();
   //Set_CPOL();
   Reset_CPHA();
   //Set_CPHA();

  //Enter 4-byte mode  
   nSS_Active();
   SPI_Master_FIFO_Data_Put(0xB7);  //switch to 32bit address mode
   delayMicroseconds(10); 
   nSS_Inactive();

   Select_SFI_32bit_Address();
   */


  SFI_DMA_Destination_Upper_Left_Corner(0,0);
    SFI_DMA_Transfer_Width_Height(400,1280);
    SFI_DMA_Source_Width(400);//

  //execute DMA to show 400x1280 picture
   for(i=0;i<7;i++)
   {
    SFI_DMA_Source_Start_Address(i*400*1280*2);//
    Start_SFI_DMA();
      Check_Busy_SFI_DMA();
    delay(30);
  NextStep();
   }


}
//-----------------------------------------------------------------------------
 void Geometric(void)
{
  uint16_t i;
  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);

  Canvas_Image_Start_address(0);//Layer 1
  Canvas_image_width(400);//
    Active_Window_XY(0,0);
  Active_Window_WH(400,1280);



///////////////////////////Square

    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

   for(i=0;i<=200;i+=8)
  {Foreground_color_65k(Red);
  Line_Start_XY(0+i,0+i);
  Line_End_XY(399-i,1279-i);
  Start_Square();
  delay(50);
  }

    for(i=0;i<=200;i+=8)
  {Foreground_color_65k(Black);
  Line_Start_XY(0+i,0+i);
  Line_End_XY(399-i,1279-i);
  Start_Square();
  delay(50);
  }
  delay(2000);
///////////////////////////Square Of Circle
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

   for(i=0;i<=160;i+=8)
  {Foreground_color_65k(Green);
  Line_Start_XY(0+i,0+i);
  Line_End_XY(399-i,1279-i);
  Circle_Square_Radius_RxRy(10,10);
  Start_Circle_Square();
  delay(50);
  }

    for(i=0;i<=160;i+=8)
  {Foreground_color_65k(Black);
  Line_Start_XY(0+i,0+i);
  Line_End_XY(399-i,1279-i);
  Circle_Square_Radius_RxRy(10,10);
  Start_Circle_Square();
  delay(50);
  }
    delay(2000);

///////////////////////////Circle
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

   for(i=0;i<=200;i+=8)
  {Foreground_color_65k(Blue);
  Circle_Center_XY(400/2,1280/2);
  Circle_Radius_R(i);
  Start_Circle_or_Ellipse();
  delay(50);
  }

    for(i=0;i<=200;i+=8)
  {Foreground_color_65k(Black);
  Circle_Center_XY(400/2,1280/2);
  Circle_Radius_R(i);
  Start_Circle_or_Ellipse();
  delay(50);
  }
    delay(2000);

///////////////////////////Ellipse
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

   for(i=0;i<=200;i+=8)
  {Foreground_color_65k(White);
  Circle_Center_XY(400/2,1280/2);
  Ellipse_Radius_RxRy(i,i+100);
  Start_Circle_or_Ellipse();
  delay(50);
  }

    for(i=0;i<=200;i+=8)
  {Foreground_color_65k(Black);
  Circle_Center_XY(400/2,1280/2);
  Ellipse_Radius_RxRy(i,i+100);
  Start_Circle_or_Ellipse();
  delay(50);
  }
    delay(2000);

 ////////////////////////////Triangle
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

    for(i=0;i<=180;i+=8)
  {Foreground_color_65k(Yellow);
  Triangle_Point1_XY(i,1280/2);
  Triangle_Point2_XY(399-i,i*2);
  Triangle_Point3_XY(399-i,1279-i*2);
  Start_Triangle();
  delay(50);
  }

    for(i=0;i<=180;i+=8)
  {Foreground_color_65k(Black);
  Triangle_Point1_XY(i,1280/2);
  Triangle_Point2_XY(399-i,i*2);
  Triangle_Point3_XY(399-i,1279-i*2);
  Start_Triangle();
  delay(50);
  }
    delay(2000);


 ////////////////////////////line
    Foreground_color_65k(Black);
  Line_Start_XY(0,0);
  Line_End_XY(399,1279);
  Start_Square_Fill();

    for(i=0;i<=400;i+=8)
  {Foreground_color_65k(Cyan);
  Line_Start_XY(i,0);
  Line_End_XY(399-i,1279);
  Start_Line();
  //delay(50);
  }
     for(i=0;i<=1280;i+=8)
  {Foreground_color_65k(Cyan);
  Line_Start_XY(0,1279-i);
  Line_End_XY(399,i);
  Start_Line();
  //delay(50);
  }

  /*
    for(i=0;i<=400;i+=8)
  {Foreground_color_65k(Black);
  Line_Start_XY(i,0);
  Line_End_XY(399-i,1279);
  Start_Line();
  delay(50);
  }
  for(i=0;i<=1280;i+=8)
  {Foreground_color_65k(Black);
  Line_Start_XY(0,1279-i);
  Line_End_XY(399,i);
  Start_Line();
  delay(50);
  }
      */
    delay(2000);


}
//-----------------------------------------------------------------------------

void gray(void)
{ 
  int i,col,line;

  Select_Main_Window_16bpp();
  Main_Image_Start_Address(0);        
  Main_Image_Width(400);              
  Main_Window_Start_XY(0,0);  
  Canvas_Image_Start_address(0);
  Canvas_image_width(400);//
  Active_Window_XY(0,0);
  Active_Window_WH(400,1280);



   col=0;line=0;
  for(i=0;i<32;i++)
    { Foreground_color_65k(i<<11);
    Line_Start_XY(col,line);
    Line_End_XY(col+12,line+213);
    Start_Square_Fill();
    col+=12;
  }
     col=0;line=213;
  for(i=31;i>=0;i--)
    { Foreground_color_65k(i<<11);
    Line_Start_XY(col,line);
    Line_End_XY(col+12,line+213);
    Start_Square_Fill();
    col+=12;
  }

   col=0;line=426;
  for(i=0;i<64;i++)
    { Foreground_color_65k(i<<5);
    Line_Start_XY(col,line);
    Line_End_XY(col+6,line+213);
    Start_Square_Fill();
    col+=6;
  }
     col=0;line=639;
  for(i=63;i>=0;i--)
    { Foreground_color_65k(i<<5);
    Line_Start_XY(col,line);
    Line_End_XY(col+6,line+213);
    Start_Square_Fill();
    col+=6;
  }


   col=0;line=852;
  for(i=0;i<32;i++)
    { Foreground_color_65k(i);
    Line_Start_XY(col,line);
    Line_End_XY(col+12,line+213);
    Start_Square_Fill();
    col+=12;
  }
     col=0;line=1065;
  for(i=31;i>=0;i--)
    { Foreground_color_65k(i);
    Line_Start_XY(col,line);
    Line_End_XY(col+12,line+213);
    Start_Square_Fill();
    col+=12;
  }


    delay(1000);
  NextStep();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
