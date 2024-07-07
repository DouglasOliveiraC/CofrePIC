/*

high level delay routines - see delay.h for more info.

Designed by Shane Tolmie of www.microchipC.com corporation.  Freely distributable.
Questions and comments to webmaster@microchipC.com.
PICuWEB - Program PIC micros with C. Site has FAQ and sample source code.
www.microchipC.com.

For Microchip 12C67x, 16C7x, 16F87x and Hi-Tech C 
[jo:130813] works for XC8 too!
[jo:160207] function names changed to comply with PMR2415 Coding standard

*/

#ifndef __DELAY_C
#define __DELAY_C
#include <xc.h>
#include "always.h"

unsigned char delayus_variable;

#include "delay.h"

void delay_big_us(unsigned int cnt) {
  unsigned char	i;

  i = (unsigned char)(cnt>>8);
  while(i>=1) {
    i--;
    delay_us(253);
    CLRWDT();
  }
  delay_us((unsigned char)(cnt & 0xFF));
}

void delay_ms(unsigned char cnt) {
  unsigned char	i;
  do {
    i = 4;
    do {
      delay_us(250);
      CLRWDT();
    } while(--i);
  } while(--cnt);
}

//this copy is for the interrupt function
void delay_ms_interrupt(unsigned char cnt) {
  unsigned char	i;
  do {
    i = 4;
    do {
      delay_us(250);
    } while(--i);
  } while(--cnt);
}

void delay_big_ms(unsigned int cnt) {
  unsigned char	i;
  do {
    i = 4;
    do {
      delay_us(250);
      CLRWDT();
    } while(--i);
  } while(--cnt);
}

void delay_s(unsigned char cnt) {
  unsigned char i;
  do {
    i = 4;
    do {
      delay_ms(250);
      CLRWDT();
    } while(--i);
  } while(--cnt);
}

#endif


