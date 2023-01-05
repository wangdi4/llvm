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

#ifndef TAL_H_WITH_API
#error "tal_api.h can only be included from TAL.h"
#endif //! defined(TAL_H_WITH_API)

#ifndef TAL_API_H
#define TAL_API_H

/************************************************************************/
/************************************************************************/
/** \defgroup Overview */
/** \defgroup CompileAndLink Compiling and Linking */
/** \defgroup Basics API Concepts*/
/** \defgroup TALX TAL C++ Macros */
/** \defgroup Tasks */
/** @{ */
/** \defgroup BeginTask */
/** \defgroup BeginNamedTask */
/** \defgroup EndTask */
/** @} */
/** \defgroup Relations */
/** \defgroup VirtualTasks Virtual Tasks */
/** \defgroup Counters Software and Hardware Counters */
/** \defgroup EventsAndMarkers Events and Markers */
/** \defgroup Parameters Parameters */
/** \defgroup Triggers Triggers */
/** \defgroup Misc Advanced Control APIs */
/** \defgroup Optimization Optimizing and Managing Overhead */
/************************************************************************/

/************************************************************************/
/** \addtogroup Overview
 **
 ** Platform View and TAL API Overview.
 **
 ** Intel(R) GPA Platform View is an instrumentation-based tool. It obtains its
 *data through the TAL API, which
 ** will be used synonymously with Platform View throughout the remainder of
 *this document. Although the tool
 ** ships with key software already instrumented, you will probably want to
 *instrument your application as well. This document
 ** will walk you through the most basic instrumentation tasks, as well as the
 *most advanced.
 **
 ** If you can't seem to get things
 ** working, consult the "Troubleshooting" section of the Intel(R) Graphics
 *Performance Analyzers help file. It describes
 ** common instrumentation pitfalls our users experience and their related
 *solutions.
 **
 ** The TAL API is divided into two broad categories: tracing APIs that actually
 *store data
 ** about your application into the trace file, and control APIs. The tracing
 *APIs are extremely lightweight,
 ** inline functions --- all they do is serialize your trace data directly.
 **
 ** The TAL tracing APIs have a number of different overloads (with explicit
 *versus implicit log levels, for example) and also in
 ** variants (explicit timestamps versus implicit timestamps). Broadly speaking,
 *the unadorned version of each API provides the basic capability, beginning a
 *task, for example.
 ** The variants provide more detailed control over the basic capability at
 *incremental cost compared to an additional tracing call.
 ** For more information, consult the per-function documentation, see the
 *section on API concepts.
 **
 ** While TAL is a fully-functional C API, TAL also provides C++-only
 *capabilities to simplfy common instrumentation tasks. For example, you can use
 *the TAL_SCOPED_TASK class to automatically call TAL_BeginTask/TAL_EndTask.
 **
 ** To understand how these different APIs fit together, it is best to walk
 *through sample code. In the code
 ** below, an application has been instrumented with a few tasks associated with
 *its
 ** toplevel functions. We have also added a "detailed" level of logging so that
 *you
 ** can dynamically obtain/opt-out of detailed logging information from the
 *application.
 ** \code
 **  DWORD WINAPI mythread(LPVOID);
 **  // Define some logging categories for our code.
 **  // Logging categories can be added to specific TAL API calls to allow them
 **  // to be turned on and off at runtime and/or compile time.
 **  #define DETAILED_INFO TAL_LOG_CAT_2
 **  void main(int, char* argv[]) {
 **    // Automatically creates a task in TAL called main()
 **    TAL_SCOPED_TASK();
 **
 **    // tell TAL that our familiar name for TAL_LOG_CAT_2 is DETAILED_INFO.
 **    // in the configuration options of the GUI, you will see DETAILED_INFO in
 *place
 **    // of LOG_CAT_2, helping you to remember your logging configuration
 *options.
 **    TAL_DescribeCategoryMask(DETAILED_INFO, "DETAILED_INFO", "Details about
 *application execution.");
 **
 **    // save the name of the app's exe, but only if we've configured
 **    // TAL to capture the DETAILED_INFO category of data. This can be
 **    // done at compile time using TAL_COMPILED_IN_CATEGORIES or at runtime
 **    // using the GUI.
 **    TAL_ParamStr(TAL_LOG_LEVEL_1, DETAILED_INFO, "ExeName", argv[0]);
 **
 **    // Now we'll create a thread just to show off multi-threaded tracing in
 *TAL
 **    {
 **      // We might be curious about the cost of CreateThread. We add tracing
 *to do the measurement.
 **      TAL_SCOPED_TASK_NAMED("CreateThread");
 **      ::CreateThread(NULL, 0, mythread, 0, 0, NULL);
 **    }
 **    for(;;) {
 **       Sleep(30);
 **    }
 **  }
 **  DWORD WINAPI mythread(LPVOID) {
 **    // Set the name of this thread so it shows  up in the GUI as something we
 *expect
 **    TAL_SetThreadName("mythread");
 **
 **    for(;;) {
 **      // Issue some basic tasks between sleeps. These will accumulate in the
 *per thread trace buffer.
 **      // This time, we're using explicit beginning and ending of tasks, just
 *to show how it works.
 **      TAL_BeginNamedTask("mythread_iteration");
 **      Sleep(150);
 **      TAL_EndTask();
 **    }
 **  }
 ** \endcode
 **
 **/

/************************************************************************/
/** \addtogroup CompileAndLink
 ** Compiling and Linking
 **
 ** \section Compiling Compiling, Linking and Initializing Platform View
 ** Before you begin instrumenting your application, you will need to do a few
 *routine enabling tasks. In practice, this should
 ** involve only two key steps:
 **   -# Make a few tweaks to your build system, so you can reach the TAL
 *headers and libraries
 **   -# Tell TAL when it should send data out to disk/network
 **
 ** \section TheBasics The Basics
 ** Add "#include <tal.h>" to every file that you want to instrument. The tal
 *headers are in the <INSTALL_DIR>\\sdk\\include
 ** directory, where <INSTALL_DIR> is the directory in which you installed GPA.
 **
 ** Typically, our customers add TAL to their
 ** "profile build." To turn off TAL instrumentation in your other
 *configurations, e.g. debug and release, define the
 ** preprocessor symbol TAL_DISABLE (preferably on the compiler command line) in
 *each of those configurations. This will
 ** cause all TAL calls to be 100% compiled out of your application.
 **
 ** \subsection Linking Linking
 ** The TAL library is available for 32-bit and 64-bit Microsoft Windows, and is
 *available in the
 ** <INSTALL_DIR>\\sdk\\libs directory in the following configurations:
 **   Visual Studio 2008 Compatible libraries.
 **   - tal_dd.lib: Dynamic C runtime with Debug symbols
 **   - tal_dr.lib: Dynamic C runtime without Debug symbols
 **   - tal_sd.lib: Static C runtime with Debug symbols
 **   - tal_sr.lib: Static C runtime without Debug symbols
 **
 **   Visual Studio 2005 Compatible libraries.
 **   - tal_dd_vs2005.lib: Dynamic C runtime with Debug symbols
 **   - tal_dr_vs2005.lib: Dynamic C runtime without Debug symbols
 **   - tal_sd_vs2005.lib: Static C runtime with Debug symbols
 **   - tal_sr_vs2005.lib: Static C runtime without Debug symbols
 **
 ** The TAL library you link to your application is a small stub library, with
 *the bulk of the TAL collector runtime
 ** functionality residing in a separate dynamically loaded runtime library.
 ** Depending on whether your machine has GPA installed or not, these import
 *libraries will behave in one of two ways:
 **   -# If run on a machine with GPA installed: TAL tracing will automatically
 *be enabled.
 **   -# If run on a machine without GPA installed: TAL tracing will be silently
 *disabled. The overhead of TAL instrumentation will be
 **      minimal.
 **
 ** \subsection Init Initialization
 ** TAL does not require any init or cleanup calls; it will automatically
 *initialize itself for every thread. This allows you to put
 ** trace calls anywhere and at any time in your application, even inside
 *constructors.
 **/

/************************************************************************/
/** \addtogroup Basics
 ** \section DisablingAndFiltering Disabling TAL, Logging Levels and Categories
 **
 ** In a large codebase, it is often necessary to fine-tune which types of
 *instrumentation are enabled at a given time.
 ** This is like "verbose versus minimal" features that exist in typical logging
 *systems. TAL provides similar capabilities via its logging level and category
 ** system. Briefly, you can include a level and category flag in every TAL API
 *call, for example:
 ** \code
 ** TAL_BeginNamedTask(TAL_LOG_LEVEL_1, TAL_LOG_CAT_1, "BasicTask")
 ** \endcode
 **
 ** Two defines are provided, TAL_MAX_COMPILED_IN_LOG_LEVEL and
 *TAL_COMPILED_IN_CATEGORIES. Levels less than or equal to the provided value,
 *or category masks contained in the compiled-in mask, will be checked at
 *runtime. Levels above or not in
 ** the mask will be compiled out completely.
 **
 ** Typically, we see our users use TAL log levels to switch between overall
 *workflows. For example, they may set up something like:
 ** - LEVEL_1 "overall performance triage, 5% or lower overhead"
 ** - LEVEL_2 "detailed performance, basic debug, 5%+ depending on categories"
 ** - LEVEL_3 "detailed debugging information"
 **
 ** <b>Intel uses categories 33 and above for Intel-supplied libraries that
 *contain instrumentation.</b> If you use categories 1 thru 32, you will
 ** be able to avoid collisions with any Intel-supplied software.
 **
 ** To help remembering the mapping of categories onto specific cateogry
 *numbers, we recommend two best-practices:
 ** -# Define your own categories using #defines in your code, e.g.
 ** \code
 ** #define MY_CATEGORY_BASIC TAL_LOG_CAT_1
 ** #define MY_CATEGORY_GFX TAL_LOG_CAT_2
 ** \endcode
 ** -# Implement next to this define a static inline function wherein for each
 *category you call TAL_DescribeCategoryMask().
 ** -# Call this function from your main function.
 **
 ** This will enable you to refer to categories in the TAL configuration files
 *and the Platform View using your given names,
 ** rather than masks.
 **
 ** \section API Variants
 **
 ** The TAL APIs and their overloads provide variance along two key axes:
 ** -# Per-call logging levels and categories
 ** -# Explicit or implicit trace buffers
 **
 ** When you are first instrumenting your code, you just want the data to appear
 *without worrying about categorizing it. Thus, in addition to
 ** APIs that take explicit logging categories and levels, TAL provides implicit
 *forms. In this form, level defaults to TAL_LOG_LEVEL_1 and category to
 *TAL_LOG_CAT_ALL.
 **
 ** When a TAL call is ready to store data, it does so into a per-thread trace
 *buffer called the TAL_TRACE. While TAL can automatically
 ** obtain the trace handle for you, you can get reductions in TAL API overhead
 *by reusing the TAL_TRACE* pointer across function calls. Hence,
 ** we provide versions of the API that take an explicit trace buffer, for when
 *you're optimizing, and an implicit one, for when you just want to instrument
 *code.
 **
 ** The net result of this flexibility is that each basic TAL API has a number
 *of different overloads. For example, the TAL_BeginNamedTask API comes in the
 *following forms:
 ** -# Plain:
 **    <code>TAL_BeginNamedTask(const char* name)</code>
 ** -# Explicit filtering based on log level and category:
 **    <code>TAL_BeginNamedTask(TAL_LOG_LEVEL lvl, TAL_UINT64 cat, const char*
 *name)</code>
 ** -# Explicit trace
 **    <code>TAL_BeginNamedTask(TAL_TRACE *t, const char* name)</code>
 ** -# Explicit trace and log levels:
 **    <code>TAL_BeginNamedTask(TAL_TRACE *t, TAL_LOG_LEVEL lvl, TAL_UINT64 cat,
 *const char* name)</code>
 ** -# Explicit trace, log levels, no overload:
 **    <code>TAL_BeginNamedTaskFiltered(TAL_TRACE *t, TAL_LOG_LEVEL lvl,
 *TAL_UINT64 cat, const char* name)</code>
 **
 ** When tal.h is included from a C file, only the third and fifth functions are
 *available.
 **
 ** \section Variants Context-specific Variants
 ** TAL functions come in a number of context-specific variants. A few examples:
 ** - <b>Ex</b>, e.g. <code>TAL_BeginNamedTaskEx(TAL_TRACE* t, const char* name,
 *TAL_UINT64 ts)</code>:
 **      functions that implicitly include a timestamp come in an explicit form
 *that accepts a custom timestamp.
 ** - <b>H</b>, e.g. <code>TAL_BeginNamedTaskH(TAL_TRACE* t, TAL_STRING_HANDLE
 *h)</code>:
 **      functions that accept const char* also come in a form that uses a
 *TAL_STRING_HANDLE. These are much more efficient than
 **      their const char* formats.
 ** - <b>WithID</b>, e.g. <code>TAL_BeginNamedTaskWithID(TAL_TRACE* t, const
 *char* name, TAL_ID id)</code>: functions that create
 **      an object that can be referenced using the TAL_AddRelation() API can be
 *created with an ID. For more details on this,
 **      please see the \ref Relations APIs or the Draw Calls case study.
 **
 ** \subsection TAL_STRING_HANDLE TAL_STRING_HANDLE
 **
 ** Every time you make a TAL API call that takes a const char* parameter, the
 *TAL collector copies the string directly into its
 ** TAL_TRACE buffers. For functions called a few times per second, this is no
 *big deal. But, when your function is called hundreds
 ** of thousands of times per frame, a huge amount of data can be generated.
 *This will slow down your capture enormously.
 **
 ** To keep file sizes small, replace all const char* arguments in frequently
 *occurring functions with TAL_STRING_HANDLEs.
 ** These handles represent strings that have been stored into the TAL collector
 *using the TAL_GetStringHandle() API, and can
 ** be reused across the life of the application.
 **
 **  To use TAL_STRING_HANDLEs most efficiently, use the following design
 *pattern:
 **  \code
 **  void foo()
 **  {
 **     static TAL_STRING_HANDLE h = TAL_GetStringHandle("foo");
 **     TAL_BeginNamedTaskH(h);
 **     ...
 **     TAL_EndTask();
 **  }
 **  \endcode
 **
 **  We sometimes see people wrap this trick into a macro, for example:
 **  \code
 **  #define TAL_BeginNamedTaskP(n) static TAL_STRING_HANDLE __h =
 *TAL_GetStringHandle(n);
 **  TAL_BeginNamedTask(__h);
 **  \endcode
 **
 ** \subsection NamingThreads Naming Threads
 ** Although optional, if you would like your threads to have names in the GUI,
 *you need to make TAL aware of your thread names.
 ** To do this, call TAL_SetThreadName() on each thread that you wish to name.
 **
 **/
