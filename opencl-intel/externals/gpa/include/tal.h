/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) 2010, Intel Corporation. All rights reserved.
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
*LICENSED
** ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, ASSISTANCE,
** INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT PROVIDE ANY
*UPDATES,
** ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY DISCLAIMS ANY WARRANTY OF
** MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY PARTICULAR PURPOSE, OR ANY
** OTHER WARRANTY.  Intel disclaims all liability, including liability for
** infringement of any proprietary rights, relating to use of the code. No
*license,
** express or implied, by estoppel or otherwise, to any intellectual property
** rights is granted herein.
**
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma warning(push, 4)
#pragma warning(disable : 4555) // expression has no effect; expected expression
                                // with side-effect
#pragma warning(disable : 4311) // 'type cast' : pointer truncation from 'void
                                // *' to 'TAL_UINT32'
#pragma warning(                                                               \
    disable : 4996) // warning C4996: 'function': was declared deprecated
#pragma warning(                                                               \
    disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4995) // warning C4995: 'strcpy': name was marked as
                                // #pragma deprecated

#ifdef TAL_TYPES_ONLY
#include "tal_structs.h"
#else
#ifndef TAL_H_WITH_API
#define TAL_H_WITH_API

#include "tal_api.h"
#include "tal_ctl.h"
#include "tal_lowlevel_api.h"
#include "tal_macros.h"
#include "tal_structs.h"

#ifndef TAL_DOXYGEN

#if TAL_PLATFORM == TAL_PLATFORM_WINDOWS
#include <malloc.h>
#if defined(TAL_KERNEL)

#endif
#else
#ifndef TAL_KERNEL
#include <stdlib.h>
#else
#include <sys/libkern.h>
#endif // defined(TAL_KERNEL)
#endif

#if !defined(TAL_KERNEL)
#include <assert.h>
#include <string.h>
#endif // !defined(TAL_KERNEL)
#endif // TAL_DOXYGEN

#define TAL_INCLUDE_PUSH_API

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS

#if defined(TAL_FEATURE_STATIC_PUSH_FUNCS)

// Create function imples
#ifdef __cplusplus
#define TAL_FUNCTION(r, x, p) extern "C" r(TAL_CALL *__p##x##_Pusher) p;
#else
#define TAL_FUNCTION(r, x, p) extern r(TAL_CALL *__p##x##_Pusher) p;
#endif //
#define TAL_FUNCTION_ARGS TAL_TRACE *t, TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat
#define TAL_FUNCTION_ARGS_ TAL_FUNCTION_ARGS,
#define UNUSED_TAL_FUNCTION_ARGS
#define TAL_FUNCTION_ARG_LVL
#define TAL_FUNCTION_ARG_CAT
#define TAL_FUNCTION_ARG_TRACE
#define TAL_FUNCTION_BODY(...)
#include "tal_push_api.h"

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS

// pusher Function Structure
typedef struct {
#define TAL_FUNCTION(r, x, params) r(TAL_CALL *p##x##_Pusher) params;
#define TAL_FUNCTION_ARGS TAL_TRACE *t, TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat
#define TAL_FUNCTION_ARGS_ TAL_FUNCTION_ARGS,
#define UNUSED_TAL_FUNCTION_ARGS
#define TAL_FUNCTION_ARG_LVL
#define TAL_FUNCTION_ARG_CAT
#define TAL_FUNCTION_ARG_TRACE
#define TAL_FUNCTION_BODY(...)
#include "tal_push_api.h"

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_BODY
  // int last;
} TAL_PUSHER_FUNCTIONS;
#ifdef TAL_CALL_STATIC_PUSHER

#define TAL_FUNCTION(r, x, p) extern "C" r TAL_CALL x##_Pusher p;
#define TAL_FUNCTION_ARGS TAL_TRACE *t, TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat
#define TAL_FUNCTION_ARGS_ TAL_FUNCTION_ARGS,
#define UNUSED_TAL_FUNCTION_ARGS
#define TAL_FUNCTION_BODY(...)
#include "tal_push_api.h"

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_BODY

