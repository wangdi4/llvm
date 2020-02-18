======================================
Debugging OpenCL(TM) Kernels on Linux*
======================================

.. contents:: :local:

Introduction
============

The Intel(R) SDK for OpenCL(TM) - CPU only runtime and The Intel(R) FPGA
Emulation Platform for OpenCL(TM) support debugging OpenCL(TM) kernels using a
GNU GDB*, or LLDB* Debugger on Linux*. It allows for debugging host code and
OpenCL(TM) kernels in a single debug session.

Enabling Debugging in OpenCL(TM) CPU Compiler and Runtime
=========================================================

To enable the debugging mode in the OpenCL(TM) CPU Compiler and Runtime or FPGA
Fast Emulator, specific options should be passed to the build options string
parameter in the ``clBuildProgram`` function.

``-g``
    The ``-g`` flag enables source-level debug information generation.

    NOTE: Passing a ``-g`` flag means that no optimizations (such as inlining,
    unrolling, vectorization, etc.) are performed, the same as if a
    ``-cl-opt-disable`` option was passed.

``-s /full/path/to/OpenCL/source/file.cl``
    This option specifies the full path to the OpenCL(TM) source file. If the
    path includes spaces, the entire path should be enclosed with double or
    single quotes.

    This option is required in cases where the source of an OpenCL(TM) kernel is
    passed to the ``clCreateProgramWithSource`` function as a string.

    Only one file can be specified with the ``-s`` option. If the file contains
    one or more ``#include`` directives, multiple paths for files with the
    ``-s`` option are not needed.

clBuildProgram Example
^^^^^^^^^^^^^^^^^^^^^^

::

  err = clBuildProgram(
          g_program,
          0,
          NULL,
          "-g -s \"<path_to_opencl_source_file>\"",
          NULL,
          NULL);

NOTE: The OpenCL(TM) kernel code must exist in a text file that is separate from
the host code. Debugging OpenCL(TM) code that appears only in a string embedded
in the host application is not supported.

Instead of passing the ``-s`` option to specify a path to an OpenCL(TM) source
file, one of the follow approaches can be used:

* Use an Intel(R) SDK for OpenCL(TM) - offline compiler
  Specifying the ``-s`` option is not needed, as the path to an OpenCL(TM)
  source file is defined by the ``-input`` option.

  ::

    ioc -cmd=build -input=kernel.cl '-bo=-g'

* Pass a string with one or more ``#include`` directives to ``clCreateProgramWithSource``.
  In this instance the paths to included files are detected automatically.

  ::

    const char* src = "#include kernel.cl";
    cl_program program = clCreateProgramWithSource(
          m_context, 1, &src, NULL, &error);


Start the Debugging Session
===========================

The standard GDB* or LLDB* commands are used to debug OpenCL(TM) programs:

::

  gdb --args ./host_program

Breakpoints and stepping functionality are fully supported.

For more information about GDB* or LLDB* debuggers see the References section.

GDB* Debugging Session Example
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Start debugging a program:

   ::

     gdb --args ./host_program

2. Set a breakpoint in a kernel:

   ::

     (gdb) break kernel.cl:5
     Make breakpoint pending on future shared library load? (y or [n]) y
     Breakpoint 1 (kernel.cl:5) pending.

3. Run the host program. The execution stops once the debugger hits the
   breakpoint in the kernel:

   ::

     (gdb) run
     Thread 19 "debugger_test_t" hit Breakpoint 1, foo (c=9 '\t') kernel.cl:5
     5         d = d + 2;

Conditional Breakpoints on Work-Items
=====================================

According to the OpenCL(TM) standard, work-items execute OpenCL(TM) kernels
simultaneously. If a work-group contains more than one work-item, the debugger
stops on a breakpoint in every running work-item.

