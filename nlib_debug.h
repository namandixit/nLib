/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#if !defined(NLIB_DEBUG_H_INCLUDE_GUARD)

# if defined(OS_WINDOWS)

#  if defined(BUILD_DEBUG)
#   define breakpoint() __debugbreak()
#   define quit() breakpoint()
#  else
#   define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#   define quit() ExitProcess(0)
#  endif

# elif defined(OS_LINUX)

#  if defined(BUILD_DEBUG)
#   define breakpoint() __asm__ volatile("int $0x03")
#   define quit() breakpoint()
#  else
#   define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#   define quit() exit(0)
#  endif

# endif

/* ==============
 * Claim (assert)
 */

# if defined(BUILD_DEBUG)
#  define claim(cond) claim_(cond, #cond, __FILE__, __LINE__)
header_function
void claim_ (B32 cond,
             Char *cond_str,
             Char *filename, U32 line_num)
{
    if (!cond) {
        report("Claim \"%s\" Failed in %s:%u\n\n",
               cond_str, filename, line_num);

        quit();
    }
}
# else
#  define claim(cond) do { cond; } while (0)
# endif

/* ===================
 * Unit Test Framework
 */

# define utTest(cond) ut_Test(cond, #cond, __FILE__, __LINE__)

header_function
void ut_Test (B32 cond,
              Char *cond_str,
              Char *filename, U32 line_num) {
    if (!(cond)) {
        report("Test Failed: (%s:%u) %s\n", filename, line_num, cond_str);
        quit();
    }
}

#define NLIB_DEBUG_H_INCLUDE_GUARD
#endif