/** \addtogroup Optimization
 ** In 90% of cases, minimizing I/O will be your highest-leverage optimization
 *task: with just a few lines of code,
 ** you can make TAL generate a tremendous amount of data. When the data gets
 *too large, TAL will become I/O bound,
 ** spending its time writing data to disk rather.
 **
 ** An indicator of being I/O bound is having large tasks in the Platform View
 *named
 ** "TAL_SendTraces" and "TAL_Flush". If these tasks are a large percentage of
 *your frame time, consult the \ref MinimizingFileSize
 ** section for steps you can take to avoid I/O bottlenecks.
 **
 ** In an ideal world, it would be possible to leave TAL instrumentation
 *compiled into your code at all times. In practice, of
 ** course, TAL's overhead is small, but still nonzero. This section will show
 *you how to keep that overhead to a manageable level.
 **
 ** If you are curious about the overhead of TAL calls, run the
 *TAL_PerformanceTest() function.
 ** This will run a simple set of benchmarks on your system using the TAL APIs,
 *and report back their estimated
 ** cost. Typically, each individual TAL call costs about 40 clocks on
 *out-of-order cores.
 **
 ** \section MinimizingFileSize Minimize File Size
 ** The key challenge for instrumenting high-performance code is keeping our
 *bandwidth
 ** requirements for the trace within the limits of the network connection.
 **
 ** The most common reason for big traces is having too much string data in the
 *trace file. For example,
 ** <code>TAL_BeginNamedTask("12345789012345")</code> will consume 28 bytes in
 *the trace buffer: 4 bytes for the TAL opcode,
 ** 8 bytes for the timestamp, and 16 bytes for the actual string.
 **
 ** A key way to reduce your bandwidth is to use string handles. Most TAL APIs
 *that take const char* parameters also accept
 ** TAL_STRING_HANDLE parameters. These string handles are 4 bytes long; for
 *long strings, this can dramatically reduce
 ** your API overhead. However, note that using them comes at a cost in heap
 *memory: You now have to store the handle
 ** itself somewhere. Make sure you do use handles not only for tasks, but also
 *parameters and counters.
 **
 ** Using the TAL_SCOPED_TASK family automatically converts passed in strings
 *into string handles.
 **
 ** \section MoveHeavyTracing Move Heavy Tracing to Log Levels and Categories
 ** During the lifetime of an application, stray TAL calls can creep into your
 *codebase for specific optimization scenarios that
 ** aren't needed by everyone in your team. Rather than disabling these
 *functions, evict them to a higher logging level, or move
 ** them to a specific category. While the function will still be compiled into
 *your code, the check is inlined in the caller and uses
 ** a single pair of cache lines. The overhead of the chosen system will be
 *considerably reduced.
 **
 ** \section Wrapping Take care when Wrapping TAL calls
 ** Some TAL API users have wrapped the TAL API calls so that they can present
 *the logging system in a neutral way to their
 ** own developers. This is a great thing to do, but there are a few caveats
 *from mistakes we've made ourselves when wrapping the API:
 ** -# Make sure you haven't prevented inlining of the TAL function.
 ** -# Make sure you haven't prevented compile-time elimination of log levels
 *and masks. When possible, use the TAL macro
 **    TAL_IS_LOGGABLE to do a very efficient compile-time or runtime test
 *yourself on the TAL log levels.
 ** -# Make sure your wrapping doesn't introduce additional branches or tests.
 *If you couldn't use TAL's logging and category system,
 **    please let us know so we can learn from your experience!
 **
 ** \section UseThreadLocalCopy Explicit use of TAL_TRACE* handle
 **
 ** When you make a call like TAL_BeginTask("Foo"), TAL has to do three things
 *behind the scenes:
 ** -# Retrieve the per-thread storage area for the trace data
 ** -# Determine whether the function call should be traced. If you had passed a
 *log level, for example, that was turned off, TAL would return early here.
 ** -# Push the actual traced data into the trace buffer
 **
 ** The first step, retrieving the per-thread storage area, can cost upwards of
 *20-30 clocks, sometimes more depending on your operating system.
 ** In some performance intensive cases, this may be a problem for you.
 **
 ** Every TAL function comes with a variant that takes an explicit trace buffer
 *as an argument. For example, TAL_BeginNamedTask(const char*) also comes
 ** in the explicit TAL_BeginNamedTask(TAL_TRACE*, const char*) variant. To
 *obtain your per-thread trace bufer handle, call the TAL_GetThreadTrace() API.
 *This
 ** API will return a different value depending on the thread from which it is
 *called, so never store it to a global variable unless that variable is itself
 *marked
 ** as thread-local.
 **
 ** One optimization you can do to reduce API overhead is to re-use the
 *TAL_TRACE* handle across TAL calls within a single function. For example:
 ** \code
 **   void MyFunction(int arg)
 **   {
 **      TAL_TRACE* trace = TAL_GetThreadTrace();
 **      TAL_BeginNamedTask(trace, "MyFunction");
 **      TAL_Parami(trace, "Arg", arg);
 **      TAL_EndTask(trace);
 **   }
 ** \endcode
 ** In comparison to unoptimized code, this function will save 2  calls to
 *TAL_GetThreadTrace(), amounting to anywhere from 20 to 50 clocks of savings,
 ** depending on your platform.
 **
 ** Some high-performance code passes around a "thread state" structure between
 *different functions. If your code does this, then you can
 ** extend the optimization approach shown here to pass the trace explicitly
 *throughout your codebase.
 **
 ** \section HoistCounters Hoist Counters to Outer Loops
 ** A common instrumentation bug is to add TAL_AddCounter functions within
 *tightly nested loops. The following example is going to be
 ** slow:
 ** \code
 ** void Foo()
 ** {
 **     for(int i 0; i < 1000; ++i)
 **     {
 **         if(something(i)) TAL_IncCounter("SomethingOccurred");
 **     }
 ** }
 ** \endcode
 ** If this counter increments frequently, TAL will end up storing a huge number
 *of opcodes into the trace buffer, wasting cycles on
 ** the saving that data to file.
 **
 ** When you encounter such functions, prefer a solution like the following:
 ** \code
 ** void Foo()
 ** {
 **     int nSomethings = 0;
 **     for(int i 0; i < 1000; ++i)
 **     {
 **         if(something(i)) nSomethings++;
 **     }
 **     TAL_AddCounter("SomethingOccurred", nSomethings);
 ** }
 ** \endcode
 **
 **
 ** \section SpecializeInner Specialize Inner Functions if Hoisting isn't
 *Possible
 ** In some cases, the hoisting trick isn't possible, yet you still want the
 *ability to dynamically enable/disable the tracing.
 **
 ** In this case, one alternative you have at your disposal is function
 *specialization. The simplest way we have seen to do this
 ** is to convert the function to a templated form. Starting with a function
 *like the following:
 ** \code
 ** void Foo()
 ** {
 **     TAL_AddCounter(x);
 **     ...
 ** }
 ** void SomeFunc()
 ** {
 **     Foo()
 ** }
 ** \endcode
 ** becomes
 ** \code
 ** template <bool b> void Foo()
 ** {
 **     if(b) TAL_AddCounter(x);
 **     ...
 ** }
 ** void SomeFunc()
 ** {
 **     if(TAL_IS_LOGGABLE(lvl,cat))
 **     {
 **         Foo<true>();
 **     }
 **     else
 **     {
 **         Foo<false>();
 **     }
 ** }
 ** \endcode
 **/

/************************************************************************/
/** \addtogroup Tasks
 ** Tasks.
 **
 ** Tasks are the fundamental data elements in a TAL trace. A task
 ** is a logical group of work on a specific thread. Tasks can nest; thus, tasks
 *typically correspond to functions,
 ** scopes, or a case block in a switch statement.
 ** \code
 ** void EverythingFn(int arg) {
 **   TAL_BeginNamedTask("EverythingFn");
 **   TAL_BeginNamedTask("SubTask 1");
 **   sleep(100);
 **   TAL_EndTask();
 **   if(arg == 7) {
 **     TAL_BeginNamedTask("Arg7SpecialCase");
 **     SpecialCaseFn();
 **     TAL_EndTask();
 **   }
 **   TAL_EndTask();
 ** }
 ** \endcode
 ** TAL tasks come in two basic variants: named tasks and function tasks. Named
 *tasks are identified with
 ** strings (as in TAL_BeginNamedTask("Foo")). Function tasks are identified
 *with function pointers, as in:
 ** \code
 ** void Fn() {
 **    TAL_BeginTask(Fn);
 **    TAL_EndTask();
 ** }
 ** \endcode
 **
 ** For named tasks, you can use the __FUNCTION__ preprocessor macro to simplify
 *task creation, e.g. TAL_BeginNamedTask(__FUNCTION__);
 ** Function tasks are useful in a task system where a function name isn't
 *available. For example:
 ** \code
 **   void scheduler_thread() {
 **      while(true) {
 **         Task* t = scheduler_dequeue();
 **         TAL_BeginTask(t->fn);
 **         t->fn();
 **         TAL_EndTask();
 **      }
 **   }
 ** \endcode
 **
 ** From a performance perspective, the function-pointer (TAL_BeginTask()) and
 *string-handle (TAL_BeginNamedTaskH()) versions of the task APIs are
 ** the fastest task API variants, since they don't have to push a full string
 *onto the trace.
 **
 ** In some cases, TAL_BeginTask and TAL_EndTask calls are placed in totally
 *separate functions.
 ** This is OK, <b>as long as any other tasks stay properly nested with respect
 *to one another.</b>
 **
 ** Bringing these APIs together, the following code is a good example of TAL
 *tasks in action:
 ** \code
 ** void BeginFrame() {
 **   TAL_BeginNamedTask("Frame");
 ** }
 ** void DoWork() {
 **   TAL_BeginTask(((void*)())DoWork);
 **   TAL_EndTask();
 ** }
 ** void EndFrame() {
 **   TAL_EndTask();
 ** }
 ** void Main() {
 **    BeginFrame();
 **    DoWork();
 **    EndFrame();
 ** }
 ** \endcode
 **
 ** Named tasks put the contents of their strings directly into your trace file.
 *For some functions,
 ** say main(), this isn't a big deal, since it doesn't get called often. But,
 *for tasks that get called
 ** thousands of times in a frame on many threads, for example
 *"RasterizeTriangles," this can pose a problem.
 **
 ** The solution to this is TAL_STRING_HANDLEs. Starting with the following
 *code:
 ** \code
 ** void DoWork() {
 **    TAL_BeginNamedTask("12345789012345");
 **    ...
 **    TAL_EndTask();
 ** }
 ** \endcode
 ** Replace this with:
 ** \code
 ** void DoWork() {
 **    static TAL_STRING_HANDLE _h = TAL_GetStringHandle("12345789012345");
 **    TAL_BeginNamedTaskH(_h);
 **    ...
 **    TAL_EndTask();
 ** }
 ** \endcode
 ** The first example will consume 28 bytes per instance. The string-handle
 *optimized version will
 ** consume 16 bytes.
 **
 ** It is possible to relate tasks to one another or to virutal tasks using the
 *\ref Relations API.
 ** For information on using this API, and the associated "WithID" variants of
 *TAL_BeginTask/TAL_BeginNamedTask,
 ** please read the \ref Relations section.
 **/

