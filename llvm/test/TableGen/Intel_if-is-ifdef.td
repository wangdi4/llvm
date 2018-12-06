// RUN: llvm-tblgen -I %p %s 2>&1 | FileCheck %s --implicit-check-not warning:
// The test is equivalent to prep-region-processing.td, except that
// #ifdef's are replaced with #if

class C;

// TableGen prints records in alpabetical order.
// CHECK-NOT: def ifdef_disabled1
// CHECK-NOT: def ifdef_disabled2
// CHECK:     def ifdef_disabled3
// CHECK-NOT: def ifdef_disabled4
// CHECK-NOT: def ifdef_disabled5
// CHECK:     def ifdef_disabled4_else
// CHECK-NOT: def ifdef_disabled5_else
// CHECK:     def ifdef_enabled1
// CHECK-NOT: def ifdef_enabled2
// CHECK:     def ifdef_enabled3
// CHECK:     def ifdef_enabled4
// CHECK-NOT: def ifdef_enabled4_else
// CHECK:     def ifdef_enabled5
// CHECK:     def ifdef_enabled6
// CHECK-NOT: def ifdef_enabled6_else
// CHECK-NOT: def ifdef_disabled6
// CHECK-NOT: def ifdef_disabled6_else

#define ENABLED1
#define ENABLED2

#if DISABLED1
//
def ifdef_disabled1 : C;

#define DISABLED2/*This one is disabled,
                   because DISABLED1 is.
*/
#endif

#if ENABLED1
def ifdef_enabled1 : C;
#endif

#if DISABLED2/*
*/
def ifdef_disabled2 : C;
#endif

/*
#if ENABLED2
def ifdef_enabled2 : C;
#endif
*/

//#if DISABLED3
def ifdef_disabled3 : C;

//#endif

/* _DISABLED4 */  /* padding */ #if _DISABLED4
def ifdef_disabled4 : C;
#else//      /*!_DISABLED4
def ifdef_disabled4_else : C;

#define ENABLED3
#endif

#if __DISABLED5
def ifdef_disabled5 : C;
/*

*/#else
#if ENABLED3
def ifdef_enabled3 : C;
#else /* //!ENABLED3
*/
def ifdef_disabled5_else : C;
#endif
#endif

#define ENABLED4
include "prep-region-include.inc"//ENABLED5 is defined inside

#if ENABLED5
def ifdef_enabled5 : C;
#endif // ENABLED5

#if DISABLED6__
// Double inclusion is an error.
include "prep-region-include.inc"
#else
#endif

#if DIS
#if DIS
#if DIS
#if DIS
#if DIS
#if DIS
#if DIS
#if DIS
#if DIS
#if DIS
#if DIS
def ifdef_disabled6 : C;
#endif
#endif
#endif
#endif
#else
def ifdef_disabled6_else : C;
#endif
#endif
#endif
#endif
#endif
#endif
#else
#define ENAB//ENAB
#endif

#if ENAB
#if ENAB
#if ENAB
#if ENAB
#if ENAB
#if ENAB
#if ENAB
#if ENAB
#if ENAB
#if ENAB
#if ENAB
def ifdef_enabled6 : C;
#endif
#endif
#endif
#endif
#else
def ifdef_enabled6_else : C;
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#if DISABLED_7
include "non-existent-file.inc"
#endif

#if DISABLED_8
\\\\\ invalid TD text /////
#endif // DISABLED_8
