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
#if (!defined(TAL_STRUCTS_H)) || defined(INHIBIT_MANAGED_DECL)
#define TAL_STRUCTS_H
#include "../common/managed_decl.h"

#include "tal_types.h"

#ifdef _MANAGED
#ifndef INHIBIT_MANAGED_DECL
#if defined(__TALTRACE_H__)
namespace GT {
namespace Tal {
#else  // !__TALTRACE_H__
namespace libTaskalyzer {
#endif // !__TALTRACE_H__
#endif // INHIBIT_MANAGED_DECL
#endif //_MANAGED

#ifndef TAL_DOXYGEN
typedef struct _TAL_VERSION {
  const char *BuildVersion;
  TAL_UINT32 ApiMajor;
  TAL_UINT32 ApiMinor;
  TAL_UINT32 FileMajor;
  TAL_UINT32 FileMinor;
} TAL_VERSION;
#endif

/** \enum _TAL_RELATION
 ** Represents a specific type of relationship between a pair of tasks.
 ** TAL has a set of APIs called the relations APIs that allow you to establish
 *a runtime
 ** relation between a pair fo tasks. There are a variety of relationships
 *between tasks
 ** recognized by TAL.
 **
 ** All relationships in TAL are symmetric, in the sense that where there is a
 *relation indicating
 ** a relationship X from Task A to Task B, there will be another relationship Y
 *implied that goes from
 ** B to A. For example, the inverse of TAL_RELATION_IS_CHILD_OF is
 *TAL_RELATION_IS_PARENT_OF.
 **
 ** In the API, you only have to specify one direction of the relationship to
 *establish the pair;
 ** which of the two values you pick is up to you --- sometimes it is more
 *convenient to establish the relationship
 ** in one direction than the other. For example, when you are creating a task,
 *it is often easier to
 ** in code to specify it as a child of the current task than remember the
 *parent task ID and establish the
 ** relationship during the child's execution function.
 **
 ** Most important among the TAL relations are the PARENT and CHILD relations.
 *In TAL
 ** nomenclature, a task that creates another task is said to be the PARENT, the
 *created task the CHILD.
 ** Where and when the child task executes is undefined --- that is, the task
 *can execute on any thread,
 ** at any time, and still be a valid parent-child relationship.
 **
 ** Sometimes, a task will be created that cannot execute until another task
 *completes. For example,
 ** you might have the following code that sets up a pair of dependent tasks,
 *the second of which cannot execute
 ** without the first completing:
 ** \code
 **   void FillInBuffer() { ... }
 **   void WriteToDisk() { ... }
 **   void SaveToDisk() {
 **      task* t = Enqueue(FillInBuffer);
 **      Enqueue(WriteToDisk, t);
 **   }
 ** \endcode
 ** To capture these relationships in TAL, use the IS_DEPENDENT_ON relation.
 *Specifically, the WriteToDisk
 ** task has a TAL_RELATION_IS_DEPENDENT_ON relation to the FillInBuffer task.
 *Or, if you want to look at it the other way, the
 ** FillInBuffer task has a TAL_RELATION_IS_PREDECESSOR_TO (the inverse of
 *IS_DEPENDENT_ON) relation to the WriteToDisk task.
 **
 ** In a continuation programming model, a developer can decompose a
 *long-running parent task into some initial work plus additional
 ** work to be done later. From a task scheduling point of view, the usual
 *meaning of the continuation is that the original task is not complete
 ** until the continuation task has also completed. To model this type of
 *relationship between tasks, use the TAL_RELATION_IS_CONTINUATION_OF and
 *TAL_RELATION_IS_CONTINUED_BY
 ** values. For a continuation-driven pair of tasks such as
 *"BeginFrame()->EndFrame()," BeginFrame has the TAL_RELATION_IS_CONTINUED_BY
 *relation to EndFrame,
 ** while EndFrame has the TAL_RELATION_IS_CONTINUATION_OF relation to
 *Beginframe.
 **
 ** A final relation in TAL is the "sibling" relation, which is used to
 *represent a group of tightly related tasks. The tasks
 ** of a parallel_for are siblings of one-another, for instance, since they are
 *all operating on logically-related parts of the same work unit.
 ** In most TAL relations, making a task X related to another Y by R implies
 *that Y is related to X by the inverse of R. For the sibling relation,
 ** those rules apply PLUS the following additional semantics:
 ** - When X is a sibling of Y, and X is a sibling of Z, Y and Z are also
 *implied to be siblings of one another.
 ** - When X is a sibling of Y, and Y is a sibling of Z, Z is implied to be a
 *sibling of X.
 ** The reason for these extra semantics is efficiency. To take a group of N
 *tasks and relate them using the traditional "opposite relation" semantics used
 *on
 ** other relations would require N*N different TAL_AddRelation calls. In
 *contrast, this requires just N --- typically, to establish a group of N
 *related siblings, each
 ** sibling just makes itself related to the 0th sibling --- from there, TAL
 *will establish the other relations for you.
 **
 ** Please see \ref Relations for more details on using the relations API.
 **/
BEGIN_ENUM_DECL(TAL_RELATION){
/* must be leq 0xFF to not overflow arg1 */ /* keep this sync'd with
                                               taskalyzer_opcodes.cpp */
#ifndef TAL_DOXYGEN
    TAL_RELATION_DEPENDENCY =
        0x1, // "that" must complete before "this" can execute
    TAL_RELATION_SIBLING =
        0x2, // "that" and "this" are part of the same logical group of tasks.
             // Example: tasks within the same parallel-for group are siblings
             // of one another.
    TAL_RELATION_PARENT = 0x3, // "that" is the logical parent task of "this"
                               // task. The two tasks may not be on same thread.
    TAL_RELATION_CONTINUATION =
        0x4, // a continuation in sense that the "that" task assumes all
             // dependencies of the continued task ("this")
    TAL_RELATION_CHILD =
        0x5, // "that" is the logical child task created by "this" task.  The
             // two tasks may not be on same thread.
    TAL_RELATION_PREDECESSOR = 0x6,    // opposite of continuation.
    TAL_RELATION_OUT_DEPENDENCY = 0x7, // opposite of dependency
#endif

    TAL_RELATION_IS_DEPENDENT_ON =
        0x8, /**< "This" task cannot start before "that" task
                completes.\ Inverse of TAL_RELATION_IS_PREDECESSOR_TO. */
    TAL_RELATION_IS_SIBLING_OF =
        0x9, /**< "This" task and "that" task are part of the same logical
                group.\ Example: tasks within the same parallel-for group are
                siblings of one another.\ Inverse of itself. */
    TAL_RELATION_IS_PARENT_OF =
        0xA, /**< "This" task is the logical parent that creates "that"
                task.\ The two tasks need not be on the same thread.\ Inverse of
                TAL_RELATION_IS_CHILD_OF. */
    TAL_RELATION_IS_CONTINUATION_OF =
        0xB, /**< "This" task continues "that" task and assumes all dependencies
                of "that" task.\ Inverse of TAL_RELATION_IS_CONTINUED_BY. */
    TAL_RELATION_IS_CHILD_OF =
        0xC, /**< "This" task is the logical child created by "that" task.\ The
                two tasks need not be on the same thread.\ Inverse of
                TAL_RELATION_IS_PARENT_OF. */
    TAL_RELATION_IS_CONTINUED_BY =
        0xD, /**< "This" task is continued by "that" task, and "that" task
                assumes all dependencies of "this" task.\ Inverse of
                TAL_RELATION_IS_CONTINUATION_OF. */
    TAL_RELATION_IS_PREDECESSOR_TO =
        0xE /**< "This" task must complete before "that" task can
               start.\ Inverse of TAL_RELATION_IS_DEPENDENT_ON. */
} END_ENUM_DECL(TAL_RELATION)

#ifndef TAL_DOXYGEN
    typedef struct _TAL_ID {
  TAL_UINT64 hi;
  TAL_UINT64 lo;
  TAL_ID_NAMESPACE ns;
  TAL_BYTE padding[3];
} TAL_ID;
#else
    struct TAL_ID;
#endif

#ifndef TAL_DOXYGEN
typedef struct _TAL_ESCAPE {
  union {
    TAL_UINT32 DWord;
    struct {
      TAL_UINT32 ArgB : 3;
      TAL_UINT32 ArgA : 8;
      TAL_UINT32 SizeInDwords : 10;
      TAL_UINT32 OpCode : 10;
      TAL_UINT32 Reserved : 1;
    } Args;
  } Command;
  TAL_UINT32 SizeInBytes;
} TAL_ESCAPE;

#endif // ndef(TAL_DOXYGEN)

#ifndef TAL_DOXYGEN
BEGIN_ENUM_DECL(TAL_ESCAPE_OPCODE) /* must be leq 0xFF to not overflow arg1 */
{TAL_ESCAPE_OPCODE_THREAD_DATA = 0x01,
 TAL_ESCAPE_OPCODE_CAPTURE_PROGRESS =
     0x02, // payload is bytes-worth of progress
 TAL_ESCAPE_OPCODE_COMMAND =
     0x03, // Command Packet from server to tal or from tal to sub tal. ArgB
           // will be one of TAL_ESCAPE_COMMAND_OPCODE
 TAL_ESCAPE_OPCODE_MIN_TIME_FENCE =
     0x04, // Notifies that all data on all traces have been sent up to this
           // time.
 TAL_ESCAPE_OPCODE_DEPRECATED = 0x05,

 TAL_ESCAPE_OPCODE_THREAD_DATA_Z =
     0x06, // same as TAL_ESCAPE_OPCODE_THREAD_DATA but the data is zlib
           // encoded.
 TAL_ESCAPE_OPCODE_TRACE_METADATA =
     0x07, // Extra data generated by systems other than the TAL collector

 TAL_ESCAPE_OPCODE_PROCESS_DATA = 0x08,
 TAL_ESCAPE_OPCODE_SYMBOL_DICTIONARY = 0x09, // Stack address symbol dictionary
 TAL_ESCAPE_OPCODE_CONTEXT_SWITCH_DATA = 0x0A, // ETW context switch data
 TAL_ESCAPE_OPCODE_SCREEN_IMAGE = 0x10,        // dib image
 TAL_ESCAPE_OPCODE_GFX_DATA = 0x11, // graphics data: dx resources, etc.
 TAL_ESCAPE_OPCODE_PROCESS_DATA_ETW =
     0x12, // not actually stored in file, but used during capture
 TAL_ESCAPE_OPCODE_MEDIA_DATA = 0x13, // MPA/Media data

 TAL_ESCAPE_OPCODE_FILEVERSION = 0xFE,
 TAL_ESCAPE_OPCODE_EOF = 0xFF} END_ENUM_DECL(TAL_ESCAPE_OPCODE)

