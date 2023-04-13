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

/** @file */

#ifndef TAL_INCLUDE_PUSH_API
#error "tal_push_api.h can only be included from TAL.h"
#endif //! defined(TAL_INCLUDE_PUSH_API)

#if TAL_COMPILER == TAL_COMPILER_MSVC || TAL_COMPILER == TAL_COMPILER_ICC
#pragma warning(push)
#pragma warning(disable : 4996)
#pragma warning(                                                               \
    disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4204) // warning C4204: nonstandard extension used:
                                // non-constant aggregate initializer
#endif // TAL_COMPILER == TAL_COMPILER_MSVC || TAL_COMPILER == TAL_COMPILER_ICC

/**@ingroup BeginTask
 ** Marks the beginning of a task, using a function pointer as its name.
 **
 ** Marks the beginning of a task, using a function pointer as its name.
 ** \param fn Pointer to the task function
 **/
TAL_FUNCTION(void, TAL_BeginTask, (TAL_FUNCTION_ARGS_ void (*fn)(void)))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginTask, TAL_FUNCTION_PARAMS_ fn)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_TASK(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, fn);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginTask
 ** Marks the beginning of a task, using a pointer to a function as its name and
 *an explicit timestamp for its start time.
 **
 ** Marks the beginning of a task, using a pointer to a function as its name and
 *an explicit timestamp for its start time.
 ** \param fn Pointer to the task function
 ** \param ts Timestamp in TAL ticks (see TAL_GetTimestampFreq()) */
