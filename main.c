#include <stdio.h>
#include "floating.h"
#include <stdlib.h>
// Following is the user interface of the conversion
int main(int argc, char **argv){
  int i;
  union floating f;
  uint16_t half;
  char buffer[256];
  if(sizeof(float) != 4){
    printf("Error:  Can only run on a system where sizeof(float) == 32");
    return -1;
  }
  for(i = 1; i < argc; ++i){
    printf("Input: \"%s\"\n", argv[i]);
    f.as_float = strtof(argv[i], NULL);
    printf("Float: %f\n", f.as_float);
    printf("Hex: 0x%08x\n", f.as_int);
    printf("Info: %s\n", floating_info(f, buffer, 256));
    half = as_ieee_16(f);
    printf("Half Hex: 0x%04x\n", half);
    printf("Half info: %s\n", ieee_16_info(half, buffer, 256));
    printf("\n");
  }

  return 0;
}