/** \addtogroup BeginTask
 ** Capture the beginning of a task.
 **
 ** Capture the beginning of a task.
 ** Functions with the 'Ex' suffix allow the caller to specify an explicit
 *timestamp; functions without the 'Ex' suffix use an internal timestamp.
 **/

/** \addtogroup BeginNamedTask
 ** Capture the beginning of a named task.
 **
 ** Capture the beginning of a named task. A named task can be used to
 ** represent a unit of work that doesn't cleanly map to a function.
 ** Functions with the 'Ex' suffix allow the caller to specify an explicit
 *timestamp; functions without the 'Ex' suffix use an internal timestamp.
 **/

/** \addtogroup EndTask
 ** Capture the end of the current task.
 **
 ** Capture the end of the current task. The current task is the one
 ** associated with the most recent BeginTask or BeginNamedTask call with
 ** no corresponding TAL_EndTask or TAL_EndTaskEx call.
 ** Functions with the 'Ex' suffix allow the caller to specify an explicit
 *timestamp; functions without the 'Ex' suffix use an internal timestamp.
 **/

/**
 ** \addtogroup VirtualTasks
 ** Virtual Tasks APIs.
 **
 ** Virtual tasks are for defining logical groups of work in your application
 *that don't themselves have
 ** a logical toplevel task.
 **
 ** A virtual task has all of the basic behaviors of a regular TAL task ---
 ** you can add parameters and counters to it, relate it to other tasks with the
 *TAL relations API,
 ** and so on.
 **
 ** The unique property of virtual tasks is that they themselves do not consume
 ** execution resources on the machine being measured. Consider, for example,
 ** a "rendering pass" in graphics --- logically, this consists of the work for
 ** a number of draw calls, yet nowhere in the execution of the system will you
 *see
 ** a task running that is the "rendering pass." In this context, the rendering
 *pass is a virtual task.
 **
 ** Why use virtual tasks? By definition, virtual tasks have no duration.
 *However, using
 ** the relations API, you can attach children to a virtual task and by doing
 *so,
 ** tell the GUI about the work associated with the task. So, to answer the
 *question,
 ** "how much work did frame 7" do, you instrument the code in the following
 *way:
 **  -# Create the virtual tasks for the current frame
 **  -# For every task that contributes time to the overall frame's work, relate
 *it back to
 **     the virtual task using the relations API.
 ** For more information on the relations API, please see the Relations section
 *of this document.
 **
 ** The API for virtual tasks is nearly identical to regular tasks: there is a
 *BeginNamedVirtualTask and EndVirtualTask API
 ** pair that you use to start and end a task. The key difference between the
 *VTask API and the regular Task API is that
 ** <b>virtual tasks ALWAYS have IDs</b>. To understand why the API is done this
 *way, its easiest to work through an example.
 **
 ** Recall that we use virtual tasks to measure abstract concepts like "Render
 *Pass Duration." Consider the following
 ** simplified code for a graphics renderer --- the draw call creates the vertex
 *task, then the vertex task creates the pixel shading task.
 ** \code
 ** void DrawIndexedPrimitive(DrawCall* draw) {
 **    EnqueueTask(DoVertexShadingWork, draw);
 ** }
 ** void DoVertexShadingWork(DrawCall* draw) {
 **    // ... shade vertices...
 **    EnqueueTask(DoPixelShadingWork, draw);
 ** }
 ** void DoPixelShadingWork(DrawCall* d) {
 **    // here is a task where we do pixel shading work...
 ** }
 ** \endcode
 ** To obtain the render pass costs, we first have to "hook" the concept of a
 *render pass within our application. A low-overhead, simple place to
 ** insert this would be in the original draw call with a static variable and a
 *quick check:
 ** \code
 ** void DrawIndexedPrimitive(DrawCall* draw) {
 ** #if TAL_ENABLED
 **    static RenderPass* sLastRP = NULL;
 **    if(draw->RenderState->RenderPass != sLastRP) {
 **       sLastRP = renderState->RenderPass;
 **       // the render pass has changed...
 **    }
 ** #endif
 **    EnqueueTask(DoVertexShadingWork, draw);
 ** }
 ** \endcode
 ** Now we know when the pass has changed. Ideally, we'd be able to simply
 *create a task
 ** when the renderpass begins and end it when it changes and we'd be done:
 ** \code
 ** void DrawIndexedPrimitive(DrawCall* draw) {
 ** #if TAL_ENABLED
 **    static RenderPass* sLastRP = NULL;
 **    if(draw->RenderState->RenderPass != sLastRP) {
 **       if(sLastRP != NULL) {
 **          TAL_EndTask(); // end the previous RenderPass task. WARNING: THIS
 *MEASURES THE WRONG THING.
 **       sLastRP = renderState->RenderPass;
 **       TAL_BeginNamedTask("RenderPass"); // WARNING: THIS MEASURES THE WRONG
 *THING.
 **    }
 ** #endif
 **    EnqueueTask(DoVertexShadingWork, draw);
 ** }
 ** \endcode
 ** Unfortunately, this doesn't actually measure RenderPasses at all! Notice how
 ** we <b>Enqueue</b> the vertex shading tasks to another thread! All that we'd
 *be timing
 ** in this example would be time on the DrawIndexedPrimitive thread spent
 *enqueuing these tasks.
 **
 ** To do this properly, we need to use the TAL Relations API. Recall that this
 *API
 ** allows you to relate tasks across disparate threads. To measure RenderPass
 *duration,
 ** we will create a tiny virtual task that represents the RenderPass itself.
 *Then,
 ** we will add each of the individul tasks (drawing function, vertex shading,
 *and pixel shading)
 ** to that virtual task. This will give it duration.
 **
 ** Remember that virtual tasks only come in forms that take IDs, e.g.
 *TAL_BeginNamedVirtualTaskWithID. The reason
 ** is that all known uses of virtual tasks involve relating them to other tasks
 *with relations. Since this requires
 ** the virtual task to have an ID, this the only form that the API takes.
 **
 ** The final code for tracking render pass duration using virtual tasks and
 *relations is as follows:
 ** \code
 ** #define TAL_RENDERPASS_NS 3 // choose a number s.t. rendertagets have their
 *own namespace...
 ** void DrawIndexedPrimitive(DrawCall* draw) {
 ** #if TAL_ENABLED
 **    static RenderPass* sLastRP = NULL;
 **    TAL_ID rtID = TAL_MakeID(TAL_RENDERPASS_NS, sLastRP, 0);
 **    if(draw->RenderState->RenderPass != sLastRP) {
 **       sLastRP = renderState->RenderPass;
 **       // create the ID. This must happen before we use the ID.
 **       TAL_CreateID(rtID);
 **       // Now, we create the virtual task.
 **       // Notice how the virtual task is begun and ended IMMEDIATELY.
 **       // This is becasue the real work will be done in child tasks
 **       // Since this is the case, the actual time spent between the Begin and
 *EndVirtualTask
 **       // is not relevant for profiling. For *this specific reason*, we will
 *make it
 **       // a virtual task.
 **       TAL_BeginNamedVirtualTaskWithID("RenderPass", rtID);
 **       TAL_Param2i("Size", sLastRP->Width, sLastRP->Height); // Virtual tasks
 *can have parameters, just like tasks
 **       TAL_EndVirtualTask();
 **    }
 ** #endif
 **    // Now, create a task for the drawing thread's work.
 **    // Notice that we create this AFTER the virtual task.
 **    // This is *mandatory* because we are about to add a reference to the
 *virtual task.
 **    // You cannot do this until the TAL_CreateID call for the task has been
 *issued.
 **    TAL_BeginTask("DrawIndexedPrimtive");
 **
 **    // Make this task a child of the RenderPass task
 **    TAL_AddRelationThis(TAL_RELATION_IS_CHILD_OF, rtID);
 **
 **    // our actual work
 **    EnqueueTask(DoVertexShadingWork, draw);
 **
 **    // end the draw call task, we're done there. We can still add work to the
 *RenderPass, however...
 **    TAL_EndTask();
 ** }
 **
 ** void DoVertexShadingWork(DrawCall* draw) {
 **    // a helpful TAL helper macro that will automatically
 **    // issue TAL_BeginNamedTask("DoVertexShadingWork") and a TAL_EndTask for
 *us.
 **    TAL_SCOPED_TASK();
 **
 **    // add this task to the RenderPass as well
 **    TAL_ID rtID = TAL_MakeID(TAL_RENDERPASS_NS, sLastRP, 0);
 **    TAL_AddRelationThis(TAL_RELATION_IS_CHILD_OF, rtID);
 **
 **    // ... shade vertices...
 **    EnqueueTask(DoPixelShadingWork, draw);
 ** }
 **
 ** void DoPixelShadingWork(DrawCall* d) {
 **    TAL_SCOPED_TASK();
 **
 **    // add this task to the RenderPass as well
 **    TAL_ID rtID = TAL_MakeID(TAL_RENDERPASS_NS, sLastRP, 0);
 **    TAL_AddRelationThis(TAL_RELATION_IS_CHILD_OF, rtID);
 **
 **    // here is a task where we do pixel shading work...
 ** }
 ** \endcode
 **
 ** Unfortunately, that's not quite all. For correctness, every time you call
 *TAL_CreateID, you need
 ** to later call TAL_RetireID. The rule is that once you have retired the ID,
 *you cannot use it in any other
 ** API calls. The code above lacks a retire ID location because we don't have a
 *clean place to put it: we can't
 ** put it in the DrawIndexedPrimitive task --- when the RenderTarget changes on
 *that thread, there still may be
 ** outstanding pixel shading work.
 **
 ** The best way to fix this is to find when the renderPass is truly completed.
 *Sometimes you will find
 ** this information in your overall task scheduler, or sometimes it will be
 *more easily accessible. Our favorite
 ** solution is when your ID is derived from a specific allocation for the task
 *--- when this happens, as is the case
 ** with this RenderPass example, we can just move the ID calls to the
 *RenderPass constructor/destructor:
 ** \code
 ** class RenderPass {
 ** public:
 **    RenderPass() {
 **       TAL_CreateID(TAL_MakeID(TAL_RENDERPASS_NS, this, 0));
 **    }
 **    ~RenderPass() {
 **       TAL_RetireID(TAL_MakeID(TAL_RENDERPASS_NS, this, 0));
 **    }
 ** }
 ** \endcode
 ** This is not always the case, so you may have to get clever. If you're just
 *implementing your task scheduling system,
 ** try to design this in from the beginning: having a clear place where you
 *know that all of a subtask's work is complete is
 ** useful, not only for TAL, but general profiling and debugging in general.
 **
 ** Note that the Platform View tries very hard to be forgiving of you if you
 *don't call TAL_RetireID, but no guarantees are made.
 **/

