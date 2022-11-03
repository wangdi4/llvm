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
#ifndef __TALX_H__
#define __TALX_H__
#include <stdarg.h>
#ifdef __cplusplus

#pragma warning(push, 4)
#pragma warning(                                                               \
    disable : 4127) // warning C4127: conditional expression is constant

/* ****************************************************************************
** TALX Logging
*******************************************************************************

**************************************************************************** */
#if defined(TAL_DISABLE)
#ifndef TAL_DOXYGEN
#include "tal.h"
#endif // ndef TAL_DOXYGEN
// --------------------------------------------------------------------
// TAL macros
// --------------------------------------------------------------------
#define TALX_TRACE(lvl, cat)
#define TALX_TRACE_FN(lvl, cat, fn)
#define TALX_TRACE_NAMED(lvl, cat, nm)

#define TALX_TRACE_WITH_PMU(lvl, cat)
#define TALX_TRACE_FN_WITH_PMU(lvl, cat, fn)
#define TALX_TRACE_NAMED_WITH_PMU(lvl, cat, nm)

#define TAL_SCOPED_TASK()
#define TAL_SCOPED_TASK_FN(fn)
#define TAL_SCOPED_TASK_NAMED(nm)
#define TAL_SCOPED_TASK_WITH_ID(id)
#define TAL_SCOPED_TASK_FN_WITH_ID(fn, id)
#define TAL_SCOPED_TASK_NAMED_WITH_ID(nm, id)
#define TAL_SCOPED_TASK_WITH_RELATION(rel, id)
#define TAL_SCOPED_TASK_FN_WITH_RELATION(fn, rel, id)
#define TAL_SCOPED_TASK_NAMED_WITH_RELATION(nm, rel, id)
#define TAL_SCOPED_TASK_FILTERED(lvl, cat)
#define TAL_SCOPED_TASK_FN_FILTERED(lvl, cat, nm)
#define TAL_SCOPED_TASK_NAMED_FILTERED(lvl, cat, nm)
#define TAL_SCOPED_TASK_WITH_ID_FILTERED(lvl, cat, id)
#define TAL_SCOPED_TASK_FN_WITH_ID_FILTERED(lvl, cat, fn, id)
#define TAL_SCOPED_TASK_NAMED_WITH_ID_FILTERED(lvl, cat, nm, id)
#define TAL_SCOPED_TASK_WITH_RELATION_FILTERED(lvl, cat, rel, id)
#define TAL_SCOPED_TASK_FN_WITH_RELATION_FILTERED(lvl, cat, fn, rel, id)
#define TAL_SCOPED_TASK_NAMED_WITH_RELATION_FILTERED(lvl, cat, nm, rel, id)

#define TAL_SCOPED_TASK_WITH_PMU(lvl, cat)
#define TAL_SCOPED_TASK_FN_WITH_PMU(lvl, cat, fn)
#define TAL_SCOPED_TASK_NAMED_WITH_PMU(lvl, cat, nm)

#else // ! defined(TAL_DISABLE)
#ifndef TAL_DOXYGEN
#include "tal.h"
#endif // ndef TAL_DOXYGEN
// --------------------------------------------------------------------
// TAL macros
// --------------------------------------------------------------------

#ifndef TAL_DOXYGEN
template <bool SAMPLE_PMU>
inline void TAL_SamplePmus(TAL_TRACE *pTrace, TAL_LOG_LEVEL in_lvl,
                           TAL_UINT64 in_cat) {
  TAL_UNUSED(pTrace);
  TAL_UNUSED(in_lvl);
  TAL_UNUSED(in_cat);
}

template <>
inline void TAL_SamplePmus<true>(TAL_TRACE *pTrace, TAL_LOG_LEVEL in_lvl,
                                 TAL_UINT64 in_cat) {
  TAL_SampleHardwareCounters(pTrace, in_lvl, in_cat);
}