TAL_FUNCTION(void, TAL_BeginTaskEx,
             (TAL_FUNCTION_ARGS_ void (*fn)(void), TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginTaskEx, TAL_FUNCTION_PARAMS_ fn, ts)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_TASK_EX(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, fn,
                         ts);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginTask
 ** Marks the beginning of a task with a specified ID, using a function pointer
 *as the task name.
 **
 ** Marks the beginning of a task with a specified ID, using a function pointer
 *as the task name.
 ** For more information on using IDs, plase see the \ref Relations section.
 ** \param fn Pointer to the task function
 ** \param id The ID associated with the task
 **/
TAL_FUNCTION(void, TAL_BeginTaskWithID,
             (TAL_FUNCTION_ARGS_ void (*fn)(void), TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginTaskWithID, TAL_FUNCTION_PARAMS_ fn, id)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_TASK_WITH_ID(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                              fn, id.ns, id.hi, id.lo);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginTask
 ** Marks the beginning of a task with an explicit ID, using a function pointer
 *as its name.
 **
 ** Marks the beginning of a task with an explicit ID, using a function pointer
 *as its name.
 ** For more information on using IDs, plase see the \ref Relations section.
 ** \param fn Pointer to the task function
 ** \param id The ID associated with the task
 ** \param ts Time stamp in TAL_ticks (see TAL_GetTimestampFreq())
 **/
TAL_FUNCTION(void, TAL_BeginTaskWithIDEx,
             (TAL_FUNCTION_ARGS_ void (*fn)(void), TAL_ID id, TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginTaskWithIDEx, TAL_FUNCTION_PARAMS_ fn, id, ts)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_TASK_WITH_ID_EX(trace, TAL_FUNCTION_ARG_LVL,
                                 TAL_FUNCTION_ARG_CAT, fn, id.ns, id.hi, id.lo,
                                 ts);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task, using a string as its name.
 **
 ** Marks the beginning of a task, using a string as its name.
 ** \param name Task name
 **/
TAL_FUNCTION(void, TAL_BeginNamedTask, (TAL_FUNCTION_ARGS_ const char *name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTask, TAL_FUNCTION_PARAMS_ name)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                            name);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task, using a string handle as its name.
 **
 ** Marks the beginning of a task, using a string handle as its name.
 **
 ** \param name Task name, represented by the TAL_STRING_HANDLE returned from
 *TAL_GetStringHandle.
 **/
TAL_FUNCTION(void, TAL_BeginNamedTaskH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTaskH, TAL_FUNCTION_PARAMS_ name)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                              name);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task, using a string as its name and an explicit
 *timestamp for its start time.
 **
 ** Marks the beginning of a task, using a string as its name and an explicit
 *timestamp for its start time.
 ** \param name Task name
 ** \param ts   Time stamp in TAL_ticks (see TAL_GetTimestampFreq())
 **/
TAL_FUNCTION(void, TAL_BeginNamedTaskEx,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTaskEx, TAL_FUNCTION_PARAMS_ name, ts)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK_EX(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT, name, ts);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task, using a string handle as its name and an
 *explicit timestamp for its start time.
 **
 ** Marks the beginning of a task, using a string handle as its name and an
 *explicit timestamp for its start time.
 **
 ** \param name Task name, represented by the TAL_STRING_HANDLE returned from
 *TAL_GetStringHandle.
 ** \param ts   Time stamp in TAL_ticks (see TAL_GetTimestampFreq()).
 **/
TAL_FUNCTION(void, TAL_BeginNamedTaskHEx,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTaskHEx, TAL_FUNCTION_PARAMS_ name, ts)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK_H_EX(trace, TAL_FUNCTION_ARG_LVL,
                                 TAL_FUNCTION_ARG_CAT, name, ts);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task with a specified ID, using a string for the
 *task name.
 **
 ** Marks the beginning of a task with a specified ID, using a string for the
 *task name.
 ** For more information on using IDs, plase see the \ref Relations section.
 **
 ** \param name Task name
 ** \param id   The ID associated with the task
 **/
TAL_FUNCTION(void, TAL_BeginNamedTaskWithID,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTaskWithID, TAL_FUNCTION_PARAMS_ name, id)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK_WITH_ID(trace, TAL_FUNCTION_ARG_LVL,
                                    TAL_FUNCTION_ARG_CAT, name, id.ns, id.hi,
                                    id.lo);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task with a specified ID, using a string handle for
 *the task name.
 **
 ** Marks the beginning of a task with a specified ID, using a string handle for
 *the task name.
 ** For more information on using IDs, plase see the \ref Relations section.
 **
 ** \param name Task name, represented by the TAL_STRING_HANDLE returned from
 *TAL_GetStringHandle
 ** \param id   The ID associated with the task.
 **/
TAL_FUNCTION(void, TAL_BeginNamedTaskHWithID,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTaskHWithID, TAL_FUNCTION_PARAMS_ name, id)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK_H_WITH_ID(trace, TAL_FUNCTION_ARG_LVL,
                                      TAL_FUNCTION_ARG_CAT, name, id.ns, id.hi,
                                      id.lo);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task with a specified ID, using a string for the
 *task name and an explicit timestamp.
 **
 ** Marks the beginning of a task with a specified ID, using a string for the
 *task name and an explicit timestamp.
 ** For more information on using IDs, plase see the \ref Relations section.
 **
 ** \param name Task name.
 ** \param id   The ID associated with the task.
 ** \param ts   Time stamp in TAL_ticks (see TAL_GetTimestampFreq())
 **/
TAL_FUNCTION(void, TAL_BeginNamedTaskWithIDEx,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_ID id, TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTaskWithIDEx, TAL_FUNCTION_PARAMS_ name, id, ts)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK_WITH_ID_EX(trace, TAL_FUNCTION_ARG_LVL,
                                       TAL_FUNCTION_ARG_CAT, name, id.ns, id.hi,
                                       id.lo, ts);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup BeginNamedTask
 ** Marks the beginning of a task with a specified ID, using a string handle for
 *the task name and an explicit timestamp.
 **
 ** Marks the beginning of a task with a specified ID, using a string handle for
 *the task name and an explicit timestamp.
 ** For more information on using IDs, plase see the \ref Relations section.
 **
 ** \param name Task name, represented by the TAL_STRING_HANDLE returned from
 *TAL_GetStringHandle
 ** \param id   The ID associated with the task.
 ** \param ts   Time stamp in TAL_ticks (see TAL_GetTimestampFreq())
 **/
TAL_FUNCTION(void, TAL_BeginNamedTaskHWithIDEx,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_ID id,
              TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedTaskHWithIDEx, TAL_FUNCTION_PARAMS_ name, id,
                  ts)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_TASK_H_WITH_ID_EX(trace, TAL_FUNCTION_ARG_LVL,
                                         TAL_FUNCTION_ARG_CAT, name, id.ns,
                                         id.hi, id.lo, ts);

#ifndef TAL_DISABLE
  p__TAL_CaptureStackBackTrace(trace, TAL_FUNCTION_ARG_LVL,
                               TAL_FUNCTION_ARG_CAT);
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/**@ingroup EndTask
 ** Marks the end of a task begun by any of the TAL_BeginTask or
 *TAL_BeginNamedTask variants.
 **
 ** Marks the end of a task begun by any of the TAL_BeginTask or
 *TAL_BeginNamedTask variants.
 **/
TAL_FUNCTION(void, TAL_EndTask, (TAL_FUNCTION_ARGS))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_EndTask, TAL_FUNCTION_PARAMS)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_END_TASK(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT);

#ifndef TAL_DISABLE
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup EndTask
 ** Marks the end of a task begun by any of the TAL_BeginTask or
 *TAL_BeginNamedTask variants, using an explicit timestamp.
 **
 ** Marks the end of a task begun by any of the TAL_BeginTask or
 *TAL_BeginNamedTask variants, using an explicit timestamp.
 ** \param ts Time stamp in TAL_ticks (see TAL_GetTimestampFreq())
 **/
TAL_FUNCTION(void, TAL_EndTaskEx, (TAL_FUNCTION_ARGS_ TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_EndTaskEx, TAL_FUNCTION_PARAMS_ ts)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_END_TASK_EX(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, ts);

#ifndef TAL_DISABLE
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/**@ingroup VirtualTasks
 ** Marks the beginning of a virtual task, using a string for the vtask's name.
 **
 ** Marks the beginning of a virtual task, using a string for the vtask's name.
 ** For more information on using IDs, plase see the \ref Relations section.
 ** \param name Virtual task name
 ** \param id   The ID associated with the virtual task.
 **/
TAL_FUNCTION(void, TAL_BeginNamedVirtualTaskWithID,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedVirtualTaskWithID, TAL_FUNCTION_PARAMS_ name,
                  id)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_VIRTUAL_TASK_WITH_ID(trace, TAL_FUNCTION_ARG_LVL,
                                            TAL_FUNCTION_ARG_CAT, name, id.ns,
                                            id.hi, id.lo);

#ifndef TAL_DISABLE
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup VirtualTasks
 ** Marks the beginning of a virtual task, using a string handle for the vtask's
 *name.
 **
 ** Marks the beginning of a virtual task, using a string handle for the vtask's
 *name.
 ** For more information on using IDs, plase see the \ref Relations section.
 ** \param name Virtual task name, represented by the TAL_STRING_HANDLE returned
 *from TAL_GetStringHandle
 ** \param id   The ID associated with the virtual task.
 **/
TAL_FUNCTION(void, TAL_BeginNamedVirtualTaskHWithID,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_BeginNamedVirtualTaskHWithID, TAL_FUNCTION_PARAMS_ name,
                  id)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_BEGIN_NAMED_VIRTUAL_TASK_H_WITH_ID(trace, TAL_FUNCTION_ARG_LVL,
                                              TAL_FUNCTION_ARG_CAT, name, id.ns,
                                              id.hi, id.lo);

#ifndef TAL_DISABLE
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/**@ingroup VirtualTasks
 ** Marks the end of a virtual task.
 **
 ** Marks the end of a virtual task.
 **
 ** Note that the end of a virtual task does not determine its duration as
 *reported in the GUI. Rather, the duration of a virtual task
 ** will be determined by the duration of all children of this task, as record
 *by the TAL_AddRelation or TAL_AddRelationThis APIs.
 **/
TAL_FUNCTION(void, TAL_EndVirtualTask, (TAL_FUNCTION_ARGS))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_EndVirtualTask, TAL_FUNCTION_PARAMS)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_END_VIRTUAL_TASK(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT);

#ifndef TAL_DISABLE
  p__TAL_ReadPMUCounters(trace);
#endif // endif TAL_DISABLE
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/************************************************************************/

/************************************************************************/
#ifndef TAL_DOXYGEN
TAL_FUNCTION(void, TAL_AddTaskRelation,
             (TAL_FUNCTION_ARGS_ TAL_RELATION relation, TAL_ID this_task_id,
              TAL_ID that_task_id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddTaskRelation, TAL_FUNCTION_PARAMS_ relation,
                  this_task_id, that_task_id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_RELATION(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                        relation, this_task_id.ns, this_task_id.hi,
                        this_task_id.lo, that_task_id.ns, that_task_id.hi,
                        that_task_id.lo);
}
#endif //! defined(TAL_FUNCTION_BODY)

#endif // ndef TAL_DOXYGEN
/************************************************************************/
#ifndef TAL_DOXYGEN
TAL_FUNCTION(void, TAL_AddTaskRelations,
             (TAL_FUNCTION_ARGS_ TAL_RELATION relation, TAL_ID this_task_id,
              TAL_UINT32 n_those_tasks, TAL_ID *those_task_ids))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddTaskRelations, TAL_FUNCTION_PARAMS_ relation,
                  this_task_id, n_those_tasks, those_task_ids)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_UINT32 i = 0;
  for (i = 0; i < n_those_tasks; ++i) {
    TAL_ID *that_task_id = &those_task_ids[i];
    TAL_PUSH_ADD_RELATION(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                          relation, this_task_id.ns, this_task_id.hi,
                          this_task_id.lo, that_task_id->ns, that_task_id->hi,
                          that_task_id->lo);
  }
}
#endif //! defined(TAL_FUNCTION_BODY)

