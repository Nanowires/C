//Kompilieren mit: gcc -o BSP BSP.c -lbcm2835  
/* Unter Umständen muss noch die Bibliothek bcm2835 vorher installiert werden.
*/

#include "hy18spi.h"

int main(int argc, char **argv)
{
   unsigned int pin;
   int x,y;
	
startTft();

    // initialize display
    hy18spi_init();

    // set black background
    hy18spi_set_rgb(0x0000);

    // wait a sec
    delay(1000);
   
    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x+y);
      }
    }
    // wait a 10 sec
    delay(1000);
 
    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x*y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x|y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x&y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, x^y);
      }
    }
    // wait a 10 sec
    delay(1000);

    // draw pattern
    for(x=0; x<128;x++) {
      for(y=0; y<160;y++) {
        hy18spi_set_pixel(x, y, (x+y)*(x+y));
      }
    }
    // wait a 10 sec
    delay(1000);

    // set a fancy background
    hy18spi_set_rgb(0xf0f0);
    // wait a sec
    delay(1000);

    // deinitialize display
    hy18spi_end();

    // deintialize spi
    bcm2835_spi_end();

    // deintialize bcm2835 library
    bcm2835_close();

    return 0;
}
