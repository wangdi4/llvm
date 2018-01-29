=================================
Debugging OpenCL kernels on Linux
=================================

.. contents:: :local:

Introduction
============

OpenCL CPU Compiler & Runtime and FPGA Fast Emulator support debugging
OpenCL kernels using GNU debugger GDB or LLDB Debugger on Linux.
It allows to debug a host code and OpenCL kernels in a single debug session.

Enabling Debugging in OpenCL CPU Compiler & Runtime
===================================================

To enable debugging mode in OpenCL CPU Compiler & Runtime or FPGA Fast Emulator,
specific options should be passed to the build options string parameter in
``clBuildProgram`` function.

``-g``
    The flag enables generation of source-level debug information

    .. note:: passing ``-g`` flag also means that no optimizations
      (such as inlining, unrolling, vectorization etc) will be performed
      as if `-cl-opt-disable` option was passed.


``-s`` */full/path/to/OpenCL/source/file.cl*
    The option specifies full path to the OpenCL source file.
    If the path includes spaces, the entire path should be enclosed with
    double or single quotes.

    This option is required in case of source of OpenCL kernel(s) is(are)
    passed to ``clCreateProgramWithSource`` function as a string.

    If the file contains one or more ``#include`` directives, it is not needed
    to add multiple paths for included files to ``-s`` option.

clBuildProgram example
^^^^^^^^^^^^^^^^^^^^^^

::

  err = clBuildProgram(
          g_program,
          0,
          NULL,
          "-g -s \"<path_to_opencl_source_file>\"",
          NULL,
          NULL);

.. note:: The OpenCL kernel code must exist in a text file,
  separate from the host code. Debugging OpenCL code that appears only
  in a string embedded in the host application is not supported.

Instead of passing ``-s`` option to specify a path to an OpenCL source file
one of the follow approaches can be used:

* Use Intel Offline Compiler (IOC).
  Using Intel Offline Compiler it is not needed to specify ``-s`` option,
  the path to an OpenCL source file is defined by ``-input`` option.

  ::

    ioc -cmd=build -input=kernel.cl '-bo=-g'

* Pass to ``clCreateProgramWithSource`` a string with one or more ``#include``
  directives. In the case paths to included files will be detected
  automatically.

  ::

    const char* src = "#include kernel.cl";
    cl_program program = clCreateProgramWithSource(
          m_context, 1, &src, NULL, &error);


Start debugging session
=======================

The normal GDB or LLDB commands are used for debug OpenCL programs:

::

  gdb --args ./host_program

Breakpoints and stepping functionality are fully supported.

For more information about GDB or LLDB debuggers, please, visit
`GDB: The GNU Project Debugger <https://www.gnu.org/software/gdb/>`__ or
`The LLDB Debugger <https://lldb.llvm.org/lldb-gdb.html>`__.

GDB gebugging session example
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. Start debugging of a program

   ::

     gdb --args ./host_program

2. Set a breakpoint in a kernel

   ::

     (gdb) break kernel.cl:5
     Make breakpoint pending on future shared library load? (y or [n]) y
     Breakpoint 1 (kernel.cl:5) pending.

3. Run the host program. Execution will be stopped once the debugger hits the
   breakpoint in the kernel.

   ::

     (gdb) run
     Thread 19 "debugger_test_t" hit Breakpoint 1, foo (c=9 '\t') kernel.cl:5
     5         d = d + 2;

Conditional Breakpoints on work items
=====================================

According to the OpenCL standard, work-items execute OpenCL kernels
simultaneously. So, if a work-group contains more than one work-item
the debugger stops on a breakpoint in every running work-item.

While a kernel compiled with ``-g`` flag the compiler generates three variables
which allow to define the current work item - one for each range in 3D NDRange:
* ``__ocl_dbg_gid0``
* ``__ocl_dbg_gid1``
* ``__ocl_dbg_gid2``

These variables can be used to set conditional breakpoints to make possible stop
on a specific work item (or work items).

Native debugger syntax
^^^^^^^^^^^^^^^^^^^^^^

The native GDB or LLDB commands for conditional breakpoints are supported.

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


GDB Plugin for OpenCL: Work-Item Breakpoint Command
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The plugin extends GDB commands sintax to make the work with conditional
breakpoints on work-items easier.
This feature is available only if GDB was configured using --with-python.

``ocl-break-workitem`` *location* *work-item*

    Set a breakpoint at the given *location*, which can specify a function name,
    a line number, or an address of an instruction.
    *work-item* speficies a work-item number in 3D NDRange, where the number of
    each range is separeted by a space. For example, 0 0 0.

To enable OpenCL work-item breakpoints ``libintelocl.so-gdb.py`` file should be
sourced from a GDB session.

::

  (gdb) source libintelocl.so-gdb.py

Set a breakpoint on a specific work-item using ``ocl-break-workitem`` command:

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

Known issues
============

1. For better debugging expirience GDB 7.12 or higher is required.
   In case of an older GDB version is used, some issues related to JIT code
   debugging may occur (such as no automatic breakpoints reset between runs,
   fail to stop on a breakpoint for second and other runs, etc).

2. ``finish`` GDB or LLDB command returns to the same line as a function call.
   ``finish`` command should advance an instruction pointer to the next
   instructuion after the call. It is expected that it will point to the next
   line after the line of calling, except a callee function returns a value
   for using. In case of OpenCL kernel code it will point to the same line as
   the call even the callee function is a void one.

3. Setting ``__local`` variables has no effect on program execution.
   A try to set variables in ``local`` address space has no effect. The new
   value will be discarded just after the next step.

4. Conditional breakpoints on work-items may take too much time to evaluate.
   It may take too much time to evaluate conditional breakpoints depended on
   a work item number for heavy NDRange kernels. To avoid such breakpoints
   the kernel source code can be modified using ``get_global_id`` function
   directly.

References
==========