template <bool SAMPLE_PMU> class TAL_ScopedTask {
public:
  TAL_LOG_LEVEL lvl;
  TAL_UINT64 cat;
  TAL_TRACE *pTrace;

  // plain
  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        void (*in_fn)())
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginTask(pTrace, in_lvl, in_cat, in_fn);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        const char *in_name)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginNamedTask(pTrace, in_lvl, in_cat, in_name);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        TAL_STRING_HANDLE in_handle)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginNamedTaskH(pTrace, in_lvl, in_cat, in_handle);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        void (*in_fn)(), TAL_ID &in_ThisId)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginTaskWithID(pTrace, in_lvl, in_cat, in_fn, in_ThisId);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        const char *in_name, TAL_ID &in_ThisId)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginNamedTaskWithID(pTrace, in_lvl, in_cat, in_name, in_ThisId);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        TAL_STRING_HANDLE in_handle, TAL_ID &in_ThisId)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginNamedTaskHWithID(pTrace, in_lvl, in_cat, in_handle, in_ThisId);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        void (*in_fn)(), TAL_RELATION in_Relation,
                        TAL_ID &in_TargetId)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginTask(pTrace, in_lvl, in_cat, in_fn);
    TAL_AddRelationThis(in_lvl, in_cat, in_Relation, in_TargetId);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        const char *in_name, TAL_RELATION in_Relation,
                        TAL_ID &in_TargetId)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginNamedTask(pTrace, in_lvl, in_cat, in_name);
    TAL_AddRelationThis(in_lvl, in_cat, in_Relation, in_TargetId);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline TAL_ScopedTask(TAL_LOG_LEVEL in_lvl, TAL_UINT64 in_cat,
                        TAL_STRING_HANDLE in_handle, TAL_RELATION in_Relation,
                        TAL_ID &in_TargetId)
      : lvl(in_lvl), cat(in_cat) {
    pTrace = TAL_GetThreadTrace();
    TAL_BeginNamedTaskH(pTrace, in_lvl, in_cat, in_handle);
    TAL_AddRelationThis(in_lvl, in_cat, in_Relation, in_TargetId);
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, in_lvl, in_cat);
  };

  inline ~TAL_ScopedTask() {
    TAL_SamplePmus<SAMPLE_PMU>(pTrace, lvl, cat);
    TAL_EndTask(pTrace, lvl, cat);
  };

  inline TAL_TRACE *operator*() { return pTrace; };
};

#endif // defined(TAL_DOXYGEN)

/** \addtogroup TALX
 ** In addition to its C API, TAL provide a TAL_SCOPED_TASK family of macros
 *that leverage C++ tricks
 ** to simplfiy instrumentation of your code. These macros
 ** use a scoped class to automatically begin and end a task. They come in
 *several variants:
 ** - Fully automatic, TAL_SCOPED_TASK, using the __FUNCTION__ preprocessor
 *macro for the task name
 ** - By function pointer, TAL_SCOPED_TASK_FN, using a passed-in function
 *pointer
 ** - By string, TAL_SCOPED_TASK_NAMED, using a passed-in const char*
 **
 ** The TAL_SCOPED_TASK and TAL_SCOPED_TASK_NAMED macros
 ** automatically use string pools to keep the cost of the tracing function low.
 **
 ** These same macros are available in a TAL_SCOPED_TASK_WITH_PMU family of
 *variants.
 ** Available across the automatic, string and function pointer base variants,
 *the
 ** WITH_PMU family of macros will issue a TAL_SampleHardwareCounters() at the
 *beginning
 ** and end of each task.
 **
 ** Put together, these macros dramatically simplify your code. What might
 *originally be:
 ** \code
 **   void MyFunction() {
 **     static TAL_STRING_HANDLE hString = TAL_AddStringToPool("MyFunction");
 **     TAL_TRACE* trace = TAL_GetThreadTrace();
 **     TAL_BeginNamedTask(trace,TAL_LOG_LVL_1,TAL_LOG_CAT_1,hString);
 **     TAL_SampleHardwareCounters(trace,TAL_LOG_LVL_1,TAL_LOG_CAT_1);
 **
 **     ... // your code here...
 **
 **     TAL_SampleHardwareCounters(trace,TAL_LOG_LVL_1,TAL_LOG_CAT_1);
 **     TAL_EndTask(trace,TAL_LOG_LVL_1,TAL_LOG_CAT_1,);
 **   }
 ** \endcode
 ** can be expressed instead as:
 ** \code
 **   void MyFunction() {
 **      TAL_ScopedTask(TAL_LOG_LVL_1,TAL_LOG_CAT_1);
 **
 **     ... // your code here...
 **
 **   }
 ** \endcode
 **
 ** The specific macros provided are:
 ** -# TALX_SCOPED_TASK(): creates a task using the __FUNCTION__ preprocessor
 *variable.
 **   -# TALX_SCOPED_TASK_WITH_ID(): creates a task as above, but also
 *associates a TAL_ID with it.
 **   -# TALX_SCOPED_TASK_WITH_RELATION(): creates a task as above, and uses
 *TAL_AddRelationToCurrent to tie this task to to a related task.
 **   -# TALX_SCOPED_TASK_FILTERED(lvl,cat): creates a task above, but only if
 *the specified lvl and cat test passes
 **   -# TALX_SCOPED_TASK_WITH_ID_FILTERED(): behaves as the _WITH_ID variant,
 *but with filtering.
 **   -# TALX_SCOPED_TASK_WITH_RELATION_FILTERED(): behaves as the
 *_WITH_RELATION variant, but with filtering.
 ** -# TALX_SCOPED_TASK_NAMED(name): Creates a task using the given name. You
 *must pass a statically constant string, not a runtime string, as the macro
 *will pool the string the first time it is called.
 **   -# As above, variants are provided for _WITH_ID, _RELATION, _FILTERED, and
 *so on.
 ** -# TALX_SCOPED_TASK_FN(fn): Creates a task based on the passed-in function
 *pointer. You can safely cast the function pointer to (void*)() for this macro
 *to work.
 **   -# As above, variants are provided for _WITH_ID, _RELATION, _FILTERED, and
 *so on.
 **/