/** \addtogroup Relations
 ** APIs for relating tasks to one another.
 **
 ** The Relations APIs tell TAL that two tasks are related to one another. By
 ** tying two tasks together, you allow the Platform View  to distill your
 *detailed task information
 ** into higher-level concepts that are more closely related to your own APIs.
 **
 ** For example, on some architectures, a single Direct3D draw call can lead to
 *dozens,
 ** if not hundreds of tasks on other threads. A common profiling question is
 *"how long was that draw call."
 ** On a multithreaded architecture, answering this requires summing up the
 *durations of these hundreds of
 ** of tasks. The TAL Relations APIs help automate that process for you.
 **
 ** Think of all your tasks and virtual tasks as nodes in a graph. TAL relations
 *allow you
 ** to create edges of certain types between nodes, say a "Parent" edge from X
 *to Y to signify
 ** that Y is the parent task of X.
 **
 ** To create these connections between tasks or vtasks, at least one of them
 *will need an identifier. This identifier
 ** allows you to refer to the task instance before, or after, it has actually
 *run.
 ** We call these identifiers TAL_IDs. A TAL_ID is a 136 bit struct consisting
 *of a 8-bit ID namespace, followed by 2 64 bit quantiites.
 ** To conveniently fill in such a struct, use the TAL_MakeID(TAL_BYTE
 *namespace, TAL_UINT64 id_hi, TAL_UINT64 id_lo) API, which will pack the struct
 *and return it.
 **
 ** The namespace portion of an ID is used to identify your family of IDs. In
 *general, every library that
 ** uses IDs should simply create its own namespace. Namespace IDs are
 *statically #defined, but to
 ** ensure they don't collide. You have to be careful when you choose a
 *namespace value --- it can't be in use by another
 ** subsystem. To protect yourself against this happening, call
 *TAL_RegisterIDNamespace once you've chosen a namespace value. This will
 ** let TAL warn you if you have a namespace colission.
 **
 ** <b>Intel reserves ID_NAMESPACEs 128-255 for software libraries it ships.</b>
 *By allocating your namespaces in the 0-127 range, you will
 ** avoid namespace-to-namespace collissions with Intel-provided software.
 **
 ** There are absolutely no constraints on the id_lo and id_hi integers, so long
 *as you can guarantee that in combination they are unique to your
 ** task and your task alone. Sometimes this is easy --- when you have a
 *pointer, e.g. Task*, then TAL_MakeID(ns,taskPtr,0) will suffice.
 ** Sometimes, however, you will need to combine a few integers to guarantee
 *uniqueness. For example, a parallel_for will
 ** often decompose into a "work unit," which itself is several tasks. In this
 *case, the ID of a task is the work unit pointer AND the task's
 ** offset within the work unit.
 **
 ** Summarizing, to tie two tasks X and Y together, you follow the following
 *steps:
 ** -# Tell TAL that the identifiers for X and Y exist using TAL_CreateID. Any
 *use of X in other API calls may not happen until X is created. The same
 *applies to Y.
 ** -# Now, in any order, you may:
 **   -# Create a task X with that identifier. This can be done using the WithID
 *variant of the
 **    task API, e.g. TAL_BeginNamedTaskWithID() API.
 **   -# Within Y, add a relation to X using TAL_AddRelationToCurrent()
 ** -# When X and all its related tasks are guaranteed to be complete, tell TAL
 *that X is no longer in use by calling the TAL_RetireID() API.
 **
 ** To see this in action, lets work through a parallel-for implementation, and
 *the challenge of measuring the total cost
 ** of the parallel-for. We begin with the actual parallel-for task:
 ** \code
 **    #define ParForNS 3
 **    void ParallelFor(int base, int count, void(*doWork)(int index)) {
 **        // Tell TAL about this space of identifiers ("namespace") --- this
 *allows
 **        // you to use the full bit range of the int id without colliding with
 *anyone else's identifiers.
 **        static bool ns_registered = false;
 **        if(ns_registered == false) {
 **          ns_registered = true;
 ** TAL_RegisterIdNamespace((TAL_ID_NAMESPACE)ParForNS,"ParallelForNamespace");
 **        }
 **
 **        // Get a unique identifier for this specific parallel-for call
 **        static int sParallelForCounter = 0;
 **        int id = sParallelForCounter++;
 **
 **        // Tell TAL that this ID exists. Without calling this, all references
 *to this ID
 **        // are invalid.
 **        TAL_CreateID(TAL_MakeID(ParForNS, id, 0));
 **
 **        // Now, begin the actual task and associate it with an ID
 **        TAL_BeginNamedTaskWithID("ParallelFor", TAL_MakeID(ParForNS, id, 0));
 **
 **        // this numTasksOutstanding will be used to decide whether the
 *parallel for has completed
 **        int numTasksOutstanding = 0;
 **
 **        // now do the actual ParallelFor work: kick off the child tasks...
 **        int countPerTask = count / GetNumWorkerThreads();
 **        int curBase = 0;
 **        int remaining = count;
 **        while(remaining > 0) {
 **           int curCount = MIN(remaining, countPerTask);
 **           if(curCount == 0) continue; // out of work
 **           AtomicIncrement(&numTasksOutstanding); // add a task to the
 *parallel for status
 **           EnqueueTask(ParallelFor_DoWork, &numTasksOutstanding, id, doWork,
 *curBase, curCount);
 **           curBase += curCount; remaining -= curCount;
 **        }
 **        // our parallel-fork kickoff has completed, but the parallel-for work
 *hasn't
 **        // we will make a subtask to measure how long we wait for completion
 **        TAL_BeginNamedTask("WaitForCompletion");
 **        while(numTasksOutstanding > 0)
 **           YieldTimeslice();
 **        TAL_EndTask();
 **
 **        // The parallel for, including all its child work, is complete. We
 *can end the root parallel-for task
 **        TAL_EndTask();
 **
 **        // Now, we MUST retire the TAL_ID that was used for this parallel for
 **        // Tells TAL that the lifetime of this ID (and the tasks it relate
 *to) have ended
 **        // We can retire here because all the child tasks are known to be
 *done, due to the wait loop above.
 **        TAL_RetireID(TAL_MakeID(ParForNS, id, 0));
 **    }
 ** \endcode
 **
 ** In addition to instrumenting the master parallel-for task, we need
 *instrument the worker task:
 ** \code
 **   void ParallelFor_DoWork(int* numTasksOutstanding, int parallelForID, void
 *(*doWork)(int index), int base, int count) {
 **       // make a task for this worker task. It does not need an ID itself.
 **       TAL_BeginNamedTask("ParallelFor");
 **
 **       // mark this task as a child of the parent parallelFor task created
 *earlier
 **       TAL_AddRelationThis(TAL_RELATION_IS_CHILD_OF, TAL_MakeID(ParForNS,
 *parallelForID, 0));
 **
 **       // do the work for this actual task...
 **       for(int i = base; i < count; ++i) {
 **          doWork(i);
 **       }
 **
 **       // The parallel for implementation above needs to know when all worker
 *tasks have completed their work
 **       AtomicDecrement(numTasksOutstanding);
 **
 **       // we're done, mark us as done
 **       TAL_EndTask();
 **   }
 ** \endcode
 **
 ** All relationships in TAL are symmetric, in the sense that where there is a
 *relation indicating
 ** a relationship X from Task A to Task B, there will be another implied
 *relationship Y that goes from
 ** B to A. For example, if you define a TAL_RELATION_IS_CHILD_OF relationship
 *from A to B, there will be an implied
 ** TAL_RELATION_IS_PARENT_OF relationship from B to A. These implied
 *relationships appear in the Platform View.
 **
 ** In the previous exampe, we suppose our task system's EnqueueTask function
 *returned a pointer, eg <code>Task* EnqueueTask(...)</code>.
 ** Rather than having to add the AddRelation
 ** call to the child task, we could reverse the direction of the relationships
 *around, creating IDs for each
 ** of the child tasks and establishing relationships up-front when we enqueue
 *them. The result is the same in the GUI, but
 ** for some codebases, the resulting instrumentation might be simpler to
 *maintain:
 ** \code
 **    #define ParForNS 3
 **    void ParallelFor(int base, int count, void(*doWork)(int index)) {
 **        // Tell TAL about this space of identifiers ("namespace") --- this
 *allows
 **        // you to use the full bit range of the int id without colliding with
 *anyone else's identifiers.
 **        static bool ns_registered = false;
 **        if(ns_registered == false) {
 **          ns_registered = true;
 ** TAL_RegisterIdNamespace((TAL_ID_NAMESPACE)ParForNS,"ParallelForNamespace");
 **        }
 **
 **        // Now, begin the root parallel for task. We won't need to give this
 *an ID
 **        // because we will be giving the child tasks IDs instead.
 **        TAL_BeginNamedTask("ParallelFor");
 **
 **        // now do the actual ParallelFor work: kick off the child tasks...
 **        int countPerTask = count / GetNumWorkerThreads();
 **        int curBase = 0;
 **        int remaining = count;
 **        std::vector<Task*> childTasks;
 **        while(remaining > 0) {
 **           int curCount = MIN(remaining, countPerTask);
 **           if(curCount == 0) continue; // out of work
 **           AtomicIncrement(&numTasksOutstanding); // add a task to the
 *parallel for status
 **           Task* childTask = EnqueueTask(ParallelFor_DoWork, doWork, curBase,
 *curCount);
 **
 **           // remember all the child tasks
 **           childTasks.push_back(task);
 **
 **           // Create the ID for the child task. This tells TAL that somethign
 *with the given ID exists,
 **           // so that it can begin looking for the matching TAL_BeginTask
 *call.
 **           TAL_ID childID = TAL_MakeID(ParForNS, (TAL_UINT64)childTask, 0);
 **           TAL_CreateID(childID);
 **
 **           // Now, say that the current task is the parent of this child ID
 **           // This allows us to create the relationship even before the child
 *task has run.
 **           TAL_AddRelationThis(TAL_RELATION_IS_PARENT_OF, childID);
 **
 **           // final bit of for loop logic
 **           curBase += curCount; remaining -= curCount;
 **        }
 **        // our parallel-fork kickoff has completed, but the parallel-for work
 *hasn't
 **        // we will make a subtask to measure how long we wait for completion
 **        TAL_BeginNamedTask("WaitForCompletion");
 **
 *WaitForMultipleTasks(childTasks.begin(),childTasks.end()-childTasks.beign(),INFINITE);
 **        TAL_EndTask();
 **
 **        // The parallel for, including all its child work, is complete. We
 *can end the root parallel-for task
 **        TAL_EndTask();
 **
 **        // Now, we MUST retire all the TAL_IDs that we used for the parallel
 *for
 **        // Here, we retire all the child IDs
 **        for(size_t i = 0; i < childTasks.size(); ++i)
 **          TAL_RetireID(TAL_MakeID(ParForNS, (TAL_UINT64)childTasks[i], 0));
 **    }
 ** \endcode
 ** Because we are using the task system's built-in Task* handle, we can
 *simplify the worker task:
 ** \code
 **   void ParallelFor_DoWork(void (*doWork)(int index), int base, int count) {
 **       Task* curTask = GetExecutingTask();
 **
 **       // make a task for this worker task. We will bind it to the ID that
 *was created for us earlier.
 **       TAL_BeginNamedTaskWithID("ParallelFor", TAL_MakeID(ParForNS,
 *(TAL_UINT64)curTask, 0));
 **
 **       // do the work for this actual task...
 **       for(int i = base; i < count; ++i) {
 **          doWork(i);
 **       }
 **
 **       // mark this task as done
 **       TAL_EndTask();
 **   }
 ** \endcode
 **
 ** The most difficult part about using TAL Relations is retiring IDs at the
 *right moment. In TAL, you may not
 ** retire an ID until all tasks that might reference that ID have also
 *completed. In the example earlier, the ID could be
 ** retired in the same function because the root ParallelFor function blocked
 *on completion of all its child tasks.
 **
 ** However, imagine we made a variant on the previous example called
 *AsyncParallelFor, which returned
 ** immediately upon being called, even when child tasks were still outstanding.
 ** In this case, we cannot retire the IDs at the end of the initial task,
 *because there may be tasks still running
 ** that will reference that ID. Such references, after the ID is retired, are
 *considered invalid and ignored by TAL.
 ** The correct behavior is to wait until the LAST worker task completes before
 *retiring the ID. The
 ** following code sample demonstrates this type of asynchronous scenario:
 ** \code
 **    void AsyncParallelFor(int base, int count, void(*doWork)(int index)) {
 **        // Tell TAL about this space of identifiers ("namespace") --- this
 *allows
 **        // you to use the full bit range of the int id without colliding with
 *anyone else's identifiers.
 **        static bool ns_registered = false;
 **        if(ns_registered == false) {
 **          ns_registered = true;
 ** TAL_RegisterIdNamespace((TAL_ID_NAMESPACE)ParForNS,"ParallelForNamespace");
 **        }
 **
 **        // Get a unique identifier for this specific parallel-for call
 **        static int sParallelForCounter = 0;
 **        int id = sParallelForCounter++;
 **
 **        // Tell TAL that this ID exists. Without calling this, all references
 *to this ID
 **        // are invalid.
 **        TAL_CreateID(TAL_MakeID(ParForNS, id, 0));
 **
 **        // Now, begin the actual task and associate it with an ID
 **        TAL_BeginNamedTaskWithID("ParallelFor", TAL_MakeID(ParForNS, id, 0));
 **
 **        // this numTasksOutstanding will be used to decide whether the
 *parallel for has completed
 **        // We will heap allocate this value instead of using the stask,
 *because we will be
 **        // returning immediately after enqueuing work, instead of waiting for
 *all child tasks to complete.
 **        int* numTasksOutstanding = new int();
 **        *numTasksOutstanding = 0; // init it to zero
 **
 **        // now do the actual ParallelFor work: kick off the child tasks...
 **        int countPerTask = count / GetNumWorkerThreads();
 **        int curBase = 0;
 **        int remaining = count;
 **        while(remaining > 0) {
 **           int curCount = MIN(remaining, countPerTask);
 **           if(curCount == 0) continue; // out of work
 **           AtomicIncrement(numTasksOutstanding); // add a task to the
 *parallel for status
 **           EnqueueTask(ParallelFor_DoWork, numTasksOutstanding, id, doWork,
 *curBase, curCount);
 **           curBase += curCount; remaining -= curCount;
 **        }
 **        // In the synchronous parallel for, we would waited here,
 **        // for the tasks to complete. In our case, we can't do that, we must
 *return.
 **        // So, we will have to retire the ID after all outstanding work has
 *completed.
 **        // All we will do here is end the root parallel-for task.
 **        TAL_EndTask();
 **    }
 **    void ParallelFor_DoWork(int* numTasksOutstanding, int parallelForID, void
 *(*doWork)(int index), int base, int count) {
 **       // make a task for this worker task. It does not need an ID itself.
 **       TAL_BeginNamedTask("ParallelFor");
 **
 **       // mark this task as a child of the parent parallelFor task created
 *earlier
 **       TAL_AddRelationThis(TAL_RELATION_IS_CHILD_OF, TAL_MakeID(ParForNS,
 *parallelForID, 0));
 **
 **       // do the work for this actual task...
 **       for(int i = base; i < count; ++i) {
 **          doWork(i);
 **       }
 **
 **       // The parallel for implementation above needs to know when all worker
 *tasks have completed their work
 **       int prevValue = AtomicDecrement(numTasksOutstanding);
 **
 **       // if we were the last task to complete, we will retire the ID
 **       // this is the case when prevValue from the decrement is 1
 **       bool retireID = prevValue == 1;
 **
 **       // we're done, mark us as done
 **       TAL_EndTask();
 **
 **       // We retire the ID here, OUTSIDE the TAL_EndTask. This is important
 *because
 **       // the ID must be retired when no tasks are running that reference
 *that ID. Since this
 **       // task references the ID, we must TAL_EndTask it before retiring.
 **       if(retireID) {
 **         TAL_RetireID(TAL_MakeID(ParForNS, parallelForID, 0));
 **       }
 **   }
 ** \endcode
 **
 ** TAL supports a variety of other types of relationships between tasks,
 *including task-to-task dependencies, tasks that
 ** are continuations of one-another, and more. For the
 ** full list of supported relations, please see the documentation on the \ref
 *_TAL_RELATION enumeration.
 **/

