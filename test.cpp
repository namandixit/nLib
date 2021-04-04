/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */
#include "nlib_tests.h"

Sint main (Sint argc, Char *argv[])
{
    unused_variable(argc);
    unused_variable(argv);

    raUnitTest(); // Uses sbufPrint, so tests might clash with printUnitTest
    htUnitTest();
    internUnitTest();
    mapUnitTest();
    printUnitTest();
    // ringLockedUnitTest();
    // queueLockedUnitTest();
   return 0;
}