/** \ingroup TALX
** Automatically begin and end a task for the current __FUNCTION__.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** This macro will use the __FUNCTION__ preprocessor macro as the task name.
**/
#define TAL_SCOPED_TASK()                                                      \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(__FUNCTION__);                                       \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          __tal_scoped_task_string_handle)

/** \ingroup TALX
** Automatically begin and end a task for the specified function pointer.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginTask and TAL_EndTask function pair. This macro will use the
*function pointer
** specified as the task name.
** \param fn  The function pointer to associate with this task.
**/
#define TAL_SCOPED_TASK_FN(fn)                                                 \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          (void (*)())fn)

/** \ingroup TALX
** Automatically begin and end a task with the specified name.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** \param nm  The name of the task to create. Must be a statically string across
*invocations --- TAL will not check the string pointer after the 1st invocation
*of this macro.
**/
#define TAL_SCOPED_TASK_NAMED(nm)                                              \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(nm);                                                 \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          __tal_scoped_task_string_handle)

/** \ingroup TALX
 ** Automatically begin and end a task for the current __FUNCTION__.
 ** This macro will instantiate a class within the containing scope that will
 *automatically
 ** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
 *automatically use string handles,
 ** keeping the cost of this function extremely low at runtime.
 ** This macro will use the __FUNCTION__ preprocessor macro as the task name.
 ** \param id TAL_ID to associate with this task
 **/
#define TAL_SCOPED_TASK_WITH_ID(id)                                            \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(__FUNCTION__);                                       \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          __tal_scoped_task_string_handle, id)

/** \ingroup TALX
** Automatically begin and end a task for the specified function pointer.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginTask and TAL_EndTask function pair. This macro will use the
*function pointer
** specified as the task name.
** \param fn  The function pointer to associate with this task.
** \param id TAL_ID to associate with this task
**/
#define TAL_SCOPED_TASK_FN_WITH_ID(fn, id)                                     \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          (void (*)())fn, id)

/** \ingroup TALX
** Automatically begin and end a task with the specified name.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** \param nm  The name of the task to create. Must be a statically string across
*invocations --- TAL will not check the string pointer after the 1st invocation
*of this macro.
** \param id TAL_ID to associate with this task
**/
#define TAL_SCOPED_TASK_NAMED_WITH_ID(nm, id)                                  \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(nm);                                                 \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          __tal_scoped_task_string_handle, id)

/** \ingroup TALX
** Automatically begin and end a task for the current __FUNCTION__.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** This macro will use the __FUNCTION__ preprocessor macro as the task name.
** \param rel One of the TAL_RELATION values determining the relation to the
*target id.
** \param id  The id that is the target of the relations.
**/
#define TAL_SCOPED_TASK_WITH_RELATION(rel, id)                                 \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(__FUNCTION__);                                       \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          __tal_scoped_task_string_handle,     \
                                          rel, id)

