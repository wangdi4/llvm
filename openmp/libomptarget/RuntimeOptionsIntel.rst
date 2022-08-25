.. INTEL_CUSTOMIZATION

..
  INTEL CONFIDENTIAL
 
  Modifications, Copyright (C) 2022 Intel Corporation
 
  This software and the related documents are Intel copyrighted materials, and
  your use of them is governed by the express license under which they were
  provided to you ("License"). Unless the License provides otherwise, you may not
  use, modify, copy, publish, distribute, disclose or transmit this software or
  the related documents without Intel's prior written permission.
 
  This software and the related documents are provided as is, with no express
  or implied warranties, other than those that are expressly stated in the
  License.

Offload Runtime Environment Variables
=====================================

Glossary
--------

Offload
^^^^^^^
Variable is used in device-independent offload runtime (libomptarget), and
plugin may also use the variable if applicable.

Plugin Common
^^^^^^^^^^^^^
Variable is only used in device-dependent plugins, and both Level Zero and
OpenCL plugins use it.

Plugin LevelZero
^^^^^^^^^^^^^^^^
Variable is only used in the Level Zero plugin.

Plugin OpenCL
^^^^^^^^^^^^^
Variable is only used in the OpenCL plugin.

Experimental
^^^^^^^^^^^^
Variable is not exposed/used in the product build.


Offload
-------

``LIBOMPTARGET_DEBUG=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Controls whether or not debugging information will be displayed.
See details in openmp/docs/design/Runtimes.rst

``LIBOMPTARGET_PROFILE=<FileName>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Allows libomptarget to generate time profile output similar to Clang's
``-ftime-trace`` option.
See details in openmp/docs/design/Runtimes.rst

``LIBOMPTARGET_MEMORY_MANAGER_THRESHOLD=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sets the threshold size for which the libomptarget memory manager will handle
the allocation.
See details in openmp/docs/design/Runtimes.rst

**Note**: This is not used with LevelZero/OpenCL plugin.

``LIBOMPTARGET_INFO=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^
Allows the user to request different types of runtime information from
libomptarget.
See details in openmp/docs/design/Runtimes.rst

``OMP_TARGET_OFFLOAD=<Str>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Str> := mandatory | disabled | default

Sets the initial value of the *target-offload-var* ICV.

``<Str>=mandatory:`` Execution is terminated if a device construct or device
memory routine is encountered and the device is not available or is not
supported by the implementation.

``<Str>=disabled:`` The behavior is as if the only device is the host device if
implementation supports.

``<Str>=default:`` Default behavior described in the `OpenMP specification`_'s
execution model.

See details in the `OpenMP specification`_.

.. _`OpenMP specification`: https://www.openmp.org/spec-html/5.1/openmp.html

``LIBOMPTARGET_INTEROP_USE_SINGLE_QUEUE=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Allows user to use a single level0 queue when asynchronous dispatch is
invoked.  

.. code-block:: rst
  <Enable> := 1

**Default**: Disabled


Plugin Common
-------------

``LIBOMPTARGET_PLUGIN=<Name>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Name> := LEVEL0 | OPENCL | CUDA | X86_64 | NIOS2 |
            level0 | opencl | cuda | x86_64 | nios2

Designates offload plugin name to use.
Offload runtime does not try to load other RTLs if this option is used.

**Default**: Undefined

``LIBOMPTARGET_DEBUG=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Displays debug information at the specified level.

``<Num>=0:`` Disabled

``<Num>=1:`` Displays basic debug information from the plugin actions such as
device detection, kernel compilation, memory copy operations, kernel
invocations, and other plugin-dependent actions.

``<Num>=2:`` Additionally displays which GPU runtime API functions are invoked
with which arguments/parameters.

**Default**: Disabled

``LIBOMPTARGET_DATA_TRANSFER_LATENCY=T,<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Adds artificial data transfer latency, <Num> microseconds, for each memory
copy operations.

**Default**: Disabled

``LIBOMPTARGET_DEVICETYPE=<Type>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Type> := GPU | gpu | CPU | cpu

Decides which device type is used.
Only OpenCL plugin supports "CPU" device type.

**Default**: GPU type

``LIBOMPTARGET_PLUGIN_PROFILE=<Enable>[,<Unit>]``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T
  <Unit>   := usec | unit_usec

Enables basic plugin profiling and displays the result when program finishes.
Microsecond is the default unit if ``<Unit>`` is not specified.

**Default**: Disabled

``LIBOMPTARGET_USM_HOST_MEM=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T | t

