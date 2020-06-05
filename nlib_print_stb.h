/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 * SPDX-License-Identifier: 0BSD OR BSL-1.0 OR Unlicense
 */

#if !defined(NLIB_PRINT_STB_H_INCLUDE_GUARD)

global_immutable
struct
{
    S16 temp; // force next field to be 2-byte aligned
    Char pair[201];
} stbfp_digitpair = { 0,
                      "00010203040506070809101112131415161718192021222324"
                      "25262728293031323334353637383940414243444546474849"
                      "50515253545556575859606162636465666768697071727374"
                      "75767778798081828384858687888990919293949596979899" };

#define STBFP_SPECIAL 0x7000

global_immutable
F64 stbfp_bot[23] = {
    1e+000, 1e+001, 1e+002, 1e+003, 1e+004, 1e+005, 1e+006, 1e+007, 1e+008, 1e+009,
    1e+010, 1e+011, 1e+012, 1e+013, 1e+014, 1e+015, 1e+016, 1e+017, 1e+018, 1e+019,
    1e+020, 1e+021, 1e+022
};

global_immutable
F64 stbfp_negbot[22] = {
    1e-001, 1e-002, 1e-003, 1e-004, 1e-005, 1e-006, 1e-007, 1e-008, 1e-009, 1e-010, 1e-011,
    1e-012, 1e-013, 1e-014, 1e-015, 1e-016, 1e-017, 1e-018, 1e-019, 1e-020, 1e-021, 1e-022
};

global_immutable
F64 stbfp_negboterr[22] = {
    -5.551115123125783e-018,  -2.0816681711721684e-019, -2.0816681711721686e-020,
    -4.7921736023859299e-021, -8.1803053914031305e-022, 4.5251888174113741e-023,
    4.5251888174113739e-024,  -2.0922560830128471e-025, -6.2281591457779853e-026,
    -3.6432197315497743e-027, 6.0503030718060191e-028,  2.0113352370744385e-029,
    -3.0373745563400371e-030, 1.1806906454401013e-032,  -7.7705399876661076e-032,
    2.0902213275965398e-033,  -7.1542424054621921e-034, -7.1542424054621926e-035,
    2.4754073164739869e-036,  5.4846728545790429e-037,  9.2462547772103625e-038,
    -4.8596774326570872e-039
};

global_immutable
F64 stbfp_top[13] = {
    1e+023, 1e+046, 1e+069, 1e+092, 1e+115, 1e+138, 1e+161, 1e+184, 1e+207, 1e+230,
    1e+253, 1e+276, 1e+299
};

global_immutable
F64 stbfp_negtop[13] = {
    1e-023, 1e-046, 1e-069, 1e-092, 1e-115, 1e-138, 1e-161, 1e-184, 1e-207, 1e-230,
    1e-253, 1e-276, 1e-299
};

global_immutable
F64  stbfp_toperr[13] = {
    8388608,
    6.8601809640529717e+028,
    -7.253143638152921e+052,
    -4.3377296974619174e+075,
    -1.5559416129466825e+098,
    -3.2841562489204913e+121,
    -3.7745893248228135e+144,
    -1.7356668416969134e+167,
    -3.8893577551088374e+190,
    -9.9566444326005119e+213,
    6.3641293062232429e+236,
    -5.2069140800249813e+259,
    -5.2504760255204387e+282
};

global_immutable
F64 stbfp_negtoperr[13] = {
    3.9565301985100693e-040,  -2.299904345391321e-063,  3.6506201437945798e-086,  1.1875228833981544e-109,
    -5.0644902316928607e-132, -6.7156837247865426e-155, -2.812077463003139e-178,  -5.7778912386589953e-201,
    7.4997100559334532e-224,  -4.6439668915134491e-247, -6.3691100762962136e-270, -9.436808465446358e-293,
    8.0970921678014997e-317
};

global_immutable
U64 stbfp_powten[20] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
    10000000000ULL,
    100000000000ULL,
    1000000000000ULL,
    10000000000000ULL,
    100000000000000ULL,
    1000000000000000ULL,
    10000000000000000ULL,
    100000000000000000ULL,
    1000000000000000000ULL,
    10000000000000000000ULL
};
#define stbfp_tento19th (1000000000000000000ULL)