    BEGIN_ENUM_DECL(
        TAL_ESCAPE_COMMAND_OPCODE) // Must be leq 7 to not overflow argb
    {
        TAL_ESCAPE_COMMAND_CONNECT =
            0x01, //  Sent from Tal Collector to command stream when it is
                  //  connected
        TAL_ESCAPE_COMMAND_EXTENDED =
            0x07 // This command is always followed by a 32 bit value that is
                 // the actual command to be processed.
    } END_ENUM_DECL(TAL_ESCAPE_COMMAND_OPCODE)

        BEGIN_ENUM_DECL(TAL_ESCAPE_COMMANDEX_OPCODE){
            // ---------------------------------------------------------------------------
            // These are commands from the server to the capture library
            TAL_ESCAPE_COMMANDEX_SERVER_SET_CAPTURE_NAME =
                0x0, // payload string "name"
            TAL_ESCAPE_COMMANDEX_SERVER_SET =
                0x01, //  payload string "command value"
            TAL_ESCAPE_COMMANDEX_SERVER_SET_CAPTURE =
                0x02, // payload UINT32 (bool true/false)
            TAL_ESCAPE_COMMANDEX_SERVER_CONNECT_COMPLETE =
                0x03, // No payload, sent after initial connect command dump is
                      // done.
            TAL_ESCAPE_COMMANDEX_SERVER_DISCONNECT =
                0x04, // No payload - indicates the server is going away.
            TAL_ESCAPE_COMMANDEX_SERVER_PERF_TEST =
                0x05, // No payload - runs the perf test on the collector.
            TAL_ESCAPE_COMMANDEX_SERVER_RECONNECT = 0x06,
            TAL_ESCAPE_COMMANDEX_SERVER_CONNECT_ESTABLISHED =
                0x07, // No payload, sent after initial connect command dump is
                      // done.

            // Add new server commands here

            // ---------------------------------------------------------------------------
            // these are messages from the capture library to the server
            TAL_ESCAPE_COMMANDEX_CAPLIB_CONNECT =
                0x1000, //  Sent from Tal to command stream when it is connected
            TAL_ESCAPE_COMMANDEX_CAPLIB_DESCRIBE_SETTING =
                0x1001, //  payload string
                        //  "command\0defaultvalue\0description\0"
            TAL_ESCAPE_COMMANDEX_CAPLIB_LOG = 0x1002, //
            TAL_ESCAPE_COMMANDEX_CAPLIB_SET_PROCESS_DEFAULT =
                0x1003, // payload pid(UINT32), Setting string
            TAL_ESCAPE_COMMANDEX_CAPLIB_SET_CAPTURE_ACK =
                0x1004, // Sent from Tal to command stream when it receives the
                        // TAL_ESCAPE_COMMAND_SET_CAPTURE
            TAL_ESCAPE_COMMANDEX_CAPLIB_DESCRIBE_CATEGORY_MASK = 0x1005,
            TAL_ESCAPE_COMMANDEX_CAPLIB_SETTING_VALUE =
                0x1006, // setting "name\0value\0"
            TAL_ESCAPE_COMMANDEX_CAPLIB_CONNECT_2 =
                0x1007, //  Sent from Tal to command stream when it is connected
            TAL_ESCAPE_COMMANDEX_CAPLIB_SET_APP_DEFAULT =
                0x1008, // payload, app name and setting string
            TAL_ESCAPE_COMMANDEX_CAPLIB_SET_DOMAIN_NAME =
                0x1009, // send to monitor list of available domains;
                        // csv-delimited
            TAL_ESCAPE_COMMANDEX_CAPLIB_TRIGGER_PAUSED_PID =
                0x100A, // send to monitor list of available domains;
                        // csv-delimited
            TAL_ESCAPE_COMMANDEX_CAPLIB_SAT_COMPLETE =
                0x100B, // send to monitor list of available domains;
                        // csv-delimited
            TAL_ESCAPE_COMMANDEX_CAPLIB_LOG_FRAME =
                0x100C, // sent from Tal to monitor log when trigger works
            TAL_ESCAPE_COMMANDEX_CAPLIB_CAPTURE_COMPLETE =
                0x100D, // send to monitor message about capture complete
            TAL_ESCAPE_COMMANDEX_CAPLIB_CONNECT_COMPLETE =
                0x100E, // send to monitor message after all default settings
                        // sent
            TAL_ESCAPE_COMMANDEX_CAPLIB_DEVICE_DESTROY_EVENT =
                0x1011, // send to monitor notification that primary device was
                        // destroyed
            TAL_ESCAPE_COMMANDEX_CAPLIB_TRIGGER_EVENT =
                0x1012, // send to monitor notification that installed from TAL
                        // trigger happened
            TAL_ESCAPE_COMMANDEX_CAPLIB_OVERRIDE_CHANGE_EVENT =
                0x1013, // send information about available state overrides to
                        // monitor
            TAL_ESCAPE_COMMANDEX_CAPLIB_APIINFO_CHANGE_EVENT =
                0x1014, // send API information to monitor

            // Add new capture library commands here

            TAL_ESCAPE_COMMANDEX_END =
                0xFFFFFFFF} END_ENUM_DECL(TAL_ESCAPE_COMMANDEX_OPCODE)