When a kernel is compiled with a ``-g`` flag, the compiler generates three
variables that define the current work item. There is a variable for each range
in the 3D NDRange:
* ``__ocl_dbg_gid0``
* ``__ocl_dbg_gid1``
* ``__ocl_dbg_gid2``

These variables can be used to set conditional breakpoints that make it possible
to stop on a specific work item (or work items).

Native Debugger Syntax
^^^^^^^^^^^^^^^^^^^^^^

The native GDB* or LLDB* commands for conditional breakpoints are supported.

::

  (gdb) break kernel.cl:3 if (__ocl_dbg_gid0 == 2)
  Make breakpoint pending on future shared library load? (y or [n]) y
  Breakpoint 3 (kernel.cl:3 if (__ocl_dbg_gid0 == 2)) pending.

  (gdb) run

  [Switching to Thread 0x7fffcffff700 (LWP 26115)]

  Thread 20 "host_program" hit Breakpoint 1, main_kernel (buf_in=0x1834280 "", buf_out=0x186c880 "")
  at kernel.cl:3
  3           size_t workdim = get_work_dim();

  (gdb) print __ocl_dbg_gid0
  $0 = 2


GDB* Plugin for OpenCL(TM): Work-Item Breakpoint Command
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The plugin extends the GDB* commands syntax to make the work with conditional
breakpoints on work-items easier. This feature is available only if GDB* was
configured using Python*.

``ocl-break-workitem`` *location* *work-item* *[temporary]*

    Set a breakpoint at the given *location*, which can specify a function name,
    a line number, or an address of an instruction.

    The *work-item* specifies a work-item number in the 3D NDRange, where the
    number of each range is separated by a space. For example, 0 0 0.

    The *temporary* keyword will set a temporary breakpoint for the requested
    line and will stop just once (similar to GDB's tbreak command).

To enable OpenCL(TM) work-item breakpoints the ``libintelocl.so-gdb.py`` file
should be sourced from a GDB* session.

::

  (gdb) source libintelocl.so-gdb.py

Set a breakpoint on a specific work-item using the ``ocl-break-workitem``
command:

::

  (gdb) ocl-break-workitem kernel.cl:5 0 0 0
  OpenCL Breakpoint set at: "kernl.cl":5 for work item (0, 0, 0)

  (gdb) run

  [Switching to Thread 0x7fffcffff700 (LWP 26115)]

  Thread 20 "host_program" hit Breakpoint 1, main_kernel (buf_in=0x1834280 "", buf_out=0x186c880 "")
  at kernel.cl:5
  5         d = d + 2;

  (gdb) print __ocl_dbg_gid0
  $0 = 0

  (gdb) print __ocl_dbg_gid1
  $1 = 0

  (gdb) print __ocl_dbg_gid2
  $2 = 0

Known Issues
============

1. For better debugging experience GDB* 7.12 or higher is required.  In cases
   where an older GDB* version is used, some issues related to JIT code
   debugging may occur (such as no automatic breakpoints reset between runs,
   fail to stop on a breakpoint for second and other runs, etc.).

2. The finish GDB* or LLDB* command returns to the same line as a function call.
   The finish command should advance an instruction pointer to the next
   instruction after the call. It is expected that it will point to the next
   line following the line of calling, except when a callee function returns a
   value for using. In the case of OpenCL(TM) kernel code it points to the same
   line as the call, even the callee function is a void one.

3. Setting ``__local`` variables has no effect on program execution. An attempt
   to set variables in a local address space has no effect. The new value is
   discarded just after the next step.

4. Conditional breakpoints on work-items may take too much time to evaluate.  It
   may take too much time to evaluate conditional breakpoints depending on a
   work item number for heavy NDRange kernels. To avoid such breakpoints the
   kernel source code can be modified using the ``get_global_id`` function.

References
==========
1. Link to more information on the GNU* Project Debugger:
    https://www.gnu.org/software/gdb/

2. Link to more information on the LLDB* Debugger:
    https://lldb.llvm.org/lldb-gdb.html
