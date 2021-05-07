#ifndef __CSL_H__
#define __CSL_H__

#define TCC 0
#if TCC
const void *const __dso_handle __attribute__ ((__visibility__ ("hidden")))
  = &__dso_handle;
#endif
#define LINUX 1

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdarg.h>

#if LINUX
#include <termios.h>
//#include <ncurses.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <asm/unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#if 0
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
//#include "/usr/include/x86_64-linux-gnu/asm/unistd.h"
//#include <arch/ia64/asm/thread_info.h>
#endif
#endif
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <errno.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>

#include <udis86.h>
#include <gmp.h>
#include <mpfr.h>
#include "defines.h"
#include "types.h"
#include "macros.h"

//#include "codegen_x86.h" // i want to make sure i have this - not using much now but probably later on
#include "lisp.h"
#include "machineCode.h"
#include "machineCodeMacros.h"
#include "prototypes.h"
#include "lc.h"

extern OVT_StaticMemSystem *_OSMS_ ;
extern OVT_Static * _OS_ ;
extern OpenVmTil * _O_;
extern struct termios SavedTerminalAttributes ;
extern CPrimitive CPrimitives [];
extern MachineCodePrimitive MachineCodePrimitives [];
extern value_t FL_NIL, T, FL_LAMBDA, FL_MACRO, FL_LABEL, FL_QUOTE ;
extern Boolean cli ;
extern value_t lv ; // last value
extern Boolean lf ;
#endif