/** \addtogroup Counters
 ** APIs for capturing hardware performance counters and software counters.
 **
 ** The TAL Counters API allows you to track different types of counters that
 *exist
 ** within your application. The API supports both hardware counters and
 *software counters, although hardware-counter support
 ** is not available on Windows at this time.
 ** The software counter APIs allow you instrument your code with any number of
 *counters of your choice.
 **
 ** \section SoftwareCounters Software counters
 ** There are two different kinds of TAL software counters that you can use:
 *absolute counters and relative counters.
 ** An absolute counter is an integer value in your application that you might
 *want to sample periodically, for example,
 ** "TotalHeapSize." The TAL_SampleCounter API records a snapshot of an absolute
 *counter and stores it, plus a timestamp,
 ** within the current task. The Platform View  provides facilities to visualize
 *how these samples change over time.
 ** A relative counter is one that you can increment/add/subtract/decrement, but
 *that has no absolute value and is
 ** evaluated at analysis time within the Platform View. For example, you can
 *use a relative counter to track
 ** the number of triangles being dynamically created:
 **
 ** \code
 ** void ProcessTriangleArray(vector<tri> triangles) {
 **   TAL_BeginNamedTask("ProcessTriangleArray");
 **   TAL_AddCounter("TrianglesSubmitted", triangles.size());
 **   foreach(tri t in triangles) {
 **     if(t.visible) {
 **        TAL_IncCounter("TrianglesVisible"); // warning, slow! See below.
 **     }
 **   }
 **   TAL_EndTask();
 ** }
 ** \endcode
 ** The Platform View evaluates relative counter values by aggregating
 *add/sub/increment/decrement operations across the
 ** current selection. When you are analyzing a trace captured from code like
 *the above example in the tool, if you have a
 ** single ProcessTriangleArray task selected, the value of the
 *"TrianglesSubmitted" counter will be reported as the amount for
 ** just that task. In contrast, if you have all ProcessTriangleArray tasks
 *selected, the value of the "TrianglesSubmitted"
 ** counter will be reported as the sum of the values passed to TAL_AddCounter
 *for the "TrianglesSubmitted" counter.
 **
 ** In the example above, we call TAL_IncCounter within a tight inner loop, yet
 *comment it with "warning,
 ** slow." When you have a tight loop like this, you will get much better
 *performance and much smaller file size
 ** by \b hoisting the counter out of the loop, as follows:
 ** \code
 ** void ProcessTriangleArray(vector<tri> triangles) {
 **   TAL_BeginNamedTask("ProcessTriangleArray");
 **   TAL_AddCounter("TrianglesSubmitted", triangles.size());
 **   int nTrisVisible = 0;
 **   foreach(tri t in triangles) {
 **     if(t.visible) {
 **        nTrisVisible += 1;
 **     }
 **   }
 **   TAL_IncCounter("TrianglesVisible", nTrisVisible); // much faster than
 *example above!
 **   TAL_EndTask();
 ** }
 ** \endcode
 **
 ** TAL counter calls can be placed anywhere in your code, so long as there is
 *at least one open task.
 ** The counter will bind to the most recent BeginTask call. Consider the
 *following example:
 ** \code
 ** void main() {
 **    TAL_BeginNamedTask("main");
 **    a();
 **    b();
 **    TAL_EndTask();
 ** }
 ** void a() {
 **   TAL_IncCounter("MyCounter");
 ** }
 ** void b() {
 **   TAL_BeginNamedTask("b");
 **   a();
 **   TAL_EndTask();
 ** }
 ** \endcode
 ** In this example, when you select the "b" task in the tool, MyCounter will
 *have the value 1, but when you select
 ** the the main() task, MyCounter will have the value 2.
 **
 ** \section CvP Counters vs Parameters
 ** To decide whether to use a TAL counter or a TAL parameter, ask yourself,
 *<b>Is the quantity meaningful when aggregated?</b>
 ** For exampe, "NumberOfTriangles" versus "DrawCallNumber." If it is
 *aggregable, then use a counter.
 **
 ** \section HwCounters Hardware counters
 ** <b>This capability is not available on Windows platforms at this time.</b>
 ** Unlike traditional sampling-based tools, TAL will not sample hardware
 *counters in your program automatically. Rather,
 ** you need to instrument your functions of interest with
 *TAL_SampleHardwareCounters API. Instrumenting
 ** your code by hand may sound annoying since it requires more up-front
 *investment, but there are several
 ** benefits to the approach over the automatic method to be aware of:
 ** -# Lower overhead. The sampling approach takes a hardware interrupt
 **    at every sample into the operating system. The cost of each sample is
 *thus in the thousands of clocks range. TAL
 **    samples occur immediately, the cost of which should be in the hundreds of
 *clocks range.
 ** -# Precise sample placement. In TAL, you can place counter samples exactly
 *where you want them. If you
 **    are interested in the L2 cache behavior for a specific task, you can put
 *the samples at the start and end of the task, for example.
 **
 ** The TAL_SampleHardwareCounters API takes no arguments. The specific set of
 *hardware signals that are sampled is determined
 ** at runtime.
 **
 ** One way to use the TAL_SampleHardwareCounter API is to put samples at the
 *start and end of specific functions you are
 ** trying to optimize. The Platform View will parse these pairs of samples
 *provide you information such as "this task had 23 L2 cache misses."
 ** In code, this instrumentation looks like the following:
 ** \code
 **    void SomeImportantFunction() {
 **        // take a sample of the hardware counters at the start of the
 *function
 **        TAL_BeginTask(__FUNCTION__);
 **        TAL_SampleHardwareCounters();
 **        ...
 **
 **        // take a sample of the hardware counters at the end of the function
 *as well
 **        TAL_SampleHardwareCounters();
 **        TAL_EndTask()
 **    }
 ** \endcode
 **
 ** You can collect multiple samples per function. Consider the following
 *instrumentation for a moment:
 ** \code
 **    void SomeImportantKernel() {
 **        // as before, we will take a sample of the hardware counters at the
 *start of the function
 **        TAL_BeginTask(__FUNCTION__);
 **        TAL_SampleHardwareCounters();
 **
 **        // here is a tight inner loop. We suspect that one of these
 *iterations is really heavy on cache misses,
 **        // but we're not sure which iteration is causing it...
 **        for(int i=...) {
 **            if((rand()*10 / RAND_MAX) == 0) { // 1 out of 10 times, ...
 **               // sample the hardware counters so we can see how
 **               TAL_SampleHardwareCounters();
 **               TAL_SampleCounter("i", i);
 **               ... // any other TAL sample calls that would help you
 *understand what was happening in this loop iteration
 **            }
 **            ...
 **        }
 **
 **        // as before, take a sample of the hardware counters at the end of
 *the function as well
 **        TAL_SampleHardwareCounters();
 **        TAL_EndTask()
 **    }
 ** \endcode
 ** In this example, a hardware counter sample is taken statistically on 10% of
 *the loop iterations, as well as a software
 ** sample of the "i" variable. The Platform View will allow you to look at
 *these counters and study how they change during
 ** the execution of the loop. If you see a sudden jump in the hardware
 *counters, the additional TAL_SampleCounter and any
 ** other TAL calls you added on the sample can be used to understand what your
 *code was doing when the hardware counters
 ** spiked.
 **
 **/

/** \addtogroup EventsAndMarkers
 ** Events and markers APIs.
 **
 ** Events indicate instantaneous points in time within a thread. Markers
 *indicate instantaneous points in time globally.
 **
 ** TAL events and markers are used when you need to capture an instantaneous
 *behavior of your application.
 ** In the Platform View, an event will show up as a caret on
 ** the current thread, while a marker shows up as a caret on the overall
 *timeline ruler.
 **
 ** Typical uses are:
 ** - <b>Markers:</b> CPU-side present was called; Vsync occurred; Flip occurred
 ** - <b>Events:</b> This thread incremented a semaphore by 1.
 **
 ** These APIs support unadorned and Ex notations. The Ex methods accept the
 *timestamp of the event, whereas
 ** the unadorned method automatically obtains the timestamp when called.
 **/

