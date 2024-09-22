#ifndef FLOATING_H
#define FLOATING_H

#include <stdint.h>

union floating {
  uint32_t as_int;
  float as_float;
};

/* This function is designed to provide information about
   the IEEE floating point value passed in.  Note that this
   ONLY works on systems where sizeof(float) == 4 */
char *floating_info(union floating f, char *buf, size_t buflen);

/* This function is designed to provide information about
   the 16b IEEE floating point value passed in.
   https://en.wikipedia.org/wiki/Half-precision_floating-point_format
*/

char *ieee_16_info(uint16_t f, char *buf, size_t buflen);



/* This function converts an IEEE 32b floating point value
   into a 16b IEEE floating point value.
 */
uint16_t as_ieee_16(union floating f);


#endif