#endif

/************************************************************************/
/** \ingroup Relations
** Specify a relationship between two tasks or virtual tasks.
**
** Specify a relationship between two tasks or virtual tasks.

** Hint: Read the function call out loud, including the relationship argument,
** to understand the distinction between this and that. E.g. "this is a child of
that task."
**
** \param this_task_id  ID of the 'this' task
** \param relation      Relationship type
** \param that_task_id  ID of the 'that' task.
**/
TAL_FUNCTION(void, TAL_AddRelation,
             (TAL_FUNCTION_ARGS_ TAL_ID this_task_id, TAL_RELATION relation,
              TAL_ID that_task_id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddRelation, TAL_FUNCTION_PARAMS_ this_task_id, relation,
                  that_task_id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_RELATION(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                        relation, this_task_id.ns, this_task_id.hi,
                        this_task_id.lo, that_task_id.ns, that_task_id.hi,
                        that_task_id.lo);
}
#endif //! defined(TAL_FUNCTION_BODY)
#ifndef TAL_DOXYGEN
TAL_FUNCTION(void, TAL_AddRelationToCurrent,
             (TAL_FUNCTION_ARGS_ TAL_RELATION relation, TAL_ID that_task_id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddRelationToCurrent, TAL_FUNCTION_PARAMS_ relation,
                  that_task_id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_RELATION_TO_CURRENT(
      trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, relation,
      that_task_id.ns, that_task_id.hi, that_task_id.lo);
}
#endif //! defined(TAL_FUNCTION_BODY)
#endif

/************************************************************************/
/**\ingroup Relations
 ** Specify a relationship between the current task and another task.
 **
 ** Specify a relationship between the current task and another task.
 ** Thus, e.g. TAL_AddRelationThis(TAL_RELATION_IS_CHILD_OF, that_id) will
 ** make the current task a child of that_id.
 ** \param relation Relationship type
 ** \param that_id  ID of the 'that' task.
 **/
TAL_FUNCTION(void, TAL_AddRelationThis,
             (TAL_FUNCTION_ARGS_ TAL_RELATION relation, TAL_ID that_id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddRelationThis, TAL_FUNCTION_PARAMS_ relation, that_id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_RELATION_THIS(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                             relation, that_id.ns, that_id.hi, that_id.lo);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/** @ingroup Relations
 ** Specify a relationship between one and a number of tasks.
 **
 ** Specify a relationship between one and a number of tasks.
 ** \param this_task_id ID of the source task
 ** \param relation       Relationship type
 ** \param n_those_tasks  Number of 'that' tasks
 ** \param those_task_ids Array of 'that' task IDs
 **/
TAL_FUNCTION(void, TAL_AddRelations,
             (TAL_FUNCTION_ARGS_ TAL_ID this_task_id, TAL_RELATION relation,
              TAL_UINT32 n_those_tasks, TAL_ID *those_task_ids))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddRelations, TAL_FUNCTION_PARAMS_ this_task_id, relation,
                  n_those_tasks, those_task_ids)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_UINT32 i = 0;
  for (i = 0; i < n_those_tasks; ++i) {
    TAL_ID *that_task_id = &those_task_ids[i];
    TAL_PUSH_ADD_RELATION(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                          relation, this_task_id.ns, this_task_id.hi,
                          this_task_id.lo, that_task_id->ns, that_task_id->hi,
                          that_task_id->lo);
  }
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
#ifndef TAL_DOXYGEN
TAL_FUNCTION(void, TAL_TaskCreated, (TAL_FUNCTION_ARGS_ TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_TaskCreated, TAL_FUNCTION_PARAMS_ id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ID_CREATED(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, id.ns,
                      id.hi, id.lo, 0);
}
#endif //! defined(TAL_FUNCTION_BODY)

#endif

/************************************************************************/
/**@ingroup Relations
 ** Capture the creation of an ID.
 **
 ** Capture the creation of an ID. This makes it possible to identify
 ** a task or a vtask, and thus establish relationships with it,
 ** before its execution. It is possible to mark this ID as globally
 ** visible, in which case other processes can use this ID to form
 ** relationships. Local IDs cannot be accessed outside the process
 ** they are created in. In either case, only the creator process can
 ** retire an ID.
 ** \param id  The ID being created.
 ** \param global Whether the ID is globally visible to other processes, or
 *sealed to this process.
 **/
TAL_FUNCTION(void, TAL_CreateIDEx,
             (TAL_FUNCTION_ARGS_ TAL_ID id, TAL_BOOL global))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_CreateIDEx, TAL_FUNCTION_PARAMS_ id, global)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ID_CREATED(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, id.ns,
                      id.hi, id.lo, global);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/**@ingroup Relations
 ** Capture the creation of an ID. This makes it possible to identify
 ** a task or a vtask, and thus establish relationships with it,
 ** before its execution. IDs created by this function are implicitly
 ** local.
 ** \param id  The ID being created.
 **/
TAL_FUNCTION(void, TAL_CreateID, (TAL_FUNCTION_ARGS_ TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_CreateID, TAL_FUNCTION_PARAMS_ id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ID_CREATED(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, id.ns,
                      id.hi, id.lo, 0);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
#ifndef TAL_DOXYGEN
/** Feature not yet implemented.
 **@ingroup Relations
 ** \param The ID of the taks being stolen.
 **/
TAL_FUNCTION(void, TAL_TaskStolen, (TAL_FUNCTION_ARGS_ TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_TaskStolen, TAL_FUNCTION_PARAMS_ id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_TASK_STOLEN(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, id.ns,
                       id.hi, id.lo);
}
#endif //! defined(TAL_FUNCTION_BODY)

#endif

/************************************************************************/
#ifndef TAL_DOXYGEN
TAL_FUNCTION(void, TAL_TaskDeleted, (TAL_FUNCTION_ARGS_ TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_TaskDeleted, TAL_FUNCTION_PARAMS_ id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ID_RETIRED(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, id.ns,
                      id.hi, id.lo);
}
#endif //! defined(TAL_FUNCTION_BODY)

#endif

/************************************************************************/
/** @ingroup Relations
 ** Capture the deletion of an ID.
 **
 ** Informs TAL that an ID is no longer being used.
 ** Making this call is necessary for TAL to recognize any relations
 ** applied against this ID. If you are seeing vtasks with zero duration,
 ** you may have forgotten to retire the ID.
 ** \param id  The ID to be retired.
 **/
TAL_FUNCTION(void, TAL_RetireID, (TAL_FUNCTION_ARGS_ TAL_ID id))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_RetireID, TAL_FUNCTION_PARAMS_ id)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ID_RETIRED(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, id.ns,
                      id.hi, id.lo);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/************************************************************************/

/************************************************************************/
/** @ingroup Counters
 ** Increments a relative counter by 1.
 **
 ** Increments a relative counter by 1.
 ** \param name Name of the counter
 **/
TAL_FUNCTION(void, TAL_IncCounter, (TAL_FUNCTION_ARGS_ const char *name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_IncCounter, TAL_FUNCTION_PARAMS_ name)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_COUNTER(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                       1);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Counters
 ** Increments a relative counter by 1.
 **
 ** Increments a relative counter by 1. The counter name to be incremented is
 *provided as a string handle, improving both
 ** performance [when tracing] and the amount of space consumed by the command
 *in the trace file
 ** \param name Name of the counter, represented by the TAL_STRING_HANDLE
 *returned from TAL_GetStringHandle
 ** */
TAL_FUNCTION(void, TAL_IncCounterH, (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_IncCounterH, TAL_FUNCTION_PARAMS_ name)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_COUNTER_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                         name, 1);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/** @ingroup Counters
 ** Decrement a relative counter by 1.
 **
 ** Decrement a relative counter by 1.
 ** \param name Name of the counter.
 **/
TAL_FUNCTION(void, TAL_DecCounter, (TAL_FUNCTION_ARGS_ const char *name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_DecCounter, TAL_FUNCTION_PARAMS_ name)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_SUB_COUNTER(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                       1);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Counters
 ** Decrement a relative counter by 1.
 **
 ** Decrement a relative counter by 1. The counter name to be decremented is
 *provided as a string handle, improving both
 ** performance [when tracing] and the amount of space consumed by the command
 *in the trace file.
 ** \param name Name of the counter, represented by the TAL_STRING_HANDLE
 *returned from TAL_GetStringHandle
 **/
TAL_FUNCTION(void, TAL_DecCounterH, (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_DecCounterH, TAL_FUNCTION_PARAMS_ name)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_SUB_COUNTER_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                         name, 1);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/** @ingroup Counters
 ** Adds the specified value to a relative counter.
 **
 ** Adds the specified value to a relative counter.
 ** \param name Name of the counter
 ** \param val  Amount to be added
 **/
TAL_FUNCTION(void, TAL_AddCounter,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_INT64 val))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddCounter, TAL_FUNCTION_PARAMS_ name, val)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_COUNTER(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                       val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Counters
** Adds the specified value to a relative counter.
**
** Adds the specified value to a relative counter. The counter name to be
*incremented is provided as a string handle, improving both
** performance [when tracing] and the amount of space consumed by the command in
*the trace file.
** \param name Name of the counter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param val  Amount to be added
**/
TAL_FUNCTION(void, TAL_AddCounterH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_INT64 val))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_AddCounterH, TAL_FUNCTION_PARAMS_ name, val)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_ADD_COUNTER_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                         name, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/** \ingroup Counters
** Stores the current value of an absolute counter to the trace.
**
** Stores the current value of an absolute counter to the trace.
** \param name Name of the counter
** \param val  Value to be stored with this sample
**/
TAL_FUNCTION(void, TAL_SampleCounter,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_INT64 val))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_SampleCounter, TAL_FUNCTION_PARAMS_ name, val)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_SAMPLE_COUNTER(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                          name, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** \ingroup Counters
 ** Stores the current value of an absolute counter to the trace.
 **
 ** Stores the current value of an absolute counter to the trace. The counter
 *name to be sampled is provided as a string handle, improving both
 ** performance [when tracing] and the amount of space consumed by the command
 *in the trace file.
 ** \param name Name of the counter, represented by the TAL_STRING_HANDLE
 *returned from TAL_SampleStringToPool. For performance reasons, passing
 *0xFFFFFFFF as a handle causes the GUI to ignore this command completely.
 ** \param val  Value to be stored with this sample.
 **/
TAL_FUNCTION(void, TAL_SampleCounterH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_INT64 val))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_SampleCounterH, TAL_FUNCTION_PARAMS_ name, val)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_SAMPLE_COUNTER_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                            name, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/** \ingroup Counters
** Sample the currently configured hardware performance counters and store them
*in the trace.
**
** Sample the currently configured hardware performance counters and store them
*in the trace.
** The TaskAnalyzer tool will help you analyze multiple such samples to
*understand the counter's trend over time.
**
** <b>This feature is not not supported on Windows at this time.</b> On
*unsupported OSes, this API will return immediately.
**/
TAL_FUNCTION(void, TAL_SampleHardwareCounters, (TAL_FUNCTION_ARGS))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_SampleHardwareCounters, TAL_FUNCTION_PARAMS)
#else //! defined(TAL_FUNCTION_BODY)
{
#ifdef TAL_ON_LRB
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_SAMPLE_HW_COUNTERS_32x4(trace, TAL_FUNCTION_ARG_LVL,
                                   TAL_FUNCTION_ARG_CAT);
#else  // TAL_PLATFORM
  UNUSED_TAL_FUNCTION_ARGS
#endif // TAL_PLATFORM
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/** @ingroup Counters
 ** Subtract the specified amount from a relative counter.
 **
 ** Subtract the specified amount from a relative counter.
 ** \param name Name of the counter
 ** \param val  Amount to be subtracted
 **/
TAL_FUNCTION(void, TAL_SubCounter,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_INT64 val))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_SubCounter, TAL_FUNCTION_PARAMS_ name, val)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_SUB_COUNTER(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                       val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Counters
** Subtract the specified amount from a relative counter.
**
** Subtract the specified amount from a relative counter. The counter name is
*provided as a string handle, improving both
** performance [when tracing] and the amount of space consumed by the command in
*the trace file.
** \param name Name of the counter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param val  Amount to be subtracted
**/
TAL_FUNCTION(void, TAL_SubCounterH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_INT64 val))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_SubCounterH, TAL_FUNCTION_PARAMS_ name, val)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_SUB_COUNTER_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT,
                         name, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/************************************************************************/

