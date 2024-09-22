

/*
 * Standard IO and file routines.
 */
#include <stdio.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "floating.h"


/* This function is designed to provide information about
   the IEEE floating point value passed in.  Note that this
   ONLY works on systems where sizeof(float) == 4.

   For a normal floating point number it it should have a + or -
   depending on the sign bit, then the significand in binary in the
   format of the most significant bit, a decimal point, and then the
   remaining 23 bits, a space, and then the exponent as 2^ and then a
   signed integer.  As an example, the floating point number .5 is
   represented as "+1.00000000000000000000000 2^-1" (without the
   quotation marks).

   There are a couple of special cases: 0 should be "+0" or "-0"
   depending on the sign.  An infinity should be "+INF" or "-INF", and
   a NAN should be "NaN".

   For denormalized numbers, write them with a leading 0. and then the
   bits in the denormalized value.

   The output should be truncated if the buffer is
   not sufficient to include all the data.
*/
char *floating_info(union floating f, char *buf, size_t buflen){
  // the sign of the number
  buf[buflen-1] = '\0';
	int sign = f.as_int>>31;
  int exponent = (f.as_int<<1>>24)-127;
  int significand = f.as_int & 0x7FFFFF;
  if (exponent == -127 && significand == 0) {
    if (sign == 0) snprintf(buf, buflen, "%s", "+0");
    else snprintf(buf, buflen, "%s", "-0");
  }
  else if (exponent == 128) {
    if (significand == 0) {
      if (sign == 0)snprintf(buf, buflen, "%s", "+INF");
      else snprintf(buf, buflen, "%s", "-INF");
    }
    else{
      snprintf(buf, buflen, "%s", "NaN");
    }
  }// normal cases
  else{
    // denormalized form
    char mantissa[24];
    mantissa[23] = '\0';
    if (exponent == -127) {
      exponent = -126;
      for (int i = 0; i < 23; i++) {
        if((significand & (1<<(22-i)))!=0) mantissa[i] = '1';
        else mantissa[i] = '0';
      }
      if (sign == 0) snprintf(buf, buflen, "+0.%s 2^%d", mantissa, exponent);
      else  snprintf(buf, buflen, "-0.%s 2^%d", mantissa, exponent);
      buf[1]='0';
      buf[2]='.';
    }
    else{
      for (int i = 0; i < 23; i++) {
        if((significand & (1<<(22-i)))!=0) mantissa[i] = '1';
        else mantissa[i] = '0';
      }
      if (sign == 0)snprintf(buf, buflen, "+1.%s 2^%d", mantissa, exponent);
      else snprintf(buf, buflen, "-1.%s 2^%d", mantissa, exponent);
    }
  }
  return buf;
}

/* This function is designed to provide information about
   the 16b IEEE floating point value passed in with the same exact format.  */
char *ieee_16_info(uint16_t f, char *buf, size_t buflen){
  // the sign of the number
  buf[buflen-1] = '\0';
	int sign = f>>15;
  int exponent =((f & 0x7C00) >> 10 )-15;
  int significand = f & 0x3FF;
  if (exponent == -15 && significand == 0) {
    if (sign == 0) snprintf(buf, buflen, "%s", "+0");
    else snprintf(buf, buflen, "%s", "-0");
  }
  else if (exponent == 16) {
    if (significand == 0) {
      if (sign == 0) snprintf(buf, buflen, "%s", "+INF");
      else  snprintf(buf, buflen, "%s", "-INF");
    }
    else{
      snprintf(buf, buflen, "%s", "NaN");
    }
  }// normal cases
  else{
    // denormalized form
    char mantissa[11]; // mantissa is 10 in 16 bits
    mantissa[10] = '\0';
    if (exponent == -15) {
      exponent = -14;
      for (int i = 0; i < 10; i++) {
        if((significand & (1<<(9-i)))!=0) mantissa[i] = '1';
        else mantissa[i] = '0';
      }
      if (sign == 0)buf[0]='+';
      else buf[0] = '-';
      buf[1]='0';
      buf[2]='.';
      //distributing mantissa
      snprintf(&(buf[3]), buflen, "%s 2^%d", mantissa, exponent);
    }
    else{
      for (int i = 0; i < 10; i++) {
        if((significand & (1<<(9-i)))!=0) mantissa[i] = '1';
        else mantissa[i] = '0';
      }
      if (sign == 0)buf[0]='+';
      else buf[0] = '-';
      buf[1]='1';
      buf[2]='.';
      //distributing mantissa
      snprintf(&(buf[3]), buflen, "%s 2^%d", mantissa, exponent);
    }
  }
  return buf;
}



/* This function converts an IEEE 32b floating point value into a 16b
   IEEE floating point value.  As a reminder: The sign bit is 1 bit,
   the exponent is 5 bits (with a bias of 15), and the significand is
   10 bits.

   There are several corner cases you need to make sure to consider:
   a) subnormal
   b) rounding:  We use round-to-even in case of a tie.
   c) rounding increasing the exponent on the significand.
   d) +/- 0, NaNs, +/- infinity.
 */
uint16_t as_ieee_16(union floating f){
  int sign = f.as_int>>31;
  int expo = (f.as_int<<1>>24)-127;
  int expo_16 = expo + 15;
  int significand = f.as_int & 0x7FFFFF;
  if (expo == 128){
    int temp = sign<<15;
    if (significand == 0){ // for infinity
      temp |= 0x7c00;
    }
    else{ // for nan
      temp |= 1;
    }
    return temp;
  }
  else if (expo == -127 && significand == 0) {
    return sign<<15;
  }
  else{// rounding part
  // underflow and overflow
    int temp;
    if (expo < -24) {//problem
      temp = sign<<15;
      return temp;// round to zero
    }
    else if (expo_16 > 31) {

      temp = (sign<<15)|0x7C00;// round to infinity
      return temp;
    }
    else{
      int sig_16 = significand >> 13;
      if (expo_16 <= 0) {
        int shift = 14 - expo_16;
        significand |= 0x800000;
        sig_16 = significand >> shift;
        int round_bit = (significand>>(shift-1)) & 1;
        int sticky_bit = significand & (((1 << shift)-1)>>1);
        if (round_bit == 0) {
            temp = sign << 15;
            temp |= sig_16;
            return temp;
        }
        else{
            if (sticky_bit!=0) {
                temp = sign << 15;
                sig_16 += 1;
                temp |= sig_16;
                return temp;
            }
            else{
              if (sig_16 & 1) {
                temp = sign << 15;
                sig_16 += 1;
                temp |= sig_16; 
                return temp;
              }
              else{
                temp = sign << 15;
                temp |= sig_16;
                return temp;
              }
            }
        }
    }
    else{
      int round_bit = (significand >> 12) & 1;
      int sticky_bit = significand & 0xFFF;
        if (round_bit == 0) {
            temp = sign << 15;
            temp |= (expo_16 << 10);
            temp |= sig_16;
            return temp;
        }
        else{
            if (sticky_bit!=0) {
              sig_16 += 1;
              temp = sign << 15;
              temp |= (expo_16<<10);
              temp |= sig_16;
              return temp;
          }  
            else{
              if (sig_16&1) {
                temp = sign << 15;
                sig_16 += 1;
                temp |= (expo_16<<10);
                temp |= sig_16; 
                return temp;
              }
              else{
                temp = sign << 15;
                temp |= (expo_16<<10);
                temp |= sig_16;
                return temp;
              }
            }
        }
    }
      
  }
}
}