            BEGIN_ENUM_DECL(
                TAL_ESCAPE_TRACE_METADATA_OPCODE) // Must be leq 7 to not
                                                  // overflow argb
    {
        TAL_ESCAPE_TRACE_METADATA_SYMBOL_TABLE =
            0x00 // payload: {pid(UINT64) {fnptr(UINT64) TALSYM_LOOKUP}+}
    } END_ENUM_DECL(TAL_ESCAPE_TRACE_METADATA_OPCODE)

#endif // ndef(TAL_DOXYGEN)

#ifndef TAL_DOXYGEN
        typedef struct _TAL_ESCAPE_COMMAND {
  TAL_ESCAPE Escape;
  TAL_ESCAPE_COMMANDEX_OPCODE CommandEx;
} TAL_ESCAPE_COMMAND;
#endif // ndef(TAL_DOXYGEN)

#ifndef TAL_DOXYGEN
BEGIN_ENUM_DECL(TAL_FILE_VERSION){
    TAL_FILE_VERSION_UNKNOWN = -1,
    TAL_FILE_VERSION_0 =
        0, // look for delimiter opcode to separate different raw traces
    TAL_FILE_VERSION_5_NET =
        5, // different raw traces are encoded as
           // TAL_ESCAPE_COMMAND(TAL_ESCAPE_OPCODE_THREAD_DATA).
           // Delimiters present but can be ignored.
    TAL_FILE_VERSION_5_FILE =
        51, // different raw traces are encoded as
            // TAL_ESCAPE_COMMAND(TAL_ESCAPE_OPCODE_THREAD_DATA).
            // Delimiters present but can be ignored.
    TAL_FILE_VERSION_6 =
        6, // file is series of TAL_ESCAPE_COMMANDs. First escape is
           // TAL_FILE_VERSION w/ 4 byte payload is this enum. Raw traces are
           // encoded as TAL_ESCAPE_COMMAND(TAL_ESCAPE_OPCODE_THREAD_DATA).
    TAL_FILE_VERSION_7 = 7,  // Version 6 with First connect info into new
                             // TAL_ESCAPE_OPCODE_PROCESS_DATA escape.
    TAL_FILE_VERSION_8 = 8,  // Version 7 with Environment params in process
                             // data, and screen image escapes.
    TAL_FILE_VERSION_9 = 9,  // Version 8 with ANNOTE_TID_PARENT_PID & param
                             // (ArgA) in ANNOTE_PID_NAME.
    TAL_FILE_VERSION_10 = 10 // Changes to support the IPC transport.
} END_ENUM_DECL(TAL_FILE_VERSION) // if you add more formats versions, update
                                  // Disassembler.cpp and update
                                  // TAL_FILEVERSION_NUMERIC
#endif                            // ndef(TAL_DOXYGEN)

