.. ..

  <!---
  Copyright (C) 2022 Intel Corporation

  This software and the related documents are Intel copyrighted materials, and
  your use of them is governed by the express license under which they were
  provided to you ("License"). Unless the License provides otherwise, you may
  not use, modify, copy, publish, distribute, disclose or transmit this
  software or the related documents without Intel's prior written permission.

  This software and the related documents are provided as is, with no express
  or implied warranties, other than those that are expressly stated in the
  License.
  --->

=============================================
Task Sequence Async API Implementation Design
=============================================

.. contents::
   :local:


Overview
========
The implementation of task sequence in OpenCL CPU runtime mainly contains 2
parts, the middle-end IR handling and the backend runtime support.

We utilize and extend the existing `OpenCL block
<https://www.khronos.org/registry/OpenCL/sdk/2.2/docs/man/html/blocks.html>`_
execution to support task sequence APIs. Each functor
to call in the task sequence is treated as a block or a kernel enqueued by
`enqueue_kernel <https://www.khronos.org/registry/OpenCL/sdk/2.2/docs/man/html/enqueue_kernel.html>`_
in OpenCL C.

The middle-end part fills bodies of a set of built-in functions, and creates a
*block invoke*, which is a special kernel to invoke the real functor.

The backend runtime part implements 4 built-in functions:
``__create_task_sequence``, ``__release_task_sequence``, ``__async`` and
``__get``.  By these functions, we create a device queue for task sequence(s),
allocate memory for the result, enqueue the block invoke and keep the returned
event.


SYCL Spec
=========
The task_sequence class has 4 main member functions -- ctor, dtor, ``async``,
and ``get``.  For the detailed API description, see the `spec drafts
<https://gitlab.devtools.intel.com/SYCL/extensions/-/blob/horobert/task_sequence/early_drafts/task_sequence/task_sequence.asciidoc>`_.


Middle-end Part
===============
The TaskSeqAsyncHandling pass is implemented to handle IR transformation. It
fills bodies of 4 function declarations coming from clang frontend, and also
creates a special kernel function *block invoke* to call the actual function.
The created block invoke can be later enqueued in ``async`` API call.


Built-in Functions
------------------
The 4 member functions of ``task_sequence`` class , ctor, dtor, ``async``, and
``get``, call built-in functions ``__create_task_sequence``,
``__release_task_sequence``, ``__async``, and ``__get``, as described in the
Spec. In the `SYCL implementation
<https://github.com/intel-restricted/applications.compilers.llvm-project/pull/1014/files#diff-2ef00819848286c27d45bc8d5a4acc81912efe647e0e5c335459904856afc2da>`_,
the 4 functions are actually templates named as
``__spirv_TaskSequenceCreateINTEL``, ``__spirv_TaskSequenceReleaseINTEL``,
``__spirv_TaskSequenceAsyncINTEL``, and ``__spirv_TaskSequenceGetINTEL``,
respectively. Details of these built-in functions can be found in the `SPIRV-V
Spec <https://gitlab.devtools.intel.com/jdgarvey/specs/-/blob/horobert/task_sequence/SPV_INTEL_fpga_task_sequence.asciidoc>`_.

The 4 ``__spirv`` functions above are declarations without bodies when
generated from clang frontend, and we need to fill their bodies in OpenCL
optimizer.

Note that we don't implement these 4 functions directly in built-in library,
as they are templates with different argument types, and thus they have
various mangled names and types from distinct task_sequence instances.

The pseudo C code of the bodies of the 4 built-in functions are as follows. The
called ``__create_task_sequence``, ``__release_task_sequence``, ``__async`` and
``__get`` are implemented in backend built-in library, and they are described
in :ref:`backend part <backendruntime>`.

.. code-block:: c

  size_t __spirv_TaskSequenceCreateINTEL(task_sequence *obj, f_t *f) {
    // Parse return_type_size from f_t
    void *impl = __create_task_sequence(return_type_size);
    return (size_t)impl;
  }

  void __spirv_TaskSequenceReleaseINTEL(task_sequence *obj) {
    __release_task_sequence((void *)obj);
  }

  void __spirv_TaskSequenceAsyncINTEL(
      task_sequence *obj, f_t *f, size_t id, unsigned capacity, ...) {
    invoke = __spirv_TaskSequenceAsyncINTEL.block_invoke_mapper(f);
    // Create literal;
    __async((void *)obj, capacity, (void *)invoke, (void *)literal);
  }

  ReturnT __spirv_TaskSequenceGetINTEL(task_sequence *obj, f_t *f,
                                       size_t id, unsigned capacity) {
    void *ret = __get((void *)obj, capacity);
    return *(ReturnT *)ret;
  }