/** \ingroup TALX
** Automatically begin and end a task for the specified function pointer.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginTask and TAL_EndTask function pair. This macro will use the
*function pointer
** specified as the task name.
** \param fn  The function pointer to associate with this task.
** \param rel One of the TAL_RELATION values determining the relation to the
*target id.
** \param id  The id that is the target of the relations.
**/
#define TAL_SCOPED_TASK_FN_WITH_RELATION(fn, rel, id)                          \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          (void (*)())fn, rel, id)

/** \ingroup TALX
** Automatically begin and end a task with the specified name.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** \param nm  The name of the task to create. Must be a statically string across
*invocations --- TAL will not check the string pointer after the 1st invocation
*of this macro.
** \param rel One of the TAL_RELATION values determining the relation to the
*target id.
** \param id  The id that is the target of the relations.
**/
#define TAL_SCOPED_TASK_NAMED_WITH_RELATION(nm, rel, id)                       \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(nm);                                                 \
  TAL_ScopedTask<false> __tal_scoped_task(TAL_LOG_LEVEL_1, TAL_LOG_CAT_ALL,    \
                                          __tal_scoped_task_string_handle,     \
                                          rel, id)

/** \ingroup TALX
** Automatically begin and end a task for the current __FUNCTION__.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** This macro will use the __FUNCTION__ preprocessor macro as the task name.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
**/
#define TAL_SCOPED_TASK_FILTERED(lvl, cat)                                     \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(__FUNCTION__);                                       \
  TAL_ScopedTask<false> __tal_scoped_task(lvl, cat,                            \
                                          __tal_scoped_task_string_handle)

/** \ingroup TALX
** Automatically begin and end a task for the specified function pointer.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginTask and TAL_EndTask function pair. This macro will use the
*function pointer
** specified as the task name.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
** \param fn  The function pointer to associate with this task.
**/
#define TAL_SCOPED_TASK_FN_FILTERED(lvl, cat, fn)                              \
  TAL_ScopedTask<false> __tal_scoped_task(lvl, cat, (void (*)())fn)

/** \ingroup TALX
** Automatically begin and end a task with the specified name.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
** \param nm  The name of the task to create. Must be a statically string across
*invocations --- TAL will not check the string pointer after the 1st invocation
*of this macro.
**/
#define TAL_SCOPED_TASK_NAMED_FILTERED(lvl, cat, nm)                           \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(nm);                                                 \
  TAL_ScopedTask<false> __tal_scoped_task(lvl, cat,                            \
                                          __tal_scoped_task_string_handle)

/** \ingroup TALX
 ** Automatically begin and end a task for the current __FUNCTION__.
 ** This macro will instantiate a class within the containing scope that will
 *automatically
 ** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
 *automatically use string handles,
 ** keeping the cost of this function extremely low at runtime.
 ** This macro will use the __FUNCTION__ preprocessor macro as the task name.
 ** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
 *loglevels for which function is traced.
 ** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
 *the logging categories for which this function provides data.
 ** \param id TAL_ID to associate with this task
 **/
#define TAL_SCOPED_TASK_WITH_ID_FILTERED(lvl, cat, id)                         \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(__FUNCTION__);                                       \
  TAL_ScopedTask<false> __tal_scoped_task(lvl, cat,                            \
                                          __tal_scoped_task_string_handle, id)

/** \ingroup TALX
** Automatically begin and end a task for the specified function pointer.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginTask and TAL_EndTask function pair. This macro will use the
*function pointer
** specified as the task name.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
** \param fn  The function pointer to associate with this task.
** \param id TAL_ID to associate with this task
**/
#define TAL_SCOPED_TASK_FN_WITH_ID_FILTERED(lvl, cat, fn, id)                  \
  TAL_ScopedTask<false> __tal_scoped_task(lvl, cat, (void (*)())fn, id)

/** \ingroup TALX
** Automatically begin and end a task with the specified name.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
** \param nm  The name of the task to create. Must be a statically string across
*invocations --- TAL will not check the string pointer after the 1st invocation
*of this macro.
** \param id TAL_ID to associate with this task
**/
#define TAL_SCOPED_TASK_NAMED_WITH_ID_FILTERED(lvl, cat, nm, id)               \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(nm);                                                 \
  TAL_ScopedTask<false> __tal_scoped_task(lvl, cat,                            \
                                          __tal_scoped_task_string_handle, id)

