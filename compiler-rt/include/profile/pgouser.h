/**
*** Copyright (C) 2023 Intel Corporation.  All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
**/

/**
*** Provided for use in the instrumentation compilation phase of
*** profile-guided optimizations. Interface to the instrumentation
*** run-time library routines for control from the application
*** being profiled.
**/

#if !defined(_PGO_USER_H)
#define _PGO_USER_H

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined _PGO_INSTRUMENT

/**
*** Direct the profiling module to create raw profile data file(s) containing
*** the data collected since the start of the application run, or the last reset
*** (see _PGOPTI_Prof_Reset_All).
***
*** For applications that involve shared objects, each shared object may have
*** its own memory region for profiling data, which will be emitted.
***
*** This routine may be called from either the main application or from a
*** shared object to dump the data of the main executable and all the shared
*** objects.
**/
extern void _PGOPTI_Prof_Dump_All(void);

/**
*** Direct the profiling module to clear the profiling data collected.
***
*** For applications that involve shared objects, each shared object may have
*** its own memory region for profiling data.
***
*** This routine will reset the data for all the shared objects that have
*** instrumentation, as well as the main application executable.
***
*** This routine may be called from either the main application or from a
*** shared object to clear the data of the main executable and all the shared
*** objects.
**/
extern void _PGOPTI_Prof_Reset_All(void);

#else
/**
*** An empty version of the routines is provided to support building and
*** linking of applications containing the PGO API routines when not
*** performing instrumentation without the need to modify the source
*** calling the routines.
**/
static void _PGOPTI_Prof_Dump_All(void) {}
static void _PGOPTI_Prof_Reset_All(void) {}

#endif /* _PGO_INSTRUMENT */

#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* _PGO_USER_H */