#define stbfp_ddmulthi(oh, ol, xh, yh) do {                             \
        F64 ahi = 0, alo, bhi = 0, blo;                                 \
        S64 bt;                                                         \
        oh = xh * yh;                                                   \
        memcpy(&bt, &xh, 8);                                            \
        bt &= ((~(U64)0) << 27);                                         \
        memcpy(&ahi, &bt, 8);                                           \
        alo = xh - ahi;                                                 \
        memcpy(&bt, &yh, 8);                                            \
        bt &= ((~(U64)0) << 27);                                         \
        memcpy(&bhi, &bt, 8);                                           \
        blo = yh - bhi;                                                 \
        ol = ((ahi * bhi - oh) + ahi * blo + alo * bhi) + alo * blo;    \
    } while (0)

#define stbfp_ddtoS64(ob, xh, xl) do {          \
        F64 ahi = 0, alo, vh, t;                \
        ob = (S64)xh;                           \
        vh = (F64)ob;                           \
        ahi = (xh - vh);                        \
        t = (ahi - xh);                         \
        alo = (xh - (ahi - t)) - (vh + t);      \
        ob += (S64)(ahi + alo + xl);             \
    } while (0)

#define stbfp_ddrenorm(oh, ol) do {             \
        F64 s;                                  \
        s = oh + ol;                            \
        ol = ol - (s - oh);                     \
        oh = s;                                 \
    } while (0)

#define stbfp_ddmultlo(oh, ol, xh, xl, yh, yl) do { ol = ol + (xh * yl + xl * yh); } while (0)

#define stbfp_ddmultlos(oh, ol, xh, yl) do { ol = ol + (xh * yl); } while (0)

header_function
void stbfp_raise_to_power10(F64 base, S32 exponent, // exponent can be -323 to +350
                            F64 *output_high, F64 *output_low)
{
    F64 ph, pl;
    if ((exponent >= 0) && (exponent <= 22)) {
        stbfp_ddmulthi(ph, pl, base, stbfp_bot[exponent]);
    } else {
        S32 e = exponent;
        if (exponent < 0) e = -e;
        S32 et = (e * 0x2c9) >> 14; /* floor( e / 23) */
        if (et > 13) et = 13; /* e=321->et=13, e=322->et=14 */
        S32 eb = e - (et * 23); /* e % 23 if 0 <= e <= 321 */

        ph = base;
        pl = 0.0;
        F64 p2h, p2l;
        if (exponent < 0) {
            if (eb) {
                --eb;
                stbfp_ddmulthi(ph, pl, base, stbfp_negbot[eb]);
                stbfp_ddmultlos(ph, pl, base, stbfp_negboterr[eb]);
            }

            if (et) {
                stbfp_ddrenorm(ph, pl);
                --et;
                stbfp_ddmulthi(p2h, p2l, ph, stbfp_negtop[et]);
                stbfp_ddmultlo(p2h, p2l, ph, pl, stbfp_negtop[et], stbfp_negtoperr[et]);
                ph = p2h;
                pl = p2l;
            }
        } else {
            if (eb) {
                e = eb;
                if (eb > 22)
                    eb = 22;
                e -= eb;
                stbfp_ddmulthi(ph, pl, base, stbfp_bot[eb]);

                if (e) {
                    stbfp_ddrenorm(ph, pl);
                    stbfp_ddmulthi(p2h, p2l, ph, stbfp_bot[e]);
                    stbfp_ddmultlos(p2h, p2l, stbfp_bot[e], pl);
                    ph = p2h;
                    pl = p2l;
                }
            }

            if (et) {
                stbfp_ddrenorm(ph, pl);
                --et;
                stbfp_ddmulthi(p2h, p2l, ph, stbfp_top[et]);
                stbfp_ddmultlo(p2h, p2l, ph, pl, stbfp_top[et], stbfp_toperr[et]);
                ph = p2h;
                pl = p2l;
            }
        }
    }

    stbfp_ddrenorm(ph, pl);
    *output_high = ph;
    *output_low = pl;
}

#define NLIB_PRINT_STB_H_INCLUDE_GUARD
#endif