    /** \enum _TAL_COUNTER_SAMPLE_TYPE
     ** The semantics associated with a particular counter.
     ** The TAL_SampleCounter API allows you to manually sample counters. In
     *almost all use cases, your counter samples will
     ** be of software concepts --- e.g. "how much memory was used," or "number
     *of bytes in the ringbuffer." However, in some rare cases,
     ** the samples you take will be specific to a particular hardware-level
     *concept, for example a core or a chip package. In these
     ** cases, you can ues the TAL_SetCounterSampleType API to tell TAL the
     *semantics of your sample; doing so will alter the way that
     ** the TAL GUI analyzes the samples, ensuring that you get the right
     *analysis for your data.
     ** See \ref Counters for more information.
     **/
    BEGIN_ENUM_DECL(
        TAL_COUNTER_SAMPLE_TYPE) /* must be leq 0xFF to not overflow arg1 */
    {
        TAL_COUNTER_SAMPLE_TYPE_SOFTWARE,  /**< The sample is a software-level
                                              concept and does not require
                                              disambiguation across running
                                              threads. */
        TAL_COUNTER_SAMPLE_TYPE_HW_THREAD, /**< The sample is a hardware-thread
                                              specific sample, e.g.\ "l2 cache
                                              misses by this thread." */
        TAL_COUNTER_SAMPLE_TYPE_CORE, /**< The sample is a core-specific sample,
                                         e.g.\ "l2 cache misses by this core."
                                       */
        TAL_COUNTER_SAMPLE_TYPE_PACKAGE /**< The sample is a package-specific
                                           sample, e.g.\ "GDDR pages read." */
    } END_ENUM_DECL(TAL_COUNTER_SAMPLE_TYPE) // if you add more formats
                                             // versions, update
                                             // Disassembler.cpp and update
                                             // TAL_FILEVERSION_NUMERIC
    /** \enum _TAL_LOG_LEVEL
     ** An enumeration describing TAL's supported logging levels.
     ** This enumeration defines a set of "logging levels" that can be used to
     *control
     ** the verbosity of your traced data. These levels can be included in your
     ** individual function calls, causing them to be disabled when TAL's
     *runtime logging
     ** level is less than the specified level. For example:
     ** \code
     ** void Foo() {
     **    TAL_BeginNamedTask(TAL_LOG_LEVEL_2, TAL_LOG_CAT_1, "Foo");
     **    TAL_EndTask();
     ** }
     ** \endcode
     ** In this example, the "Foo" function will not be traced until the log
     *level is set to 2 or above.
     **
     ** TAL's logging level is 1 by default.
     **/
    BEGIN_ENUM_DECL(TAL_LOG_LEVEL){
        TAL_LOG_LEVEL_OFF = 0,
        TAL_LOG_LEVEL_1 = 1, //  Targeted at < 5% overhead
        TAL_LOG_LEVEL_2 = 2,     TAL_LOG_LEVEL_3 = 3,   TAL_LOG_LEVEL_4 = 4,
        TAL_LOG_LEVEL_5 = 5, // Full Trace
        TAL_LOG_LEVEL_6 = 6,     TAL_LOG_LEVEL_7 = 7,   TAL_LOG_LEVEL_8 = 8,
        TAL_LOG_LEVEL_9 = 9,     TAL_LOG_LEVEL_10 = 10,

        TAL_LOG_LEVEL_MAX = 0xFF} END_ENUM_DECL(TAL_LOG_LEVEL)

#define TAL_STACK_DEPTH_MAX 256 // Default is 10

/* TAL Log categories */
/************************************************************************/
#ifndef TAL_DOXYGEN // special version of TAL_LOG_CAT_xx for doxygen
#define TAL_LOG_CAT_NONE 0x0000000000000000ull

#define TAL_LOG_CAT_N(n) (((TAL_UINT64)1) << (n - 1))
#define TAL_LOG_CAT_1 TAL_LOG_CAT_N(1)
#define TAL_LOG_CAT_2 TAL_LOG_CAT_N(2)
#define TAL_LOG_CAT_3 TAL_LOG_CAT_N(3)
#define TAL_LOG_CAT_4 TAL_LOG_CAT_N(4)
#define TAL_LOG_CAT_5 TAL_LOG_CAT_N(5)
#define TAL_LOG_CAT_6 TAL_LOG_CAT_N(6)
#define TAL_LOG_CAT_7 TAL_LOG_CAT_N(7)
#define TAL_LOG_CAT_8 TAL_LOG_CAT_N(8)
#define TAL_LOG_CAT_9 TAL_LOG_CAT_N(9)
#define TAL_LOG_CAT_10 TAL_LOG_CAT_N(10)
#define TAL_LOG_CAT_11 TAL_LOG_CAT_N(11)
#define TAL_LOG_CAT_12 TAL_LOG_CAT_N(12)
#define TAL_LOG_CAT_13 TAL_LOG_CAT_N(13)
#define TAL_LOG_CAT_14 TAL_LOG_CAT_N(14)
#define TAL_LOG_CAT_15 TAL_LOG_CAT_N(15)
#define TAL_LOG_CAT_16 TAL_LOG_CAT_N(16)
#define TAL_LOG_CAT_17 TAL_LOG_CAT_N(17)
#define TAL_LOG_CAT_18 TAL_LOG_CAT_N(18)
#define TAL_LOG_CAT_19 TAL_LOG_CAT_N(19)
#define TAL_LOG_CAT_20 TAL_LOG_CAT_N(20)
#define TAL_LOG_CAT_21 TAL_LOG_CAT_N(21)
#define TAL_LOG_CAT_22 TAL_LOG_CAT_N(22)
#define TAL_LOG_CAT_23 TAL_LOG_CAT_N(23)
#define TAL_LOG_CAT_24 TAL_LOG_CAT_N(24)
#define TAL_LOG_CAT_25 TAL_LOG_CAT_N(25)
#define TAL_LOG_CAT_26 TAL_LOG_CAT_N(26)
#define TAL_LOG_CAT_27 TAL_LOG_CAT_N(27)
#define TAL_LOG_CAT_28 TAL_LOG_CAT_N(28)
#define TAL_LOG_CAT_29 TAL_LOG_CAT_N(29)
#define TAL_LOG_CAT_30 TAL_LOG_CAT_N(30)
#define TAL_LOG_CAT_31 TAL_LOG_CAT_N(31)
#define TAL_LOG_CAT_32 TAL_LOG_CAT_N(32)
#define TAL_LOG_CAT_33 TAL_LOG_CAT_N(33)
#define TAL_LOG_CAT_34 TAL_LOG_CAT_N(34)
#define TAL_LOG_CAT_35 TAL_LOG_CAT_N(35)
#define TAL_LOG_CAT_36 TAL_LOG_CAT_N(36)
#define TAL_LOG_CAT_37 TAL_LOG_CAT_N(37)
#define TAL_LOG_CAT_38 TAL_LOG_CAT_N(38)
#define TAL_LOG_CAT_39 TAL_LOG_CAT_N(39)
#define TAL_LOG_CAT_40 TAL_LOG_CAT_N(40)
#define TAL_LOG_CAT_41 TAL_LOG_CAT_N(41)
#define TAL_LOG_CAT_42 TAL_LOG_CAT_N(42)
#define TAL_LOG_CAT_43 TAL_LOG_CAT_N(43)
#define TAL_LOG_CAT_44 TAL_LOG_CAT_N(44)
#define TAL_LOG_CAT_45 TAL_LOG_CAT_N(45)
#define TAL_LOG_CAT_46 TAL_LOG_CAT_N(46)
#define TAL_LOG_CAT_47 TAL_LOG_CAT_N(47)
/* categories 48 thru 63 are reserved reserved for future optimization */
/*
#define TAL_LOG_CAT_48 TAL_LOG_CAT_N(48)
#define TAL_LOG_CAT_49 TAL_LOG_CAT_N(49)
#define TAL_LOG_CAT_50 TAL_LOG_CAT_N(50)
#define TAL_LOG_CAT_51 TAL_LOG_CAT_N(51)
#define TAL_LOG_CAT_52 TAL_LOG_CAT_N(52)
#define TAL_LOG_CAT_53 TAL_LOG_CAT_N(53)
#define TAL_LOG_CAT_54 TAL_LOG_CAT_N(54)
#define TAL_LOG_CAT_55 TAL_LOG_CAT_N(55)
#define TAL_LOG_CAT_56 TAL_LOG_CAT_N(56)
#define TAL_LOG_CAT_57 TAL_LOG_CAT_N(57)
#define TAL_LOG_CAT_58 TAL_LOG_CAT_N(58)
#define TAL_LOG_CAT_59 TAL_LOG_CAT_N(59)
#define TAL_LOG_CAT_60 TAL_LOG_CAT_N(60)
#define TAL_LOG_CAT_61 TAL_LOG_CAT_N(61)
#define TAL_LOG_CAT_62 TAL_LOG_CAT_N(62)
#define TAL_LOG_CAT_63 TAL_LOG_CAT_N(63)
*/
#define TAL_LOG_CAT_TAL 0x0001000000000000ull
// #define TAL_LOG_CAT_DX           0x0002000000000000ull
#define TAL_LOG_CAT_RESERVED 0xFFFF000000000000ull //
#define TAL_LOG_CAT_ALL 0xFFFFFFFFFFFFFFFFull

#define TAL_LOG_CAT_COUNT 64

#else // TAL_DOXYGEN
    /** Logging categories, used for masking off specific TAL calls by the
     *category of data they obtain.
     ** See \ref Basics for more details.
     **/
    typedef enum _TAL_LOG_CATEGORY {
      TAL_LOG_CAT_1 = 0x0000000000000001, // OM_LOG_CAT_PERF
      TAL_LOG_CAT_2 = 0x0000000000000002, // OM_LOG_CAT_RAD
      TAL_LOG_CAT_3 = 0x0000000000000004, // OM_LOG_CAT_ANALYSIS0
      TAL_LOG_CAT_4 = 0x0000000000000008, // OM_LOG_CAT_MEMORY
      TAL_LOG_CAT_5 = 0x0000000000000010, // OM_LOG_CAT_PMU
      TAL_LOG_CAT_6 = 0x0000000000000020, // OM_LOG_CAT_RCS
      TAL_LOG_CAT_7 = 0x0000000000000040, // OM_LOG_CAT_MEMORY_STATISTICS
      TAL_LOG_CAT_8 = 0x0000000000000100, // OM_LOG_CAT_COMMANDS
      TAL_LOG_CAT_9 = 0x0000000000000200, // OM_LOG_CAT_ACS
      TAL_LOG_CAT_10 = 0x0000000000000400,
      TAL_LOG_CAT_11 = 0x0000000000000800,
      TAL_LOG_CAT_12 = 0x0000000000001000,
      TAL_LOG_CAT_13 = 0x0000000000002000,
      TAL_LOG_CAT_14 = 0x0000000000004000,
      TAL_LOG_CAT_15 = 0x0000000000008000,
      TAL_LOG_CAT_16 = 0x0000000000010000,
      TAL_LOG_CAT_17 = 0x0000000000020000,
      TAL_LOG_CAT_18 = 0x0000000000040000,
      TAL_LOG_CAT_19 = 0x0000000000080000,
      TAL_LOG_CAT_20 = 0x0000000000100000,
      TAL_LOG_CAT_21 = 0x0000000000200000,
      TAL_LOG_CAT_22 = 0x0000000000400000,
      TAL_LOG_CAT_23 = 0x0000000000800000,
      TAL_LOG_CAT_24 = 0x0000000001000000,
      TAL_LOG_CAT_25 = 0x0000000002000000,
      TAL_LOG_CAT_26 = 0x0000000004000000,
      TAL_LOG_CAT_27 = 0x0000000008000000,
      TAL_LOG_CAT_28 = 0x0000000010000000, // XN - Lrb Sdk
      TAL_LOG_CAT_29 = 0x0000000020000000,
      TAL_LOG_CAT_30 = 0x0000000040000000,
      TAL_LOG_CAT_31 = 0x0000000080000000,
      TAL_LOG_CAT_32 = 0x0000000100000000,
      TAL_LOG_CAT_33 = 0x0000000200000000,
      TAL_LOG_CAT_34 = 0x0000000400000000,
      TAL_LOG_CAT_35 = 0x0000000800000000,
      TAL_LOG_CAT_36 = 0x0000001000000000,
      TAL_LOG_CAT_37 = 0x0000002000000000,
      TAL_LOG_CAT_38 = 0x0000004000000000,
      TAL_LOG_CAT_39 = 0x0000008000000000,
      TAL_LOG_CAT_40 = 0x0000010000000000,
      TAL_LOG_CAT_41 = 0x0000020000000000,
      TAL_LOG_CAT_42 = 0x0000040000000000,
      TAL_LOG_CAT_43 = 0x0000080000000000,
      TAL_LOG_CAT_44 = 0x0000100000000000,
      TAL_LOG_CAT_45 = 0x0000200000000000,
      TAL_LOG_CAT_46 = 0x0000400000000000,
      TAL_LOG_CAT_47 = 0x0000800000000000,
      /* 48 thru 62 are reserved for future use */
      TAL_LOG_CAT_TAL = 0x0001000000000000,
      TAL_LOG_CAT_DX = 0x0002000000000000,
      //  TAL_LOG_CAT_51 = 0x0008000000000000, // TBB
      TAL_LOG_CAT_ALL = 0xFFFFFFFFFFFFFFFF
    } TAL_LOG_CATEGORY;
#endif

#ifndef TAL_DOXYGEN
#define TALX_ID_NAMESPACE 0xFF
#endif // TAL_DOXYGEN

// Set default compiled in log levels and categories.
#ifndef TAL_MAX_COMPILED_IN_LOG_LEVEL
#define TAL_MAX_COMPILED_IN_LOG_LEVEL TAL_LOG_LEVEL_MAX
#endif // TAL_MAX_COMPILED_IN_LOG_LEVEL

#ifndef TAL_COMPILED_IN_CATEGORIES
#define TAL_COMPILED_IN_CATEGORIES TAL_LOG_CAT_ALL
#endif // TAL_COMPILED_IN_CATEGORIES

#ifdef _MANAGED
#ifndef INHIBIT_MANAGED_DECL
#if defined(__TALTRACE_H__)
} // namespace GT {
} //  namespace Tal {
#else  // !__TALTRACE_H__
}
#endif // !__TALTRACE_H__
#endif // INHIBIT_MANAGED_DECL
#endif //_MANAGED

#ifdef _MANAGED
#ifndef INHIBIT_MANAGED_DECL
#define INHIBIT_MANAGED_DECL
#include "public\tal_structs.h"
#undef INHIBIT_MANAGED_DECL
#endif // INHIBIT_MANAGED_DECL
#endif //_MANAGED
#undef __MANAGED_DECL_H__

#endif // TAL_STRUCTS_H

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
