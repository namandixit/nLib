/*
 * Creator: Naman Dixit
 * Notice: © Copyright 2020 Naman Dixit
 */

#if !defined(NLIB_DEBUG_H_INCLUDE_GUARD)

# if defined(OS_WINDOWS)

#  if defined(BUILD_INTERNAL)
#   define breakpoint() __debugbreak()
#   define quit() breakpoint()
#  else
#   define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#   define quit() ExitProcess(0)
#  endif

# elif defined(OS_LINUX)

#  if defined(BUILD_INTERNAL)
#   define breakpoint() __asm__ volatile("int $0x03")
#   define quit() breakpoint()
#  else
#   define breakpoint() do{report("Fired breakpoint in release code, quitting...\n");quit();}while(0)
#   define quit() exit(0)
#  endif

# endif

#define NLIB_DEBUG_H_INCLUDE_GUARD
#endif
