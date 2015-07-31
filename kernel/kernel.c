
#include <stdio.h>

#include "text-cons.h"

void kInit(void)
{
    console_Init();

    printf("Hello, world!\n");
}

void kMain(void)
{
    printf("kMain()\n");
}
