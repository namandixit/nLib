/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */
#define NLIB_TESTS
#define NLIB_NO_LIBC
#include "nlib.h"

int main (void);
int main (void)
{
    say("=============== %f\n", 38685626227668133590597632.0);
    sbufUnitTest(); // Uses sbufPrint, so tests might clash with printUnitTest
    htUnitTest();
    internUnitTest();
    mapUnitTest();
//    printUnitTest();
    return 0;
}