/** \addtogroup Parameters
 ** APIs for capturing parameters with a task.
 **
 ** TAL parameters help you identify a task by its runtime arguments. Parameters
 *can be strings, 32- or 64- bit integers or floating point values.
 **
 ** A classic use of parameters is to drill into outlier behavior on a
 *particular task. For example, we might observe
 ** that a particular framebuffer-blending task is unusually high. To understand
 *why, we would add parameters
 ** to the task that describe the framebuffer format, and any other runtime
 *state that influence the blending operation:
 ** \code
 ** void DrawCall_BlendToFramebuffer(RenderState* rs, Pixel* p) {
 **   // a helpful TAL macro that will automatically issue
 *TAL_BeginNamedTask(__FUNCTION__)
 **   // and a matching TAL_EndTask when this scope closes.
 **   TAL_SCOPED_TASK();
 **   // issue some paramters for diagnostics purposes
 **   TAL_IncCounter("NumPixelsBlended");
 **   TAL_Param2i("PixelPos", p->x, p->y);
 **   TAL_Parami("ColorMask", rs->RenderTarget.ColorMask);
 **   TAL_ParamStr("BlendFunc", rs->RenderTarget.BlendFunc ? BLEND_ADD ?
 *"Additive" : "Unknown");
 **
 **   // .. actual work...
 **   ...
 ** }
 ** \endcode
 ** In the resulting trace, a typical workflow is to toggle between tasks while
 *watching the parameters panel
 ** to understand what parameters are driving the change in cost.
 **
 ** Deciding whether to use a parameter or a counter, at least for integers, can
 *sometimes be subjective. The best way
 ** to decide is to ask yourself, <b>Is the quantity meaningful when
 *aggregated?</b> In the exampe above,
 ** we would use a counter for "NumberOfPixelsBlended," but a parameter for
 *"BlendChannelMask" or "FrameNumber."
 ** This is because "AveragePixelsBlended" and "TotalPixelsBlended" make sense
 *from an analysis point of view: they heave meaningful
 ** numbers when totaled or averaged. In contrast, "AverageBlendChannelMask" or
 *"AverageFrameNumber" do not make sense.
 **
 ** While parameters are extraordinarily helpful for profiling, it is important
 *to balance
 ** that with their performance cost. The API calls themselves are not large,
 *but each parameter call creates about 20 bytes
 ** of data in the trace file. If you issue 4 parameters, 13,000 times per
 *thread per frame, spread acorss 32 threads, at 30Hz,
 ** your parameters themselves
 ** will consume 1 <b>gigabyte</b> per second of disk space! There are two
 *typical ways to deal with "parameter bloat,":
 ** -# <b>Hoisting:</b> Hoist parameters to a parent task. If you have a
 *parameter that is the same across many different calls,
 **    for example "Render Target Blend State," then to move it to a task that
 *created all of these tasks. In graphics, for
 **    example, most blending tasks can trace their origin back to a Drawing
 *task, where the original state was specified. In
 **    that case, we would put the blend state in the drawing task, then just
 *tie the individual blending tasks back to that
 **    base call using TAL's Relations API, giving an order of magnitude of
 *savings in terms of overall performance.
 ** -# <b>Log levels/categories:</b> Classify your parameters into different
 *categories and levels of verbosity. For example,
 **    we might say that we only need the number of pixels blended at all times
 *everything else "only when doing detailed
 **    analysis." To implement this in TAL, we would move the bulk of the
 *parameters to TAL_LOG_LEVEL_2. Then, we could enable
 **    or disable that function at runtime, or even compile time if we desired.
 **
 **/

#if defined(TAL_DISABLE)

#define TAL_GetThreadTrace() (0)
#define TAL_GetVersion() (0)
#define TAL_CanLog(l, c) (0)
#define TAL_Heartbeat(handle)
#define TAL_Flush(handle)
#define TAL_SendTraces()
#define TAL_SendTracesEx(x)

#define TAL_GetTimestampFreq() (1)
#define TAL_GetProcessorFreq() (1)
#define TAL_IsCapturing() (0)
#define TAL_SetLogLevel(t)
#define TAL_SetLogCategory(t)
#define TAL_GetFileVersion() (0)
#define TAL_SetTaskColor(...)
#define TAL_SetNamedTaskColor(...)
#define TAL_SetThreadName(n)
#define TAL_SetTraceName(handle, n)
#define TAL_SetThreadNameByTid(x, y)
#define TAL_GetStringHandle(n) (0)
#define TAL_RegisterCollector(x, y)
#define TAL_UnregisterCollector(x, y)
#define TAL_RegisterTransport(x, y, z)
#define TAL_RegisterDistiller(x, y)
#define TAL_UnregisterDistiller(x, y)
#define TAL_DescribeCategoryMask(x, y, z)

// deprecated functions
#define TAL_ThreadInit()
#define TAL_ThreadInitEx(s)
#define TAL_ThreadCleanup()
#define TAL_SetCaptureMode(m, ms)
#define TAL_BeginCapture(x, y, z) (1)
#define TAL_EndCapture() (1)
#define TAL_BeginCaptureToNetwork(t) (1)
#define TAL_EndCaptureToNetwork()
#define TAL_BeginCaptureToFile()
#define TAL_EndCaptureToFile(f)
#define TAL_SetLogFunction(f)
#define TAL_AddStringToPool(n) (0)

const TAL_ID talid = {0, 0, 0};
#define TAL_MakeID(n, h, l) (talid)

#define TAL_Vasprintf(...) (0)

#else // TAL_DISABLE

/************************************************************************/
#ifndef TAL_DOXYGEN
TAL_INLINE void TAL_CALL TAL_ThreadInit(void) {}
TAL_INLINE void TAL_CALL TAL_ThreadInitEx(const char *thread_name) {
  TALCTL_SET_TRACE_NAME_PARAMS params;
  params.handle = p__TAL_GetThreadTrace();
  params.name = thread_name;
  p__talctl(TALCTL_SET_TRACE_NAME, &params, sizeof(params), NULL, NULL);
}
TAL_INLINE void TAL_CALL TAL_ThreadCleanup(void) {}
#endif // TAL_DOXYGEN
/************************************************************************/

/************************************************************************/
/** \ingroup Basics
 ** Get the thread-specific trace handle for the calling thread.
 **
 ** Get the thread-specific trace handle for the calling thread.
 ** The return value of this function is the thread-specific buffer into which
 *all TAL APIs
 ** are serialized. Typically, you do not have to call this funciton:
 *TAL_BeginNamedTask("MyTask"), for example, will
 ** automatically call TAL_GetThreadTrace() for you.
 **
 ** One way to reduce TAL overhead, however, is to re-use the
 *TAL_GetThreadTrace() pointer across multiple TAL calls.
 ** For example, if you have a function that contains multiple TAL_ calls, you
 *might rewrite it as:
 ** \code
 **   void MyFunction() {
 **     TAL_TRACE* trace = TAL_GetThreadTrace();
 **     TAL_BeginTask(trace, "MyFunction");
 **     TAL_Parami(trace, "myparam", 3);
 **     TAL_EndTask(trace);
 **   }
 ** \endcode
 ** This removes two calls to TAL_GetThreadTrace, which can save you anywhere
 *from 20 to 100 clocks of overhead,
 ** depending on your platform and operating system.
 **
 ** The return value of TAL_GetThreadTrace will never change across the lifetime
 *of your thread. As a result, you can
 ** take the TAL_GetThreadTrace() value and store into your own data structure,
 *if you desire. However, <b>TAL_GetThreadTrace()
 ** will return a pointer specific to the calling thread,</b> so if you do save
 *the TAL_TRACE* pointer somewhere, make sure
 ** to store it in a thread-specific way.
 **
 ** The following are valid ways to save TAL_TRACE:
 ** \code
 **    void SomeFunction() {
 **       TAL_TRACE* t = TAL_GetThreadTrace();
 **       TAL_BeginNamedTask(t, "SomeFunction");
 **       ...
 **       TAL_EndTask(t);
 **    }
 **    void ThreadProc() {
 **       TAL_Trace* t = TAL_GetThreadTrace();
 **       while(true) {
 **           int cmd = GetCommand();
 **           TAL_BeginNamedTask(t, "ProcessCommand");
 **           ProcessCommand(cmd);
 **           TAL_EndTask(t);
 **       }
 **    }
 **    __declspec(thread) TAL_TRACE* gtTrace = NULL;
 **    void ThreadProc2() {
 **       gtTrace = TAL_GetThreadTrace();
 **    }
 ** \endcode
 **/
TAL_INLINE TAL_TRACE *TAL_GetThreadTrace(void) {
  return p__TAL_GetThreadTrace();
}

/************************************************************************/
/** \ingroup Basics
 ** Get the thread-specific trace handle for the specified thread ID.
 **
 ** Get the thread-specific trace handle for the specified thread ID.
 **/
TAL_INLINE TAL_TRACE *TAL_GetThreadTraceEx(
    TAL_PROCESS *process /**< The TAL_PROCESS containing the specified thread.
                            Use TAL_GetCurrentProcessHandle() to obtain the
                            current process handle. */
    ,
    TAL_UINT64 tid /**< The thread ID to be looked up. */) {
  TAL_TRACE *trace = NULL;
  TAL_UINT32 resultSize = sizeof(trace);
  TALCTL_GET_THREAD_TRACE_EX_PARAMS params;
  params.process = process;
  params.tid = tid;
  p__talctl(TALCTL_GET_THREAD_TRACE_EX, &params, sizeof(params), &trace,
            &resultSize);
  return trace;
}

/************************************************************************/
/** \ingroup Basics
 ** Get the TAL_PROCESS handle for the current process.
 **
 ** Get the TAL_PROCESS handle for the current process.
 **/
TAL_INLINE TAL_PROCESS *TAL_GetCurrentProcessHandle(void) {
  TAL_PROCESS *process;
  TAL_UINT32 resultSize = sizeof(process);
  p__talctl(TALCTL_GET_CURRENT_PROCESS_HANDLE, NULL, 0, &process, &resultSize);
  return process;
}

/************************************************************************/
/** Return true if we would log data at the given lvl and category
**
**/
TAL_INLINE TAL_BOOL TAL_CALL TAL_CanLog(TAL_LOG_LEVEL in_lvl,
                                        TAL_UINT64 in_Cat) {
  TAL_TRACE *tr = TAL_GetThreadTrace();
  return TAL_IS_LOGGABLE(tr, in_lvl, in_Cat);
}

/************************************************************************/
#ifndef TAL_DOXYGEN
TAL_INLINE void TAL_CALL TAL_Heartbeat(TAL_TRACE *trace /**< Trace handle */
) {
  TAL_UNUSED(trace);
}
#endif // TAL_DOXYGEN

/************************************************************************/
/** \ingroup Misc
 ** Begins writing all TAL_ commands issued against the calling thread to disk.
 **
 ** TAL_Flush marks all TAL_ commands issued against the calling thread as ready
 *to be written to disk. The write will not actually begin until the next
 *TAL_SendTraces call.
 **
 ** Typically, you will not need to flush individual traces, or call
 *TAL_SendTraces manually. Threads' trace buffers are
 ** flushed automatically about every 100 or so TAL_ calls, and a background
 *thread calls TAL_SendTraces() every few milliseconds.
 **
 ** This function exists for specific scenarios where you want to guarantee that
 *data from a specific thread
 ** gets sent without disrupting any other threads in the system. In such cases,
 *the following commands will
 ** ensure that the given TAL_Event is written to disk without actually
 ** affecting any of the other threads in the system:
 ** \code
 **    TAL_Event(TAL_GetThreadTrace(), "ImportantEvent");
 **    TAL_Flush(TAL_GetThreadTrace());
 **    TAL_SendTracesEx(false); // send the just-enqueued trace
 ** \endcode
 **/
#ifdef __cplusplus
TAL_INLINE void TAL_CALL TAL_Flush(void) { p__TAL_Flush(TAL_GetThreadTrace()); }
#endif              //! def __cplusplus
TAL_INLINE void TAL_CALL TAL_Flush(TAL_TRACE *trace /**< Trace handle */
) {
  p__TAL_Flush(trace);
}

/************************************************************************/
/** \ingroup Basics
 ** Send all flushed, unsent traces to the capture tool (or file).
 **
 ** Send all flushed, unsent traces from all threads to the capture tool (or
 *file).
 **/
TAL_INLINE void TAL_CALL TAL_SendTracesEx(
    TAL_BOOL in_bFlushAllTraces /**< If true, this will flush the traces in all
                                   threads before sending. */
) {
  p__talctl(TALCTL_SEND_TRACES_EX, &in_bFlushAllTraces,
            sizeof(in_bFlushAllTraces), NULL, NULL);
}

/************************************************************************/
/** \ingroup Basics
 ** Send all traced data to the capture tool (or file).
 **
 ** Send all traced data to the capture tool (or file). This
 ** should be called only in cases where the periodic TAL_SendTraces calls from
 *the
 ** watchdog thread don't occur often enough. Note that it has relatively high
 *overhead, since it walks over every thread in the system, forcing it to output
 *any un-flushed data.
 **
 ** If you want to send data at lower overhead and are confident that your
 ** data is already buffered (via TAL_Flush), then use TAL_SendTracesEx(false).
 **/
TAL_INLINE void TAL_CALL TAL_SendTraces() { TAL_SendTracesEx(TAL_TRUE); }

/************************************************************************/
/** \ingroup Misc
 ** Get the frequency of the TAL timestamp clock.
 **
 ** Get the frequency of the TAL timestamp clock.
 **
 ** When calling TAL_BeginTaskEx, TAL_EndTaskEx, TAL_EventEx,
 ** or TAL_MarkerEx, the provided timestamp value must be obtained with
 *TAL_GetCurrentTime().
 ** If you use other timestamp sources, e.g. Win32's QueryPerformanceCounter,
 *make sure to convert
 ** the obtained values into the TAL's timespace first.
 **
 **/
TAL_INLINE TAL_UINT64 TAL_CALL TAL_GetTimestampFreq(void) {
  TAL_UINT64 freq = 1;
  TAL_UINT32 resultSize = sizeof(freq);
  p__talctl(TALCTL_GET_TIMESTAMP_FREQ, NULL, 0, &freq, &resultSize);
  return freq;
}