/** @ingroup EventsAndMarkers
 ** Records a process-specific marker.
 **
 ** TAL markers are used to record instantaneous points within your software
 *that are process wide.
 ** They will be displayed at the top of the timeline, rather than on the
 *specific thread where the marker is recorded.
 ** Some examples of good marker usages are for hardware-level events such as
 *"vertical sync occured" or "system entered low power mode."
 **
 ** \param name Name of the marker
 **/
TAL_FUNCTION(void, TAL_Marker, (TAL_FUNCTION_ARGS_ const char *name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Marker, TAL_FUNCTION_PARAMS_ name)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_MARKER(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup EventsAndMarkers
 ** Records a process-specific marker, with explicit timestamp.
 **
 ** TAL markers are used to record instantaneous points within your software
 *that are process wide.
 ** They will be displayed at the top of the timeline, rather than on the
 *specific thread where the marker is recorded.
 ** Some examples of good marker usages are for hardware-level events such as
 *"vertical sync occured" or "system entered low power mode."
 **
 ** \param name Name of the marker
 ** \param ts   Time stamp in TAL_ticks (see TAL_GetTimestampFreq())
 **/
TAL_FUNCTION(void, TAL_MarkerEx,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_MarkerEx, TAL_FUNCTION_PARAMS_ name, ts)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_MARKER_EX(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                     ts);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/

/** @ingroup EventsAndMarkers
 ** Records a thread-specific event.
 **
 ** TAL events are used to record instantaneous events within your software.
 *They will be
 ** displayed alongside any tasks you have recorded in your application. Classic
 *uses of events are
 ** for error cases ("buffer became full") and other performance-critical
 *occurances ("dropped data because bandwidth exceeded").
 **
 ** \param name Name of the event
 **/
TAL_FUNCTION(void, TAL_Event, (TAL_FUNCTION_ARGS_ const char *name))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Event, TAL_FUNCTION_PARAMS_ name)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_EVENT(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup EventsAndMarkers
 ** Records a thread-specific event with explicit timestamp.
 **
 ** TAL events are used to record instantaneous events within your software.
 *They will be
 ** displayed alongside any tasks you have recorded in your application. Classic
 *uses of events are
 ** for error cases ("buffer became full") and other performance-critical
 *occurances ("dropped data because bandwidth exceeded").
 **
 ** \param name Name of the event
 ** \param ts   Time stamp in TAL_ticks (see TAL_GetTimestampFreq())
 **/
TAL_FUNCTION(void, TAL_EventEx,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_UINT64 ts))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_EventEx, TAL_FUNCTION_PARAMS_ name, ts)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_EVENT_EX(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                    ts);
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/**  @ingroup Parameters
** Attaches a named 32-bit signed integer value to the enclosing task, vtask or
*event.
**
** Attaches a named 32-bit signed integer value to the enclosing task, vtask or
*event.
** \param name Name of the parameter
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_Parami,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_INT32 value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Parami, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I32V(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      1, &value);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named pair of 32-bit signed integer values to the enclosing task,
*vtask or event.
**
** Attaches a named pair of 32-bit signed integer values to the enclosing task,
*vtask or event.
** \param name Name of the parameter
** \param value1 1st parameter value
** \param value2 2nd parameter value
**/
TAL_FUNCTION(void, TAL_Param2i,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_INT32 value1,
              TAL_INT32 value2))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param2i, TAL_FUNCTION_PARAMS_ name, value1, value2)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_INT32 val[2];
  val[0] = value1;
  val[1] = value2;
  TAL_PUSH_PARAM_I32V(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      2, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches an named array of 32-bit signed integer values to the enclosing
*task, vtask or event.
**
** Attaches an named array of 32-bit signed integer values to the enclosing
*task, vtask or event.
** \param name Name of the parameter
** \param nelems Number of integer elements in parameter
** \param values Pointer to parameter values
**/
TAL_FUNCTION(void, TAL_Paramiv,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_UINT32 nelems,
              TAL_INT32 *values))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Paramiv, TAL_FUNCTION_PARAMS_ name, nelems, values)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I32V(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      nelems, values);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named 64-bit signed integer value to the enclosing task, vtask or