Enables use of USM host memory if ``#pragma omp requires unified_shared_memory``
is present in the program, and ``omp_target_alloc`` routine is invoked.

**Default**: USM shared memory

**Note**: It appears to be better not to claim this partial support of
``unified_shared_memory`` and remove this feature to avoid confusion.

``LIBOMPTARGET_DYNAMIC_MEMORY_SIZE=<Num>[,<Method>]``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sets the size (in megabyte) of dynamic memory allocatable within a kernel.
``<Method>=0``: Use allocator not supporting free
``<Method>=1``: Use allocator supporting free. This is the default option when
``<Num>`` is greater than zero.

**Default**: 1 (1MB)

``INTEL_ENABLE_OFFLOAD_ANNOTATIONS=<Path>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Enables ITT annotations in the target program if ``<Path>`` is not empty.

**Default**: Disabled

``LIBOMPTARGET_ONEAPI_USE_IMAGE_OPTIONS=<Bool>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Bool> := 1 | T | t | 0 | F | f

Enables/disables use of target build options embedded in the target image.

**Default**: Enabled

``LIBOMPTARGET_ONEAPI_SHOW_BUILD_LOG=<Bool>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Bool> := 1 | T | t | 0 | F | f

Enables/disables printing of the build logs produced by the device compiler
for the target programs.

**Default**: Disabled

``LIBOMPTARGET_ONEAPI_LINK_LIBDEVICE=<Bool>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Bool> := 1 | T | t | 0 | F | f

Enables/disables fallback libdevice linking in the plugins.

**Default**: Disabled

``LIBOMPTARGET_ONEAPI_THIN_THREADS_THRESHOLD=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Num> is a floating point number from [0.0, 1.0] interval.

Loop kernels with known ND-range may be known to have few iterations
and they may not exploit the offload device to the fullest extent.
Let's assume a device has ``N`` total HW threads available,
and the kernel requires ``M`` hardware threads with local work size
set to ``L``.
If ``(M < N * <Num>)``, then we will try to iteratively half ``L``
to increase the number of HW threads used for executing the kernel.
Effectively, we will end up with ``L`` less than the kernel's SIMD width,
so the HW threads will not use all their SIMD lanes.
This should allow more parallelism, because the stalls in the SIMD lanes
will be distributed across more HW threads, and the probability
of having a stall (or a sequence of stalls) on a critical path
in the kernel should decrease.

**Default**: 0.1

Plugin LevelZero
----------------

``LIBOMPTARGET_LEVEL0_COMPILATION_OPTIONS=<Options>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Passes ``<Options>`` when building native target program binaries.
``<Options>`` may include valid OpenCL/Level Zero build options.

``LIBOMPTARGET_LEVEL0_TARGET_GLOBALS=<Disable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Disable> := 0 | F | f

Disables passing ``-cl-take-global-address`` option when building target
program binaries. Disabling this may result in incorrect program behavior.

| **Default**: Enabled

``LIBOMPTARGET_LEVEL0_MATCH_SINCOSPI=<Disable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Disable> := 0 | F | f

Disables passing ``-cl-match-sincospi`` option when building target program
binaries.

**Default**: Enabled

``LIBOMPTARGET_LEVEL0_USE_DRIVER_GROUP_SIZES=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T | t

Enables using local work size (i.e., team size) suggested by Level Zero
runtime.

**Default**: Disabled

``LIBOMPTARGET_DEVICES=<DeviceKind>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <DeviceKind> := DEVICE | SUBDEVICE | SUBSUBDEVICE | ALL |
                  device | subdevice | subsubdevice | all

Controls how subdevices are exposed to users.

``DEVICE/device``: Only top-level devices are reported as OpenMP devices, and
``subdevice`` clause is supported.

``SUBDEVICE/subdevice``: Only 1st-level subdevices are reported as OpenMP
devices, and ``subdevice`` clause is ignored.

``SUBSUBDEVICE/subsubdevice``: Only 2nd-level subdevices are reported as OpenMP
devices, and ``subdevice`` clause is ignored. On Intel GPU using Level Zero
backend, limiting the ``subsubdevice`` to a single compute slice within a tile
also requires setting additional GPU compute runtime environment variable
``CFESingleSliceDispatchCCSMode=1``.

``ALL/all``: All top-level devices and their subdevices are reported as OpenMP
devices, and ``subdevice`` clause is ignored. This is not supported on Intel GPU
and is being deprecated.

**Default**: Equivalent to ``<DeviceKind>=device``