/************************************************************************/
/** \ingroup Misc
** Get the frequency of the processor's clock.
**
** Get the frequency of the processor's clock.
** When calling TAL_BeginTaskEx, TAL_EndTaskEx, TAL_EventEx,
** or TAL_MarkerEx, the provided ts value must be in TAL_ticks.
** Use the following formula to convert processor ticks (e.g. those that come
** from rdtsc) to TAL_ticks:
** ((processor_ticks * TAL_GetTimestampFreq()) / GetProcessorFreq()).
**
** <b>Notes:</b>
** <ul>
**   <li> This value is not necessarily the same as
*TAL_GetTimestampFrequency</li>
** </ul>
**/
TAL_INLINE TAL_UINT64 TAL_CALL TAL_GetProcessorFreq(void) {
  TAL_UINT64 freq = 1;
  TAL_UINT32 resultSize = sizeof(freq);
  p__talctl(TALCTL_GET_PROCESSOR_FREQ, NULL, 0, &freq, &resultSize);
  return freq;
}

/************************************************************************/
/************************************************************************/

/************************************************************************
** Dynamically sets the log level for TAL.
** Dynamically sets the log level for TAL.
** \deprecated{Although this function is deprecated, it still has effect.
*Although discouraged, if you do
** set the log level via the configuration file (tal.conf) and then call this
*API, the value you pass in here will take precidence.}
**/
TAL_INLINE void TAL_CALL TAL_SetLogLevel(TAL_LOG_LEVEL in_lvl) {
  p__talctl(TALCTL_SET_LOG_LEVEL, &in_lvl, sizeof(in_lvl), NULL, NULL);
}

/************************************************************************
** Dynamically sets the category mask for TAL.
** Dynamically sets the category mask for TAL.
** \deprecated{Although this function is deprecated, it still has effect.
*Although discouraged, if you do
** set the log mask via the configuration file (tal.conf) and then call this
*API, the value you pass in here will take precidence.}
**/
TAL_INLINE void TAL_CALL TAL_SetLogCategory(TAL_UINT64 in_Cat) {
  p__talctl(TALCTL_SET_LOG_CATEGORY, &in_Cat, sizeof(in_Cat), NULL, NULL);
}

/************************************************************************/
/** \ingroup Basics
** Determine whether the TAL Collector is currently capturing.
** Determine whether the TAL Collector is currently capturing. Depending on
** the capturing mode, there may or may not be
** a connection to a file or network when TAL_IsCapturing returns true.
**/
TAL_INLINE TAL_BOOL TAL_CALL TAL_IsCapturing(void) {
  TAL_BOOL result = TAL_FALSE;
  TAL_UINT32 resultSize = sizeof(result);
  p__talctl(TALCTL_IS_CAPTURING, NULL, 0, &result, &resultSize);
  return result;
}

/************************************************************************/
#ifndef TAL_DOXYGEN // hide deprecated functions
TAL_INLINE void TAL_CALL TAL_SetCaptureMode(TAL_UINT32 mode, TAL_UINT32 param) {
  TAL_UNUSED(mode);
  TAL_UNUSED(param);
}
TAL_INLINE TAL_BOOL TAL_CALL TAL_BeginCapture(const char *trans,
                                              const char *name, int to) {
  TAL_UNUSED(trans);
  TAL_UNUSED(name);
  TAL_UNUSED(to);
  return TAL_FALSE;
}
TAL_INLINE void TAL_CALL TAL_BeginCaptureToFile(const char *name) {
  TAL_UNUSED(name);
}
TAL_INLINE TAL_BOOL TAL_CALL TAL_BeginCaptureToNetwork(int to) {
  TAL_UNUSED(to);
  return TAL_FALSE;
}
TAL_INLINE void TAL_CALL TAL_EndCapture(void) {}
TAL_INLINE void TAL_CALL TAL_EndCaptureToFile(void) {}
TAL_INLINE void TAL_CALL TAL_EndCaptureToNetwork(void) {}
#endif              // ndef TAL_DOXYGEN
/************************************************************************/

/************************************************************************/
#ifndef TAL_DOXYGEN
/** \ingroup Misc
** Register an additional trace source.
** Register an additional trace source.
** In TAL, you can obtain data from multiple processes, not just the currently
*running process.
** The way that this works is that each child process has a custom TAL_TRANSPORT
*registered via TAL_RegisterTransport that
** forwards its collected data up to the parent process.
** On the parent process side, two steps need to be followed. First, a
*TAL_TRANSPORT needs to be registered that knows
** how to connect-to and recieve from the child process. Next, this transport
*has to be registered as a collector, passing
** in the transport name as the in_pParam argument. This will cause the connect
*and receive function of the transport
** to be periodically polled for data by the TAL runtime.
**/
TAL_INLINE TAL_BOOL TAL_CALL TAL_RegisterCollector(const char *in_Type,
                                                   const char *in_pParam) {
  TAL_BOOL result = TAL_FALSE;
  TAL_UINT32 resultSize = sizeof(result);
  TALCTL_REGISTER_COLLECTOR_PARAMS params;
  params.type = in_Type;
  params.param = in_pParam;
  p__talctl(TALCTL_REGISTER_COLLECTOR, &params, sizeof(params), &result,
            &resultSize);
  return result;
}

/** \ingroup Misc
** Remove an additional trace source.
**/
TAL_INLINE TAL_BOOL TAL_CALL TAL_UnregisterCollector(const char *in_Type,
                                                     const char *in_pParam) {
  TAL_BOOL result = TAL_FALSE;
  TAL_UINT32 resultSize = sizeof(result);
  TALCTL_UNREGISTER_COLLECTOR_PARAMS params;
  params.type = in_Type;
  params.param = in_pParam;
  p__talctl(TALCTL_UNREGISTER_COLLECTOR, &params, sizeof(params), &result,
            &resultSize);
  return result;
}
#endif              // ndef TAL_DOXYGEN

/************************************************************************/
#ifndef TAL_DOXYGEN // hide deprecated functions

TAL_INLINE TAL_BOOL TAL_CALL
TAL_RegisterDistiller(const char *in_Name, TAL_DistillerFn in_pDistillerFunc) {
  TAL_BOOL result = TAL_FALSE;
  TAL_UINT32 resultSize = sizeof(result);
  TALCTL_REGISTER_DISTILLER_PARAMS params;
  params.name = in_Name;
  params.getDistillerFunc = in_pDistillerFunc;
  p__talctl(TALCTL_REGISTER_DISTILLER, &params, sizeof(params), &result,
            &resultSize);
  return result;
}

TAL_INLINE TAL_BOOL TAL_CALL TAL_UnregisterDistiller(const char *in_Name) {
  TAL_BOOL result = TAL_FALSE;
  TAL_UINT32 resultSize = sizeof(result);
  TALCTL_UNREGISTER_DISTILLER_PARAMS params;
  params.name = in_Name;
  p__talctl(TALCTL_UNREGISTER_DISTILLER, &params, sizeof(params), &result,
            &resultSize);
  return result;
}

TAL_INLINE TAL_STRING_HANDLE TAL_CALL TAL_AddStringToPool(const char *str) {
  TAL_STRING_HANDLE result = (TAL_STRING_HANDLE)-1;
  TAL_UINT32 resultSize = sizeof(result);
  p__talctl(TALCTL_ADD_STRING_TO_POOL, &str, sizeof(str), &result, &resultSize);
  return result;
}
#endif              // ndef TAL_DOXYGEN

/************************************************************************/
/** \ingroup Basics
** Get the handle for a given string in TAL's string pool (after first adding
*the string to the pool if it
** doesn't already exist).
** Look up the handle for a given string in TAL's string pool; if it doesn't
*exist in the pool, add it and
** return its handle. You can use the pooled string handle to refer to the
*string in various TAL functions
** to reduce your trace buffer sizes.
**
** These string handles are used as efficient substitutes for string literals in
*various TAL APIs,
** such as TAL_BeginNamedTaskH, TAL_ParamH, and TAL_SampleCounterH, to name just
*a few. Although there is
** no physical limit to the length of the strings, excessively long strings may
*not get displayed correctly
** within the TaskAnalyzer GUI.
**
** The built-in instrumentation within the TAL collector uses strings prefixed
*with "TAL_" and "TAL ". To avoid
** colliding with these strings, make sure you don't use these prefixes.
**
** This function is thread-safe; it makes use of thread synchronization to
*protect data shared
** among multiple threads. So try to use it only during your application's init
*time, rather than during
** its normal operation.
**/
TAL_INLINE TAL_STRING_HANDLE TAL_CALL
TAL_GetStringHandle(const char *str /**< The string to look up */) {
  TAL_STRING_HANDLE result = (TAL_STRING_HANDLE)-1;
  TAL_UINT32 resultSize = sizeof(result);
  p__talctl(TALCTL_GET_STRING_HANDLE, &str, sizeof(str), &result, &resultSize);
  return result;
}

/************************************************************************/
/** \ingroup Counters
 ** Set counter sample semantics for a particular sampled counter.
 ** Tells TAL what semantics to apply when interpreting a counter sample
 ** of the provided name. The TAL_COUNTER_SAMPLE_TYPE enumeration provides
 ** details on the different semantics. For example, you would use
 *TAL_COUNTER_SAMPLE_TYPE_CORE
 ** to say that a given sample is coupled to a core-specific hardware counter.
 ** In this case, the counter samples will be specially interpreted by TAL so
 *that
 ** the multiple threads affected by this sample do not over-count the sampled
 *data.
 **
 ** All sampled counters will default to the TAL_COUNTER_SAMPLE_TYPE_SOFTWARE
 *type
 ** unless you specify otherwise.
 **
 ** This function is thread-safe; it makes use of thread synchronization to
 *protect data shared
 ** among multiple threads. So try to use it only during your application's init
 *time, rather than during
 ** its normal operation.
 **
 **/
TAL_INLINE void TAL_CALL
TAL_SetCounterSampleType(const char *str, TAL_COUNTER_SAMPLE_TYPE sample_type) {
  TALCTL_SET_COUNTER_SAMPLE_TYPE_PARAMS params;
  params.str = str;
  params.type = sample_type;
  p__talctl(TALCTL_SET_COUNTER_SAMPLE_TYPE, &params, sizeof(params), NULL,
            NULL);
}

/************************************************************************/
/** \ingroup Basics
** Set the description for a category mask bit.
** Set the description for a category mask bit.
** TAL supports a concept of logging levels and categories. You can
** pass a level and category to every TAL call --- there are then compile-
** and run-time configuration options for TAL that allow you to selectively
** enable or disable capturing of specific levels and categories. For
** more information, please see the \ref Basics section.
**
** This function attaches a the provided descriptions to the specified category
*bits. This helps
** you remember which bits corepsond to which application subsystems. This is
*especially
** useful in large and complex system, where dozens of different categories of
*data might be available for use.
**
** This API can be called multiple times --- the effect in this case is to add
*to the overall
** category description.
** For example, suppose two totally independent systems issue the following:
** \code
**   TAL_DescribeCategoryMask(TAL_LOG_CAT_1, "Perf", "Basic performance
*information");
**   TAL_DescribeCategoryMask(TAL_LOG_CAT_1, "ToplevelTasks", "Top-level
*tasks");
** \endcode
** This will show up in the TAL UI as a single checkbox, "Perf and
*ToplevelTasks."
** Our suggested design pattern is to provide a static inline
*"DescribeTALCategories() function
** alongside where you define/enumerate your application categories. For
*example:
** \code
**   #define MYAPP_CAT_PERF TAL_LOG_CAT_1
**   #define MYAPP_CAT_MEM  TAL_LOG_CAT_2
**   static inline void DescribeTALCategories() {
**      TAL_DescribeCategoryMask(MYAPP_CAT_PERF, "Perf", "Basic performance
*information, ~5% overhead");
**      TAL_DescribeCategoryMask(MYAPP_CAT_MEM,  "Memory", "Memory allocation
*information.");
**   }
** \endcode
** Then, call this describe function somewhere in your init sequence.
**
** An alternative and fancier approach is to use nested macros:
** \code
**   #define MYAPP_CATEGORIES() \
**       MYAPP_CATEGORY(MYAPP_CAT_PERF,TAL_LOG_CAT_1,"Perf","Basic performance
*info")\
**       MYAPP_CATEGORY(MYAPP_CAT_PERF,TAL_LOG_CAT_1,"Perf","Basic performance
*info")\
**   enum MyCategories {
**       #define MYAPP_CATEGORY(e,v,_i1,_i2) e=v,
**       MYAPP_CATEGORIES()
**       #undef MYAPP_CATEGORY
**   }
** \endcode
** Then in main:
** \code
**      #define MYAPP_CATEGORY(_i1,v,n,d) TAL_DescribeCategoryMask(v,n,d)
**      MYAPP_CATEGORIES()
**      #undef MYAPP_CATEGORY
** \endcode
**/
TAL_INLINE void TAL_CALL TAL_DescribeCategoryMask(
    TAL_UINT64 categoryMask, /**< The mask bit to describe */
    const char *tag,         /**< Short name */
    const char *description  /**< Full description */
) {
  TALCTL_DESCRIBE_CATEGORY_MASK_PARAMS params;
  params.mask = categoryMask;
  params.tag = tag;
  params.description = description;
  p__talctl(TALCTL_DESCRIBE_CATEGORY_MASK, &params, sizeof(params), NULL, NULL);
}