*event.
**
** Attaches a named 64-bit signed integer value to the enclosing task, vtask or
*event.
** \param name Name of the parameter
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_Param64i,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_INT64 value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param64i, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I64V(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      1, &value);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named array of 64-bit signed integer values to the enclosing task,
*vtask or event.
**
** Attaches a named array of 64-bit signed integer values to the enclosing task,
*vtask or event.
** \param name Name of the parmaeter
** \param nelems Number of integer elements in values array
** \param values Pointer to parameter values
**/
TAL_FUNCTION(void, TAL_Param64iv,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_UINT32 nelems,
              TAL_INT64 *values))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param64iv, TAL_FUNCTION_PARAMS_ name, nelems, values)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I64V(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      nelems, values);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named pointer value to the enclosing task, vtask or event.
**
** Attaches a named pointer value to the enclosing task, vtask or event.
** \param name Name of the parameter
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_Paramp,
             (TAL_FUNCTION_ARGS_ const char *name, void *value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Paramp, TAL_FUNCTION_PARAMS_ name, value)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
#ifdef TAL_64
  TAL_UINT64 v = (TAL_UINT64)value;
  TAL_PUSH_PARAM_I64V(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      1, &v);
#else  // TAL_32
  TAL_UINT32 v = (TAL_UINT32)value;
  TAL_PUSH_PARAM_I32V(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      1, &v);