/** \ingroup TALX
** Automatically begin and end a task for the current __FUNCTION__.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** This macro will use the __FUNCTION__ preprocessor macro as the task name.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
** \param rel One of the TAL_RELATION values determining the relation to the
*target id.
** \param id  The id that is the target of the relations.
**/
#define TAL_SCOPED_TASK_WITH_RELATION_FILTERED(lvl, cat, rel, id)              \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(__FUNCTION__);                                       \
  TAL_ScopedTask<false> __tal_scoped_task(                                     \
      lvl, cat, __tal_scoped_task_string_handle, rel, id)

/** \ingroup TALX
** Automatically begin and end a task for the specified function pointer.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginTask and TAL_EndTask function pair. This macro will use the
*function pointer
** specified as the task name.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
** \param fn  The function pointer to associate with this task.
** \param rel One of the TAL_RELATION values determining the relation to the
*target id.
** \param id  The id that is the target of the relations.
**/
#define TAL_SCOPED_TASK_FN_WITH_RELATION_FILTERED(lvl, cat, fn, rel, id)       \
  TAL_ScopedTask<false> __tal_scoped_task(lvl, cat, (void (*)())fn, rel, id)

/** \ingroup TALX
** Automatically begin and end a task with the specified name.
** This macro will instantiate a class within the containing scope that will
*automatically
** issue a TAL_BeginNamedTaskH and TAL_EndTask function pair. It will
*automatically use string handles,
** keeping the cost of this function extremely low at runtime.
** \param lvl A value from the TAL_LOG_LEVEL enumeration specifying the
*loglevels for which function is traced.
** \param cat One or a combination of the TAL_LOG_CATEGORIES values determining
*the logging categories for which this function provides data.
** \param nm  The name of the task to create. Must be a statically string across
*invocations --- TAL will not check the string pointer after the 1st invocation
*of this macro.
** \param rel One of the TAL_RELATION values determining the relation to the
*target id.
** \param id  The id that is the target of the relations.
**/
#define TAL_SCOPED_TASK_NAMED_WITH_RELATION_FILTERED(lvl, cat, nm, rel, id)    \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(nm);                                                 \
  TAL_ScopedTask<false> __tal_scoped_task(                                     \
      lvl, cat, __tal_scoped_task_string_handle, rel, id)

#define TAL_SCOPED_TASK_WITH_PMU(lvl, cat)                                     \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(__FUNCTION__);                                       \
  TAL_ScopedTask<true> __tal_scoped_task(lvl, cat,                             \
                                         __tal_scoped_task_string_handle)
#define TAL_SCOPED_TASK_FN_WITH_PMU(lvl, cat, fn)                              \
  TAL_ScopedTask<true> __tal_scoped_task(lvl, cat, (void (*)())fn)
#define TAL_SCOPED_TASK_NAMED_WITH_PMU(lvl, cat, nm)                           \
  static TAL_STRING_HANDLE __tal_scoped_task_string_handle =                   \
      TAL_GetStringHandle(nm);                                                 \
  TAL_ScopedTask<true> __tal_scoped_task(lvl, cat,                             \
                                         __tal_scoped_task_string_handle)
// Back compat
#define TALX_TRACE(lvl, cat) TAL_SCOPED_TASK_FILTERED(lvl, cat)
#define TALX_TRACE_FN(lvl, cat, fn) TAL_SCOPED_TASK_FN_FILTERED(lvl, cat, fn)
#define TALX_TRACE_NAMED(lvl, cat, nm)                                         \
  TAL_SCOPED_TASK_NAMED_FILTERED(lvl, cat, nm)
#define TALX_TRACE_WITH_PMU(lvl, cat) TAL_SCOPED_TASK_WITH_PMU(lvl, cat)
#define TALX_TRACE_FN_WITH_PMU(lvl, cat, fn)                                   \
  TAL_SCOPED_TASK_FN_WITH_PMU(lvl, cat, fn)
#define TALX_TRACE_NAMED_WITH_PMU(lvl, cat, nm)                                \
  TAL_SCOPED_TASK_NAMED_WITH_PMU(lvl, cat, nm)

/** \ingroup TALX
 ** Obtains a pointer to the scoped class that is used to automatically issue
 *the TAL_BeginTask and TAL_EndTask calls.
 **
 ***/
#define TAL_GET_SCOPED_TRACE() (*talx_tracer)
#endif // defined(TAL_DISABLE)

#pragma warning(pop)

#endif // __cplusplus
#endif // __TALX_H__

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