/************************************************************************/
/** \ingroup Relations
** Registers an ID namespace with TAL.
**
** This API registers an ID namespace with TAL. This helps with bookkeeping
*about
** what namespaces are being used by what subsystem: TAL will print a runtime
*warning to the console
** if two systems reigster for the same ID namespace, helping you diagnose and
*avoid namespace collissions.
**
** A TAL ID is a three-tuple of values: an ID namespace, and two 64-bit
*quantities.
** The purpose of the ID namepsace is to allow use of arbitrary quantities in
*the 64 bit fields,
** without worry that the values from one library will collide with values from
*another library.
**
** As a concrete example, consider two separate systems, a physics system and a
*graphics system. The physics
** system might choose to assign IDs using pointers to PhysicsTask* structures,
*e.g. TAL_MakeID(0,(TAL_UINT64)physTaskPtr,0). Simultaneously, the graphics
** system might choose to assign IDs simply using a raw sequencing number, e.g.
*TAL_MakeID(0,gGfxSeqNumber,0). It is
** possible, in this situation, for both systems to accidentally come up with
*the same sequence number and pointer, causing
** the IDs from one system to collide with the IDs from the other.
**
** The ID Namespace argument is provided to avoid this situation. In the example
*above, we would
** assign the physics subsystem to a different "namespace" than graphics, e.g.
** TAL_MakeID(1,physTask,0) and TAL_MakeID(2,gGfxSeqNumber,0).
**
** Since namespaces are assigned at compile time, it is important that you
*choose the namespace argument carefully so
** it does not collide with other subsystems. The TAL_RegisterIdNamespace helps
*with this bookkeeping by tracking
** what namespaces are in use, and printing an error when two collide.
**
** We recommend using this function in the initialization function for each
*subsystem. So, for example, when
** the physics and graphics systems are initialized, we would issue the
*following calls:
** \code
** void InitPhysics() {
**    TAL_RegisterIdNamespace(1, "Physics");
**    ...
** }
** void InitGraphics() {
**    TAL_RegisterIdNamespace(2, "Graphics");
**    ...
** }
** \endcode
**
**/
TAL_INLINE void TAL_CALL TAL_RegisterIdNamespace(
    TAL_ID_NAMESPACE ns, /**< 8-bit namespace value */
    const char *ns_name /**< A descriptive name for the namespace. If there is a
                           collission between namespaces, TAL will use this name
                           in its error report. */
) {
  TALCTL_REGISTER_ID_NAMESPACE_PARAMS params;
  params.ns = ns;
  params.ns_name = ns_name;
  p__talctl(TALCTL_REGISTER_ID_NAMESPACE, &params, sizeof(params), NULL, NULL);
}

/************************************************************************
 ** Set a user-specified name for the calling thread.
 ** Set a user-specified name for the calling thread, which will appear in the
 *label
 ** for the corresponding track in the timeline view of the Platform View.
 ** The default name is a concatenation of the OS-specific PID and thread ID.
 ** \ingroup Basics
 **/
TAL_INLINE void TAL_CALL TAL_SetThreadName(
    const char
        *name /**< Name of thread as it should appear in the Platform View */
) {
  p__talctl(TALCTL_SET_THREAD_NAME, &name, sizeof(name), NULL, NULL);
}

/************************************************************************
 ** Set a user-specified name for a given trace.
 ** Set a user-specified name for a given trace, which will appear in the label
 *of
 ** the corresponding track in the timeline view of the Platform View.
 ** The default name is the same as the default thread name.
 ** \ingroup Basics
 **/
TAL_INLINE void TAL_CALL TAL_SetTraceName(
    TAL_TRACE *trace, /**< Trace whose name should be changed */
    const char
        *name /**< Name of trace as it should appear in the Platform View */
) {
  TALCTL_SET_TRACE_NAME_PARAMS params;
  params.handle = trace;
  params.name = name;
  p__talctl(TALCTL_SET_TRACE_NAME, &params, sizeof(params), NULL, NULL);
}

/************************************************************************
 ** Set a user-specified name for a given thread.
 ** Set a user-specified name for a given thread, which will appear in the label
 ** for the corresponding track in the timeline view of the Platform View.
 ** The default name is a concatenation of the OS-specific PID and thread ID.
 ** \ingroup Basics
 **/
TAL_INLINE void TAL_CALL TAL_SetThreadNameByTid(
    TAL_UINT64
        tid, /**< OS-specific ID for the thread whose name should be changed */
    const char
        *name /**< Name of thread as it should appear in the Platform View */
) {
  TAL_SetTraceName(TAL_GetThreadTraceEx(TAL_GetCurrentProcessHandle(), tid),
                   name);
}

/************************************************************************/
/** \ingroup Tasks
 ** Sets the color for the specified task when rendered in the Platform View.
 **
 ** Sets the color for the specified task when rendered in the Platform View.
 ** NOTE: It is recommended to keep each of the red, green, and blue
 ** values below 128. This will allow the Platform View to provide
 ** a distinctly brighter color when the task is selected.
 **/
TAL_INLINE void TAL_CALL
TAL_SetTaskColor(void (*fn)(void), /**< Pointer to the task function */
                 TAL_UINT8 red, TAL_UINT8 green, TAL_UINT8 blue) {
  TALCTL_SET_TASK_COLOR_PARAMS params;
  params.fn = fn;
  params.red = red;
  params.green = green;
  params.blue = blue;
  p__talctl(TALCTL_SET_TASK_COLOR, &params, sizeof(params), NULL, NULL);
}

/************************************************************************/
/**\ingroup Tasks
 ** Sets the color for the specified task when rendered in the Platform View.
 **
 ** Sets the color for the specified task when rendered in the Platform View.
 ** NOTE: It is recommended to keep each of the red, green, and blue
 ** values below 128. This will allow the Platform View to provide
 ** a distinctly brighter color when the task is selected.
 **/
TAL_INLINE void TAL_CALL
TAL_SetNamedTaskColor(const char *name, /**< Task name */
                      TAL_UINT8 red, TAL_UINT8 green, TAL_UINT8 blue) {
  TALCTL_SET_NAMED_TASK_COLOR_PARAMS params;
  params.name = name;
  params.red = red;
  params.green = green;
  params.blue = blue;
  p__talctl(TALCTL_SET_NAMED_TASK_COLOR, &params, sizeof(params), NULL, NULL);
}

/************************************************************************/
/** \ingroup Misc
 ** Redirects all status output of the TAL runtime to a specified logging
 *function.
 ** Redirects all status output of the TAL runtime to a specified logging
 *function. By default, this output
 ** will be sent to standard out.
 ** \param in_pProc Function pointer that will be called for every log output
 *made by TAL
 **/
TAL_INLINE void TAL_CALL TAL_SetLogFunction(TAL_LogProc in_pProc) {
  p__talctl(TALCTL_SET_LOG_FUNCTION, &in_pProc, sizeof(in_pProc), NULL, NULL);
}

/************************************************************************/
/** \ingroup Triggers
 ** Evaluates the trigger conditions using the given name and value,
 ** and takes the associated action(s) of the trigger condition(s) that are
 *satisfied.
 **
 ** Evaluates the trigger conditions applying to the given name, substituting
 *the given
 ** for the name in the trigger condition expression. If the expression
 *evaluates to TRUE
 ** when TAL_TestTriggerConditionsH is called, the TAL runtime takes the
 *action(s)
 ** associated with the satisfied trigger condition(s). There are two possible
 *trigger
 ** actions: CAPTURE_START and CAPTURE_STOP.
 **
 ** For example, if the CAPTURE_START trigger is specified as follows:
 **
 ** \code  CAPTURE_START Frame > 5   \endcode
 **
 ** then TAL_TestTriggerConditionsH(<i>hFrame</i>, <i>nFrame</i>) with
 *<i>hFrame</i> being
 ** the string handle for "Frame" and <i>nFrame</i> > 5, will cause capturing to
 *start.
 **
 ** Trigger conditions are specified in TAL settings (in tal.conf or
 *taserver.conf). See
 ** the default tal.conf file for complete information on formatting trigger
 *conditions.
 **
 ** NOTE: On LRB machines, if TAL_TestTriggerConditionsH() is called from code
 *running on the
 ** host, capturing will start/stop both on the host and on LRB. But if
 *TAL_TestTriggerConditionsH()
 ** is called from code running on LRB, capturing will start/stop only on LRB.
 *Thus, LRB-side
 ** triggers apply only when capturing to a LRB file (i.e. where the
 * /lrb/tal.conf file contains
 **
 ** \code  OUTPUT file /lrb/mytrace.gpa_trace  \endcode
 **
 ** \param in_Name  The handle for the name appearing in the trigger conditions
 *to be tested.
 ** \param in_Value The value to substitute for the name when testing the
 *trigger conditions.
 **/
TAL_INLINE void TAL_CALL
TAL_TestTriggerConditionsH(TAL_STRING_HANDLE in_NameHandle, int in_Value) {
  TALCTL_TEST_TRIGGER_CONDITIONS_PARAMS params;
  params.nameHandle = in_NameHandle;
  params.value = in_Value;
  p__talctl(TALCTL_TEST_TRIGGER_CONDITIONS, &params, sizeof(params), NULL,
            NULL);
}

/************************************************************************/
/**\ingroup Relations
** Packs the provided arguments into a TAL_ID structure.
**
** The TAL_ID structure is a 136 bit structure that can be used to uniquely
*identify a TAL
** task or virtual task. To fill in the fields of this structure, we suggest
*using TAL_MakeID,
** as it provides forward and backward compatibility across changes in the
*TAL_ID structure.
**
** A typical use of this API is as follows:
** \code
** TAL_ID id = TAL_MakeID(0,parent_ptr,0);
** TAL_AddRelationThis(TAL_RELATION_IS_CHILD_OF, id);
** \endcode
**
** In this sequence, the MakeID function itself has no side effects; it merely
*creates the TAL_ID
** structure.
**/
TAL_INLINE TAL_ID TAL_CALL TAL_MakeID(
    TAL_ID_NAMESPACE id_ns, /**< ID namespace */
    TAL_UINT64 id_hi,       /**< High QWORD of the ID value */
    TAL_UINT64 id_lo        /**< Low QWORD of the ID value */
) {
  TAL_ID id;
  id.hi = id_hi;
  id.lo = id_lo;
  id.ns = id_ns;
  return id;
}

/** Runs a series of measurements on the calling thread to determine TAL
overhead.

Use this function to determine, on your current machine, the overhead of common
TAL operations.

This function will run a series of tests on the calling thread. The output will
be a list of TAL APIs and their per-clock overhead. The overhead is reported in
several different configurations. Pay attention to two configurations in
particular: <ul> <li>- When TAL is disabled: represents the overhead when no GUI
is connected to your application.</li> <li>- When TAL is enabled:  represents
the overhead were a GUI connected but you had infinite bandwidth to save
data</li>
</ul>

**/
TAL_INLINE void TAL_CALL TAL_PerformanceTest(TAL_BOOL in_bPrint) {
  p__talctl(TALCTL_PERFORMANCE_TEST, &in_bPrint, sizeof(in_bPrint), NULL, NULL);
}

#ifdef __cplusplus
extern "C"
#endif // __cplusplus
#if !defined(TAL_KERNEL) && !defined(TAL_DOXYGEN)
    char *
    TAL_Vasprintf(const char *fmt, TAL_VA_LIST ap);
#endif

#endif // TAL_DISABLE
#endif // TAL_API_H

/* ************************************************************************* **
** ************************************************************************* **
** EOF
** ************************************************************************* **
** ************************************************************************* */
