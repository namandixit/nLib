/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */
//#define NLIB_MALLOC_NO_GLOBAL
#define NLIB_TESTS
#include "nlib.h"

int main (void)
{
    sbufUnitTest(); // Uses sbufPrint, so tests might clash with printUnitTest
    htUnitTest();
    internUnitTest();
    mapUnitTest();
    printUnitTest();
    return 0;
}
