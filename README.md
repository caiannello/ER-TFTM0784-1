```
///////////////////////////////////////////////////////////////////////////////

Display library for EASTRISING 1280x400 TFT LCD (model ER-TFTM0784-1)

This one is different from its peers, which usually have just a RA8876. 
It adds An SSD2828 controller which requires its own initialization.

This is a sloppy port to Arduino of demo code originally written by 
JAVEN LIU for a C8051 microcontroller.

Product URL:

https://www.buydisplay.com/spi-1280x400-7-84-ips-tft-lcd-module-with-ra8876-capacitive-touch-panel

///////////////////////////////////////////////////////////////////////////////

-------------------------------------------------------------------------------
The module has three main controllers on it:
-------------------------------------------------------------------------------

    RA8876:     Main display controller which does the graphics and text.
                It has attached 16MB SDRAM and can do DMA to move graphics
                around and stuff.

                It can use any of the following communications schemes:
                    
                    * SPI 4-Wire (up to ~50Mhz if wires are short)
                    * 8080 Parallel, 16 bit (needs a TON of IO PINS)
                    * 8080 Parallel, 8 bit

    SSD2828:    Low-level MIPI LCD driver. SPI interface. Needs to be 
                sent some configuration data on powerup, and it can be
                ignored after that.

    GT911:      Capacitive touch panel controller. I2C interface + 1 Interrupt
                flag output pin. 

-------------------------------------------------------------------------------
NOTE
-------------------------------------------------------------------------------

I ordered mine configured for 3.3V and SPI interface. It draws around 300 mA 
at this voltage, so it can't be powered directly by any Arduino. An external 
power supply is necessary. OR, you can configure it for 5V and not worry about 
it. 
        
I did my development and testing on an Arduino DUE, hence the 3.3V, but the 
code should work on other Arduinos or Teensy with minimal modification.

-------------------------------------------------------------------------------
CONNECTING AN ARDUINO DUE IN SPI MODE
-------------------------------------------------------------------------------

SSD2828 Panel Driver 

    /CS             24
    RESET           25
    SDI             26
    SCLK            27

GT911 Touch Screen

    SDA             28
    SCL             29
    INT             30
    RESET           31

RA8876 LCD Controller

    RESET           34
    /CS             10
    SDI             109 (Arduino DUE ISP port MOSI pin)
    SDO             108 (Arduino DUE ISP port, MISO pin)
    SCLK            110 (Arduino DUE ISP port, SCK pin)

-------------------------------------------------------------------------------
CONNECTING AN ARDUINO DUE IN 16-BIT 8080 PARALLEL MODE
-------------------------------------------------------------------------------

SSD2828 Panel Driver 

    /CS             24
    RESET           25
    SDI             26
    SCLK            27

GT911 Touch Screen

    SDA             28
    SCL             29
    INT             30
    RESET           31

RA8876 LCD Controller

    RESET           34
    RS              33
    /CS             34
    WR              35
    RD              36
    D0              38          
    D1              39
    D2              40
    D3              41
    D4              42
    D5              43
    D6              44
    D7              45
    D8              46
    D9              47
    D10             48
    D11             49
    D12             50
    D13             51
    D14             52
    D15             53

-------------------------------------------------------------------------------
EXAMPLES
-------------------------------------------------------------------------------

demo.ino     - Demonstrates the RA8876 graphics.

-------------------------------------------------------------------------------
HELP WANTED
-------------------------------------------------------------------------------

There is much room for improvement and optimization here, so please
lend a hand.

---
```