#define TAL_FUNCTION_BODY(fname, ...)                                          \
  { fname##_Pusher(__VA_ARGS__); }
#else // TAL_CALL_STATIC_PUSHER
#ifdef TAL_DISABLE
#define TAL_FUNCTION_BODY(fname, ...)                                          \
  { UNUSED_TAL_FUNCTION_ARGS(__VA_ARGS__); };
#else // TAL_DISABLE
#define TAL_FUNCTION_BODY(fname, ...)                                          \
  {                                                                            \
    if (__p##fname##_Pusher) {                                                 \
      __p##fname##_Pusher(__VA_ARGS__);                                        \
    }                                                                          \
  }
#endif // TAL_DISABLE
#endif // TAL_CALL_STATIC_PUSHER
#define TAL_FUNCTION_PARAMS                                                    \
  TAL_FUNCTION_ARG_TRACE, TAL_FUNCTION_ARG_LVL, TAL_LOG_CAT_ALL
#define TAL_FUNCTION_PARAMS_ TAL_FUNCTION_PARAMS,
#endif // TAL_FEATURE_STATIC_PUSH_FUNCS

#define TAL_FUNCTION(r, x, p) TAL_INLINE r TAL_CALL x p
#define TAL_FUNCTION_ARGS TAL_TRACE *t
#define TAL_FUNCTION_ARGS_ TAL_FUNCTION_ARGS,
#define UNUSED_TAL_FUNCTION_ARGS TAL_UNUSED(t);
#define TAL_FUNCTION_ARG_LVL TAL_LOG_LEVEL_1
#define TAL_FUNCTION_ARG_CAT TAL_LOG_CAT_ALL
#define TAL_FUNCTION_ARG_TRACE t
#include "tal_push_api.h"

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_BODY

#define TAL_FUNCTION(r, x, p) TAL_INLINE r TAL_CALL x##Filtered p
#define TAL_FUNCTION_ARGS TAL_TRACE *t, TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat
#define TAL_FUNCTION_ARGS_ TAL_FUNCTION_ARGS,
#define UNUSED_TAL_FUNCTION_ARGS                                               \
  TAL_UNUSED(t);                                                               \
  TAL_UNUSED(in_lvl);                                                          \
  TAL_UNUSED(in_cat);
#define TAL_FUNCTION_ARG_LVL in_lvl
#define TAL_FUNCTION_ARG_CAT in_cat
#define TAL_FUNCTION_ARG_TRACE t
#include "tal_push_api.h"

#ifdef __cplusplus
#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_BODY

#define TAL_FUNCTION(r, x, p) TAL_INLINE r TAL_CALL x p
#define TAL_FUNCTION_ARGS
#define TAL_FUNCTION_ARGS_
#define UNUSED_TAL_FUNCTION_ARGS
#define TAL_FUNCTION_ARG_LVL TAL_LOG_LEVEL_1
#define TAL_FUNCTION_ARG_CAT TAL_LOG_CAT_ALL
#define TAL_FUNCTION_ARG_TRACE TAL_GetThreadTrace()
#include "tal_push_api.h"

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_BODY

#define TAL_FUNCTION(r, x, p) TAL_INLINE r TAL_CALL x p
#define TAL_FUNCTION_ARGS TAL_TRACE *t, TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat
#define TAL_FUNCTION_ARGS_ TAL_FUNCTION_ARGS,
#define UNUSED_TAL_FUNCTION_ARGS                                               \
  TAL_UNUSED(t);                                                               \
  TAL_UNUSED(in_lvl);                                                          \
  TAL_UNUSED(in_cat);
#define TAL_FUNCTION_ARG_LVL in_lvl
#define TAL_FUNCTION_ARG_CAT in_cat
#define TAL_FUNCTION_ARG_TRACE t
#include "tal_push_api.h"

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_BODY

#define TAL_FUNCTION(r, x, p) TAL_INLINE r TAL_CALL x p
#define TAL_FUNCTION_ARGS TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat
#define TAL_FUNCTION_ARGS_ TAL_FUNCTION_ARGS,
#define UNUSED_TAL_FUNCTION_ARGS                                               \
  in_lvl;                                                                      \
  in_cat;
#define TAL_FUNCTION_ARG_LVL in_lvl
#define TAL_FUNCTION_ARG_CAT in_cat
#define TAL_FUNCTION_ARG_TRACE TAL_GetThreadTrace()
#include "tal_push_api.h"

#undef TAL_FUNCTION
#undef TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_ARGS_
#undef TAL_FUNCTION_ARG_LVL
#undef TAL_FUNCTION_ARG_CAT
#undef TAL_FUNCTION_ARG_TRACE
#undef UNUSED_TAL_FUNCTION_ARGS
#undef TAL_FUNCTION_BODY

#endif // !__cplusplus
#undef TAL_INCLUDE_PUSH_API

#if defined(TAL_FEATURE_STATIC_PUSH_FUNCS)
#undef TAL_STATIC_PUSHER_NAME
#endif // defined(TAL_FEATURE_STATIC_PUSH_FUNCS)

// Depricated functions
#if TAL_COMPILER == TAL_COMPILER_MSVC || TAL_COMPILER == TAL_COMPILER_ICC
#pragma deprecated(TAL_Heartbeat)
#pragma deprecated(TAL_ThreadInit)
#pragma deprecated(TAL_ThreadInitEx)
#pragma deprecated(TAL_ThreadCleanup)
#pragma deprecated(TAL_SetCaptureMode)
#pragma deprecated(TAL_BeginCapture)
#pragma deprecated(TAL_BeginCaptureToFile)
#pragma deprecated(TAL_BeginCaptureToNetwork)
#pragma deprecated(TAL_EndCapture)
#pragma deprecated(TAL_EndCaptureToFile)

#pragma deprecated(TAL_AddStringToPool)
#pragma deprecated(TAL_AddTaskRelation)
#pragma deprecated(TAL_AddTaskRelations)
#pragma deprecated(TAL_AddRelationToCurrent)

#pragma deprecated(TAL_RELATION_DEPENDENCY)
#pragma deprecated(TAL_RELATION_SIBLING)
#pragma deprecated(TAL_RELATION_PARENT)
#pragma deprecated(TAL_RELATION_CONTINUATION)
#pragma deprecated(TAL_RELATION_CHILD)
#pragma deprecated(TAL_RELATION_PREDECESSOR)
#pragma deprecated(TAL_RELATION_OUT_DEPENDENCY)

#pragma deprecated(TAL_RegisterDistiller)
#pragma deprecated(TAL_UnregisterDistiller)

#endif // MSVC/ICC only guard
#include "talx.h"
#endif // TAL_H_WITH_API // include guard on regular include of <tal.h>

#endif // overall #if TAL_TYPES_ONLY that splits out header based on TAL_NO_API
       // or not...

#pragma warning(pop)

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
