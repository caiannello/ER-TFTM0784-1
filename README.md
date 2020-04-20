# ER-TFTM0784-1
Arduino library for the 1280x400 TFT LCD + Touchscreen from BuyDisplay.com

https://www.buydisplay.com/spi-1280x400-7-84-ips-tft-lcd-module-with-ra8876-capacitive-touch-panel

Ported to Arduino by Craig Iannello, original by Javen Liu 


Devices accessed and pin usage:

   CHIP    PURPOSE           INTERFACE
   RA8876  Graphics Driver   SPI 4-Wire OR 8/16 bit Parallel
   GT911   Touch             I2C + Interrupt flag output
   SSD2828 MIPI bridge       SPI 4-Wire
   
The demo example is pretty much verbatim of Javen's original.

Note the two seperate SPI's.  I read that the RA8876 doesn't properly
tri-state (release) the bus while it's unselected. I was too lazy to test
that, and just left the SSD2828 as software serial on some pins,
and hooked the HW SPI up to the RA8876. The 2828 only needs to be spoken 
to on powerup and doesnt need to be fast.

The touch panel code has been left as bit-bang too out of laziness.