The function ``__spirv_TaskSequenceAsyncINTEL.block_invoke_mapper`` is used to
obtain the corresponding block invoke of the async function passed
``__spirv_TaskSequenceAsyncINTEL``, and it is described below.

Block Invoke
------------
Block invoke is a special kernel we create to call the actual function. It
extracts arguments from the *block literal* passed by the runtime, then calls
the actual function, and retrieves the return value and saves it into the
memory pointed by *result address* specified in the block literal. The pseudo
code of the block invoke is as follows. The block literal is described in
:ref:`the following section <blockliteral>`.

.. code-block:: c

  void func._block_invoke_kernel(void *raw_literal) {
    LiteralType *literal = raw_literal;
    args... = load from literal.Arguments...;
    ReturnT *result_addr = literal.ResultAddr;
    *result_addr = func(args...);
  }

Each async function to call has a corresponding block invoke, and thus, we need
to determine the proper block invoke in ``__spirv_TaskSequenceAsyncINTEL``.
A function block invoke mapper
``__spirv_TaskSequenceAsyncINTEL.block_invoke_mapper`` is created for the
purpose. The mapper accepts the async function passed to
``__spirv_TaskSequenceAsyncINTEL``, and returns the corresponding block invoke.

Since async functions and their corresponding block invokes are compile-time
known values, the mapper simply compares the argument and known async functions,
and returns the function pointer of the found block invoke. A typical example
in pseudo C code is as follows.

.. code-block:: c

  void some_func1() {
    __spirv_TaskSequenceAsyncINTEL(..., foo);
  }

  void some_func2() {
    __spirv_TaskSequenceAsyncINTEL(..., bar);
    __spirv_TaskSequenceAsyncINTEL(..., baz);
  }

  // Gerenated mapper
  void *__spirv_TaskSequenceAsyncINTEL.block_invoke_mapper(void *async_func) {
    if (async_func == foo)
      return foo._block_invoke_kernel;
    if (async_func == bar)
      return bar._block_invoke_kernel;
    return baz._block_invoke_kernel;
  }

.. _blockliteral:


Block Literal
-------------
Block literal is a structure used to pass the arguments and the return value
address of the function to call. It is defined as follows.

.. code-block:: c

  struct BlockLiteral {
    unsigned Size;       // The whole size of this structure
    unsigned Align;      // The alignment of this structure
    void *BlockInvoke;   // Pointer to the corresponding block invoke kernel
    Arguments...;        // Arguments to pass to the function to call
    void *ResultAddr;    // The memory address to save the result
  };

A block literal is allocated in ``__spirv_TaskSequenceAsyncINTEL`` call, and
its member ``Size``, ``Align``, ``BlockInvoke`` and all arguments are filled,
and then it's passed to runtime backend, and ``ResultAddr`` is filled, and it's
passed to block invoke later on. Block invoke extracts arguments from it and
saves result to the ``ResultAddr``.


.. _backendruntime:

Backend Runtime Part
====================
``__create_task_sequence``, ``__release_task_sequence``, ``__async`` ``__get``
are finally inlined, within which ``__ocl_task_sequence_create``,
``__ocl_task_sequence_release``, ``__ocl_task_sequence_async``,
``__ocl_task_sequence_get`` will be invoked respectively with extra implicit
runtime parameters.

__ocl_task_sequence_create
------------------------
For each ``task_sequence``, a data structure is allocated. It is defined as
follows.

.. code-block:: c

  struct task_sequence_data {
    std::vector<char *> results;     // Buffers to store returned values
    std::vector<clk_event_t> events; // OCL events to observe async tasks
    size_t result_size;  // Size of every single value returned by __get()
    unsigned delivered;  // Number of __get() being invoked
  };

In this function, the size of result of task(s) in the task sequence is saved
to ``result_size`` in the data structure.  This size is calculated by
middle-end and passed to this function by argument. Then the address of the
data structure is returned as ID of this task sequence.

__ocl_task_sequence_async
-----------------------
The first time to enqueue an async task for the SYCL program, a device queue
only for task sequences will be created.

Before an async task (i.e. block) is enqueued, runtime allocates memory for the
result (whose size is determined by ``result_size``) and fills the memory
address to block literal, and then constructs a single work-item ndrange to
execute the task, and saves result address and event (in ``results`` and
``events``) in the data structure of this task sequence. To ensure the sequence
order in one task sequence, the event of last task is in the wait list of the
current task.

__ocl_task_sequence_get
---------------------
This function waits for the task finishing its execution and returns the memory
address of result. ``delivered`` is used to tell how many ``__get()`` has been
invoked in order to skip waiting for finished tasks and return immediately
once current task finish its execution.

This function could be blocked if current task is executing.

__ocl_task_sequence_release
-------------------------
All results in a task sequence could not be released util destructing the task
sequence object. At that time, the data structure of this task sequence also
should be released.