#endif // TAL_32
}
#endif //! defined(TAL_FUNCTION_BODY)

/**  @ingroup Parameters
** Attaches a named floating point value to the enclosing task, vtask or event.
**
** Attaches a named floating point value to the enclosing task, vtask or event.
** \param name Name of the parameter
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_Paramf,
             (TAL_FUNCTION_ARGS_ const char *name, float value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Paramf, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_FV(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name, 1,
                    &value);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named pair of floating point values to the enclosing task, vtask
*or event.
**
** Attaches a named pair of floating point values to the enclosing task, vtask
*or event.
** \param name Name of the parameter
** \param value1 1st parameter value
** \param value2 2nd parameter value
**/
TAL_FUNCTION(void, TAL_Param2f,
             (TAL_FUNCTION_ARGS_ const char *name, float value1, float value2))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param2f, TAL_FUNCTION_PARAMS_ name, value1, value2)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  float val[2];
  val[0] = value1;
  val[1] = value2;
  TAL_PUSH_PARAM_FV(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name, 2,
                    val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
 ** Attaches a named three-tuple of floating point values to the enclosing task,
 *vtask or event.
 **
 ** Attaches a named three-tuple of floating point values to the enclosing task,
 *vtask or event.
 ** \param name Name of the parameter
 ** \param value1 1st parameter value
 ** \param value2 2nd parameter value
 ** \param value3 3rd parameter value
 **/
TAL_FUNCTION(void, TAL_Param3f,
             (TAL_FUNCTION_ARGS_ const char *name, float value1, float value2,
              float value3))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param3f, TAL_FUNCTION_PARAMS_ name, value1, value2,
                  value3)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  float val[3];
  val[0] = value1;
  val[1] = value2;
  val[2] = value3;
  TAL_PUSH_PARAM_FV(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name, 3,
                    val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named four-tuple of floating point values to the enclosing task,
*vtask or event.
**
** Attaches a named four-tuple of floating point values to the enclosing task,
*vtask or event.
** \param name Name of the parameter
** \param value1 1st parameter value
** \param value2 2nd parameter value
** \param value3 3rd parameter value
** \param value4 4th parameter value
**/
TAL_FUNCTION(void, TAL_Param4f,
             (TAL_FUNCTION_ARGS_ const char *name, float value1, float value2,
              float value3, float value4))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param4f, TAL_FUNCTION_PARAMS_ name, value1, value2,
                  value3, value4)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  float val[4];
  val[0] = value1;
  val[1] = value2;
  val[2] = value3;
  val[3] = value4;
  TAL_PUSH_PARAM_FV(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name, 4,
                    val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
 ** Attaches a named array of floating point values to the enclosing task, vtask
 *or event.
 **
 ** Attaches a named array of floating point values to the enclosing task, vtask
 *or event.
 ** \param name Name of the parameter
 ** \param nelems Number of float elements in values array
 ** \param values Poiner to parameter values
 **/
TAL_FUNCTION(void, TAL_Paramfv,
             (TAL_FUNCTION_ARGS_ const char *name, TAL_UINT32 nelems,
              float *values))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Paramfv, TAL_FUNCTION_PARAMS_ name, nelems, values)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_FV(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                    nelems, values);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
 ** Attaches a named string to the enclosing task, vtask or event.
 **
 ** Attaches a named string to the enclosing task, vtask or event.
 ** \param name Name of the parameter
 ** \param value Parameter value
 **/