``LIBOMPTARGET_LEVEL0_MEMORY_POOL=<Option>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Option>       := 0 | <PoolInfoList>
  <PoolInfoList> := <PoolInfo>[,<PoolInfoList>]
  <PoolInfo>     := <MemType>[,<AllocMax>[,<Capacity>[,<PoolSize>]]]
  <MemType>      := all | device | host | shared
  <AllocMax>     := positive integer or empty, max allocation size in MB
  <Capacity>     := positive integer or empty, number of allocations from a
                    single block
  <PoolSize>     := positive integer or empty, max pool size in MB

Controls how reusable memory pool is configured.
Pool is a list of memory blocks that can serve at least ``<Capacity>``
allocations of up to ``<AllocMax>`` size from a single block, with total size
not exceeding ``<PoolSize>``.

**Default**: Equivalent to ``<Option>=device,1,4,256,host,1,4,256,shared,8,4,256``

``LIBOMPTARGET_LEVEL0_USE_COPY_ENGINE=<Value>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Value>   := <Disable> | <Type>
  <Disable> := 0 | F | f
  <Type>    := main | link | all

Controls how to use copy engines for data transfer if device supports.

``0 | F | f``: Disables use of copy engines.
``main``: Enables only main copy engines if device supports.
``link``: Enables only link copy engines if device supports.
``all``: Enables all copy engines if device supports.

**Default**: all

``LIBOMPTARGET_LEVEL0_DEFAULT_TARGET_MEM=<MemType>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <MemType> := DEVICE | HOST | SHARED | device | host | shared

Decides memory type returned by ``omp_target_alloc`` routine.

**Default**: device

``LIBOMPTARGET_LEVEL0_USE_DEVICE_MEM=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T | t

Uses device memory type for ``omp_target_alloc`` routine.

**Note**: Default is already *device*, so we should remove this

``LIBOMPTARGET_LEVEL0_SUBSCRIPTION_RATE=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sets over-subscription parameter that is used when computing the team
size/counts for a target region.

**Default**: 4

``LIBOMPTARGET_ONEAPI_REDUCTION_SUBSCRIPTION_RATE=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sets under-subscription parameter that is used when computing the team
counts for a target region that requires cross-team reduction updates.

  <Num> is a number greater than or equal to 0.

'0' disables special handling for kernels with reductions, so
``LIBOMPTARGET_LEVEL0_SUBSCRIPTION_RATE`` takes the effect.

**Default**: 8 for discrete devices, 1 for non-discrete devices or/and
for kernels that use atomic-free reductions.

``LIBOMPTARGET_LEVEL0_KERNEL_WIDTH=<Width>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Width> := 8 | 16 | 32

Forces use of ``<Width>`` when computing the team size/counts for a target
region.

**Default**: Use existing kernel property

``LIBOMPTARGET_LEVEL0_STAGING_BUFFER_SIZE=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sets the staging buffer size to ``<Num>`` KB.
Staging buffer is used in copy operations between host and device as a
temporary storage for two-step copy operation. The buffer is only used for
discrete devices.

**Default**: 16

``LIBOMPTARGET_LEVEL_ZERO_COMMAND_BATCH=<Value>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Value> := <Type>[,<Count>]
  <Type>  := none | NONE | copy | COPY | compute | COMPUTE
  <Count> := maximum number of commands to batch

Enables command batching for a target region.

``<Type>=none|NONE``: Disables command batching.
``<Type>=copy|COPY``: Enables command batching for a target region for data
transfer.
``<Type>=compute|COMPUTE``: Enables command batching for a target region for
data transfer and compute, disabling use of copy engine.

If ``<Type>`` is either ``copy`` or ``compute`` (enabled) and ``<Count>`` is not
specified, batching is performed for all eligible commands for the target
region.

**Default**: ``<Type>=none`` (Disabled)

``LIBOMPTARGET_LEVEL_ZERO_USE_MULTIPLE_COMPUTE_QUEUES=<Bool>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Bool> := 1 | T | t | 0 | F | f

Enables/disables using multiple compute queues for multiple host threads if the
device supports.

**Default**: Disabled

``LIBOMPTARGET_LEVEL_ZERO_USE_IMMEDIATE_COMMAND_LIST=<Bool>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Bool> := 1 | T | t | 0 | F | f

Enables/disables using immediate command list for kernel submission.

**Default**: Disabled

Plugin OpenCL
-------------

``LIBOMPTARGET_OPENCL_DATA_TRANSFER_METHOD=<Method>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Method> := 0 | 1 | 2

