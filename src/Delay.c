#include "main.h"

void __DELAY(uint32_t cycles){
    int i;
    for (i = cycles; i > 0; i--) __asm__("nop");
}