TAL_FUNCTION(void, TAL_ParamStr,
             (TAL_FUNCTION_ARGS_ const char *name, const char *value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParamStr, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  // Do check here to avoid va_arg parsing for TAL_Param (uses PUSH_PARAM_S as
  // well)
  if (TAL_IS_LOGGABLE(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT)) {
    TAL_PUSH_PARAM_S(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                     value);
  }
}
#endif //! defined(TAL_FUNCTION_BODY)

/************************************************************************/
/** @ingroup Parameters
** Attaches a named 32-bit signed integer value to the enclosing task, vtask or
*event.
**
** Attaches a named 32-bit signed integer value to the enclosing task, vtask or
*event. This version uses TAL_STRING_HANDLE to specify the parameter name,
*making it faster
** and more bandwidth-efficient than the TAL_Parami version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_ParamiH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_INT32 value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParamiH, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I32V_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                        1, &value);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named pair of 32-bit signed integer values to the enclosing task,
*vtask or event.
**
** Attaches a named pair of 32-bit signed integer values to the enclosing task,
*vtask or event. This version uses TAL_STRING_HANDLE to specify the parameter
*name, making it faster
** and more bandwidth-efficient than the TAL_Param2i version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value1 1st parameter value
** \param value2 2nd parameter value
**/
TAL_FUNCTION(void, TAL_Param2iH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_INT32 value1,
              TAL_INT32 value2))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param2iH, TAL_FUNCTION_PARAMS_ name, value1, value2)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  int val[2] = {value1, value2};
  TAL_PUSH_PARAM_I32V_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                        2, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named array of 32-bit signed integer values to the enclosing task,
*vtask or event.
**
** Attaches a named array of 32-bit signed integer values to the enclosing task,
*vtask or event. This version uses TAL_STRING_HANDLE to specify a parameter,
*making it faster
** and more bandwidth-efficient than the TAL_Paramiv version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param nelems Number of integer elements in values array
** \param values Pointer to parameter values
**/
TAL_FUNCTION(void, TAL_ParamivH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_UINT32 nelems,
              TAL_INT32 *values))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParamivH, TAL_FUNCTION_PARAMS_ name, nelems, values)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I32V_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                        nelems, values);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named 64-bit signed integer value to the enclosing task, vtask or
*event.
**
** Attaches a named 64-bit signed integer value to the enclosing task, vtask or
*event. This version uses TAL_STRING_HANDLE to specify the parameter name,
*making it faster
** and more bandwidth-efficient than the TAL_Param64i version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_Param64iH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_INT64 value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param64iH, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I64V_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                        1, &value);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named array of 64-bit signed integer values to the enclosing task,
*vtask or event.
**
** Attaches a named array of 64-bit signed integer values to the enclosing task,
*vtask or event. This version uses TAL_STRING_HANDLE to specify the parameter
*name, making it faster
** and more bandwidth-efficient than the TAL_Param64iv version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param nelems Number of integer elements in value array
** \param values Pointer to parameter values
**/
TAL_FUNCTION(void, TAL_Param64ivH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_UINT32 nelems,
              TAL_INT64 *values))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param64ivH, TAL_FUNCTION_PARAMS_ name, nelems, values)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_I64V_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                        nelems, values);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named pointer value to the enclosing task, vtask or event.
**
** Attaches a named pointer value to the enclosing task, vtask or event. This
*version uses TAL_STRING_HANDLE to specify the parameter name, making it faster
** and more bandwidth-efficient than the TAL_Paramp version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_ParampH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, void *value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParampH, TAL_FUNCTION_PARAMS_ name, value)
#else //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
#ifdef TAL_64
  TAL_UINT64 v = (TAL_UINT64)value;
  TAL_PUSH_PARAM_I64V_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                        1, &v);
#else  // TAL_32
  TAL_UINT32 v = (TAL_UINT32)value;
  TAL_PUSH_PARAM_I32V_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                        1, &v);
#endif // TAL_32
}
#endif //! defined(TAL_FUNCTION_BODY)

/**  @ingroup Parameters
** Attaches a named floating point value to the enclosing task, vtask or event.
**
** Attaches a named floating point value to the enclosing task, vtask or event.
*This version uses TAL_STRING_HANDLE to specify the parameter name, making it
*faster
** and more bandwidth-efficient than the TAL_Paramf version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_ParamfH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, float value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParamfH, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_FV_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      1, &value);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named pair of floating point values to the enclosing task, vtask
*or event.
**
** Attaches a named pair of floating point values to the enclosing task, vtask
*or event. This version uses TAL_STRING_HANDLE to specify the parameter name,
*making it faster
** and more bandwidth-efficient than the TAL_Param2f version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value1 1st parameter value
** \param value2 2nd parameter value
**/
TAL_FUNCTION(void, TAL_Param2fH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, float value1,
              float value2))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param2fH, TAL_FUNCTION_PARAMS_ name, value1, value2)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  float val[2] = {value1, value2};
  TAL_PUSH_PARAM_FV_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      2, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named three-tuple of floating point values to the enclosing task,
