/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */
//#define NLIB_MALLOC_NO_GLOBAL
#define NLIB_TESTS
#include "nlib.h"

int main (void)
{
    sbufUnitTest();
    htUnitTest();
    internUnitTest();
    mapUnitTest();

#if 0
    say("%s\n", "Hello");
    say("%c\n", '!');
    say("%b %b\n", 9, 11);
    say("%o %o\n", 9, 11);
    say("%x %x\n", 9, 11);
    say("%X %X\n", 9, 11);
    say("%ld %ld %lu\n", INT64_MAX, INT64_MIN, UINT64_MAX);
    say("%p ", (void*)say); printf("%p\n", (void*)say);

    say("\n");

    int a = 0;
    say("%d%n\t", 12345, &a); say("%d\n", a);

    say("\n");

    say("%10d\n", -12345678);
    say("%*d\n", 10, -12345678);
    say("%*d\n", 5, -12345678);

    say("\n");

    say("%.10d\n", -12345678);
    say("%.*d\n", 10, -12345678);
    say("%.*d\n", 5, -12345678);

    say("\n");

    say("%-10.4d\n", 43);
    say("%+10.4d\n", 43);
    say("%010.4d\n", 43);
#endif
    return 0;
}
