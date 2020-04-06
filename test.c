/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2019 Naman Dixit
 */

#include "nlib.h"
#include "print.h"

int main (void)
{
    sbufUnitTest();
    htUnitTest();
    internUnitTest();
    mapUnitTest();

    print("%s\n", "Hello");
    print("%c\n", '!');
    print("%b %b\n", 9, 11);
    print("%o %o\n", 9, 11);
    print("%x %x\n", 9, 11);
    print("%X %X\n", 9, 11);
    print("%ld %ld %lu\n", INT64_MAX, INT64_MIN, UINT64_MAX);
    print("%p ", (void*)print); printf("%p\n", (void*)print);

    print("\n");

    int a = 0;
    print("%d%n\t", 12345, &a); print("%d\n", a);

    print("\n");

    print("%10d\n", -12345678);
    print("%*d\n", 10, -12345678);
    print("%*d\n", 5, -12345678);

    print("\n");

    print("%.10d\n", -12345678);
    print("%.*d\n", 10, -12345678);
    print("%.*d\n", 5, -12345678);

    print("\n");

    print("%-10.4d\n", 43);
    print("%+10.4d\n", 43);
    print("%010.4d\n", 43);

    return 0;
}