*vtask or event.
**
** Attaches a named three-tuple of floating point values to the enclosing task,
*vtask or event. This version uses TAL_STRING_HANDLE to specify the parameter
*name, making it faster
** and more bandwidth-efficient than the TAL_Param3f version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value1 1st parameter value
** \param value2 2nd parameter value
** \param value3 3rd parameter value
**/
TAL_FUNCTION(void, TAL_Param3fH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, float value1,
              float value2, float value3))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param3fH, TAL_FUNCTION_PARAMS_ name, value1, value2,
                  value3)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  float val[3] = {value1, value2, value3};
  TAL_PUSH_PARAM_FV_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      3, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named four-tuple of floating point values to the enclosing task,
*vtask or event.
**
** Attaches a named four-tuple of floating point values to the enclosing task,
*vtask or event. This version uses TAL_STRING_HANDLE to specify the parameter
*name, making it faster
** and more bandwidth-efficient than the TAL_Param4f version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value1 1st parameter value
** \param value2 2nd parameter value
** \param value3 3rd parameter value
** \param value4 4th parameter value
**/
TAL_FUNCTION(void, TAL_Param4fH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, float value1,
              float value2, float value3, float value4))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param4fH, TAL_FUNCTION_PARAMS_ name, value1, value2,
                  value3, value4)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  float val[4] = {value1, value2, value3, value4};
  TAL_PUSH_PARAM_FV_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      4, val);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
 ** Attaches a named array of floating point values to the enclosing task, vtask
 *or event.
 **
 ** Attaches a named array of floating point values to the enclosing task, vtask
 *or event. This version uses TAL_STRING_HANDLE to specify the parameter name,
 *making it faster
 ** and more bandwidth-efficient than the TAL_Paramfv version.
 ** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
 *returned from TAL_GetStringHandle
 ** \param nelems Number of float elements in values array
 ** \param values Pointer to parameter values
 **/
TAL_FUNCTION(void, TAL_ParamfvH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, TAL_UINT32 nelems,
              float *values))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParamfvH, TAL_FUNCTION_PARAMS_ name, nelems, values)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  TAL_PUSH_PARAM_FV_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                      nelems, values);
}
#endif //! defined(TAL_FUNCTION_BODY)

/** @ingroup Parameters
** Attaches a named string parameter to the enclosing task, vtask or event.
**
** Attaches a named string parameter to the enclosing task, vtask or event. This
*version uses TAL_STRING_HANDLE to specify the parameter name, making it faster
** and more bandwidth-efficient than the TAL_ParamStr version.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param value Parameter value
**/
TAL_FUNCTION(void, TAL_ParamStrH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, const char *value))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParamStrH, TAL_FUNCTION_PARAMS_ name, value)
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  // Do check here to avoid va_arg parsing for TAL_ParamH (uses PUSH_PARAM_S_H
  // as well)
  if (TAL_IS_LOGGABLE(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT)) {
    TAL_PUSH_PARAM_S_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                       value);
  }
}
#endif //! defined(TAL_FUNCTION_BODY)

#ifndef TAL_KERNEL
/** @ingroup Parameters
 ** Attaches a named printf-formatted string to the enclosing task, vtask or
 *event.
 **
 ** Attaches a named printf-formatted string to the enclosing task, vtask or
 *event.
 **
 ** Note that this parameter is provided for convenience;
 ** it has extremely slow performance, as compared to other TAL_Param calls.
 ** \param name Name of the parameter
 ** \param fmt  printf-style format specifier for parameter value
 **/
TAL_FUNCTION(void, TAL_Param,
             (TAL_FUNCTION_ARGS_ const char *name, const char *fmt, ...))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_Param, TAL_FUNCTION_PARAMS, name, fmt) // FIXME
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  if (TAL_IS_LOGGABLE(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT)) {
    char *val = NULL;
    TAL_VA_LIST ap;
    TAL_VA_START(ap, fmt);
    val = TAL_Vasprintf(fmt, ap);
    TAL_VA_END(ap);
    TAL_PUSH_PARAM_S(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                     val);
    free(val);
  }
}
#endif //! defined(TAL_FUNCTION_BODY)
#endif // ndef TAL_KERNEL

#ifndef TAL_KERNEL
/** @ingroup Parameters
** Attaches a named printf-formatted string to the enclosing task, vtask or
*event.
**
** Attaches a named printf-formatted string to the enclosing task, vtask or
*event. This version uses TAL_STRING_HANDLE to specify the parameter name,
*making it faster
** and more bandwidth-efficient than the TAL_Param version.
**
** Note that this parameter is provided for convenience;
** it has extremely slow performance, as compared to other TAL_Param calls.
** \param name Name of the parameter, represented by the TAL_STRING_HANDLE
*returned from TAL_GetStringHandle
** \param fmt printf format specifier for parameter value
**/
TAL_FUNCTION(void, TAL_ParamH,
             (TAL_FUNCTION_ARGS_ TAL_STRING_HANDLE name, const char *fmt, ...))
#if defined(TAL_FUNCTION_BODY)
TAL_FUNCTION_BODY(TAL_ParamH, TAL_FUNCTION_PARAMS, name, fmt) // FIXME
#else  //! defined(TAL_FUNCTION_BODY)
{
  TAL_TRACE *trace = TAL_FUNCTION_ARG_TRACE;
  if (TAL_IS_LOGGABLE(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT)) {
    char *val = NULL;
    va_list ap;
    va_start(ap, fmt);
    val = TAL_Vasprintf(fmt, ap);
    va_end(ap);
    TAL_PUSH_PARAM_S_H(trace, TAL_FUNCTION_ARG_LVL, TAL_FUNCTION_ARG_CAT, name,
                       val);
    free(val);
  }
}
#endif //! defined(TAL_FUNCTION_BODY)

#endif // ndef TAL_KERNEL
#if TAL_COMPILER == TAL_COMPILER_MSVC || TAL_COMPILER == TAL_COMPILER_ICC
#pragma warning(pop)
#endif // TAL_COMPILER == TAL_COMPILER_MSVC || TAL_COMPILER == TAL_COMPILER_ICC

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