Uses the specified method when performing memory copy operations.
This is only effective when ``LIBOMPTARGET_OPENCL_USE_SVM=1``.

``<Method>=0``: Uses ``clEnqueueRead/WriteBuffer`` API function on a temporary
OpenCL buffer (``cl_mem``) created from a SVM pointer.

``<Method>=1``: Uses ``clEnqueueSVMMap/Unmap`` API function.

``<Method>=2``: Uses ``clEnqueueSVMMemcpy`` API function.

**Default**: ``<Method>=1`` if ``LIBOMPTARGET_OPENCL_USE_SVM=1``

``LIBOMPTARGET_OPENCL_SUBSCRIPTION_RATE=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sets over-subscription parameter that is used when computing the team
size/counts for a target region.

**Default**: 4

``LIBOMPTARGET_ONEAPI_REDUCTION_SUBSCRIPTION_RATE=<Num>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Sets under-subscription parameter that is used when computing the team
counts for a target region that requires cross-team reduction updates.

  <Num> is a number greater than or equal to 0.

'0' disables special handling for kernels with reductions, so
``LIBOMPTARGET_OPENCL_SUBSCRIPTION_RATE`` takes the effect.

**Default**: 8 for discrete devices, 1 for non-discrete devices or/and
for kernels that use atomic-free reductions.

``LIBOMPTARGET_ENABLE_SIMD=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> : 1 | T

TODO

``LIBOMPTARGET_OPENCL_INTEROP_QUEUE=<QueueType>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <QueueType> := inorder_async | inorder_shared_sync

Decides queue properties used in a custom interop object.
Custom interop is different from OpenMP 5.1 interop and is not user-facing
interface.

``<QueueType>=inorder_async``: Returns a new in-order OpenCL queue for interop
objects created for asynchronous usage.

``<QueueType>=inorder_shared_sync``: Returns an existing in-order OpenCL queue
for interop obejcts created for synchronous usage.

**Default**: New in-order queue for synchronous, existing out-of-order queue for
asynchronous usage.

``LIBOMPTARGET_OPENCL_COMPILATION_OPTIONS=<Options>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Passes ``<Options>`` when compiling target programs.
``<Options>`` may include valid OpenCL build options.

``LIBOMPTARGET_OPENCL_LINKING_OPTIONS=<Options>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Passes ``<Options>`` when linking target programs.
``<Options>`` may include valid OpenCL build options.

``LIBOMPTARGET_OPENCL_TARGET_GLOBALS=<Disable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Disable> := 0 | F | f

Disables passing ``-cl-take-global-address`` option when building target program
binaries. Disabling this may result in incorrect program behavior.

**Default**: Enabled

``LIBOMPTARGET_OPENCL_MATCH_SINCOSPI=<Disable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Disable> := 0 | F | f

Disables passing ``-cl-match-sincospi`` option when building target program
binaries.

**Default**: Enabled

``LIBOMPTARGET_OPENCL_USE_DRIVER_GROUP_SIZES=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T | t

Enables using local work size (i.e., team size) suggested by OpenCL runtime.

**Default**: Disabled

``LIBOMPTARGET_OPENCL_USE_SVM=<Bool>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Bool> := 1 | T | t | 0 | F | f

Enables/disables using SVM memory for default memory type.

**Default**: Disabled (USM device by default)

``LIBOMPTARGET_OPENCL_USE_BUFFER=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T | t

Enables using OpenCL buffer (``cl_mem``) for memory allocated by
``omp_target_alloc`` routine.

**Default**: Disabled

``LIBOMPTARGET_OPENCL_USE_SINGLE_CONTEXT=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T | t

Enables using a single OpenCL context for all devices under the same platform.

**Default**: Disabled (single context per device)


Experimental
------------

``LIBOMPTARGET_DUMP_TARGET_IMAGE=<Enable>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <Enable> := 1 | T | t

Dumps target binaries embeded in the fat binary to the current directory.

**Default**: Disabled

``LIBOMPTARGET_LOCAL_WG_SIZE=<SizeDesc>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <SizeDesc> := {<NumX>,<NumY>,<NumZ>}

Forces using the specified size description for local work size (team size).
This is for internal experiments and may not work correctly in certain cases.

``LIBOMPTARGET_GLOBAL_WG_SIZE=<SizeDesc>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: rst

  <SizeDesc> := {<NumX>,<NumY>,<NumZ>}

Forces using the specified size description for global work size (team size *
team count). This is for internal experiments and may not work correctly in
certain cases.

.. END INTEL_CUSTOMIZATION
