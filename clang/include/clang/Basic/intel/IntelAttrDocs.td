//===--- IntelAttrDocs.td - Documentation for intel-specific attrs --------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

def NumComputeUnitsDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
This attribute is documented in `Intel FPGA SDK for OpenCL Programming Guide`_.
See sections *Specifying Number of Compute Units* and *Kernel
replication using the num_compute_units(X, Y, Z) Attribute*.

To increase the data-processing efficiency of an OpenCL kernel, you can instruct
the Intel FPGA SDK for OpenCL Offline Compiler to generate multiple kernel
compute units. Each compute unit is capable of executing multiple work-groups
simulteneously.

.. warning:: CAUTION:
   Multiplying the number of kernel compute units increases data throughput at
   the expense of global memory bandwidth contention among compute units.

To specify the number of compute units for a kernel, inster the
``num_compute_units(N)`` attribute int the kernel source code.

For example, the code fragment below directs the offline compiler to instantiate
two compute units in a kernel:

.. code-block:: c

  __attribute__((num_compute_units(2)))
  __kernel void test(__global const float * restrict a,
                     __global const float * restrict b,
                     __global float * restrict answer)
  {
    size_t gid = get_global_id(0);
    answer[gid] = a[gid] + b[gid];
  }

The offline compiler distributes work-groups across the specified number of
compute units.

You can replicate your single-work-item OpenCL kernel by including the
``num_compute_units(X, Y, Z)`` kernel attribute.
As mentioned in *Specifying Number of Compute Units*, including the
``num_compute_units(N)`` kernel attribute in your kernel instructs the Intel
FPGA SDK for OpenCL Offline Compiler to generate multiple compute units to
process data. Since the offline compiler processes a single work-item kernel in
one compute unit, the ``num_compute_units(N)`` attribute instructs the offline
compiler to generate *N* identical copies of the kernel in hardware.

.. _`Intel FPGA SDK for OpenCL Programming Guide`: https://www.altera.com/documentation/mwh1391807965224.html#mwh1391807939093
  }];
}

def AutorunDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
This attribute is documented in `Intel FPGA SDK for OpenCL Programming Guide`_.
See section *Omit Communication Hardware between the Host and the Kernel*.

The ``autorun`` kernel attribute instructs the Intel FPGA SDK for OpenCL Offline
Compiler to omit logic that is used for communication between the host and the
kernel. A kernel that uses the ``autorun`` attribute starts executing
automatically before any kernel that the host launches explicitly. In addition,
this kernel restarts automatically as soon as it finishes its execution.
The ``autorun`` kernel attribute notifies the offline compiler that the kernel
runs on its own and will not be enqueued by any host.

To leverage the ``autorun`` attribute, a kernel must meet all of the following
criteria:

1. Does not use I/O channels.

  .. note:: Kernel-to-kernel channels are supported

2. Does not have any arguments

3. Has either the ``max_global_work_dim(0)`` attribute or the
   ``reqd_work_groups_size(X, Y, Z)`` attribute.

   .. note:: The parameters of the ``reqd_work_group_size(X, Y, Z)`` attribute
      must be divisors of 2^32.

As mentioned above, kernels with the ``autorun`` attribute cannot have any
arguments and start executing without the host launching them explicitly. As a
result, the offline compiler does not need to generate the logic for
communication between the host and the kernel. Omitting this logic reduces
utilization and allows the offline compiler to apply additional performance
optimizations.

A typical use case for the ``autorun`` attribute is a kernel that reads data
from one of more kernel-to-kernel channels, processes the data, and the writes
the results to one or more channels.

.. code-block:: c

  channel int chan_in;
  channel int chan_out;

  __attribute__((max_global_work_dim(0)))
  __attribute__((autorun))
  __kernel void plusOne() {
    while (1) {
      int data_int = read_channel_intel(chan_in);
      write_channel_intel(chan_out, data_in + 1);
    }
  }

.. _`Intel FPGA SDK for OpenCL Programming Guide`: https://www.altera.com/documentation/mwh1391807965224.html#mwh1391807939093
  }];
}

def UsesGlobalWorkOffsetDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
The ``uses_global_work_offset`` kernel attribute indicates whether the host is allowed to enqueue
the kernel with a non-zero global_work_offset argument using the clEnqueueNDRangeKernel API.

A value of ``uses_global_work_offset(0)`` instructs the Intel FPGA SDK for OpenCL Offline
Compiler to assume that calls to clEnqueueNDRangeKernel will always have a NULL or all zero
global_work_offset argument. It is recommended to apply the ``uses_global_work_offset(0)`` attribute
on all kernels that are always enqueued with a zero global_work_offset to allow the offline
compiler to perform additional optimizations. The host will prevent kernels marked with
``uses_global_work_offset(0)`` from being enqueued with a non-zero global_work_offset.

A value of ``uses_global_work_offset(1)`` instructs the Intel FPGA SDK for OpenCL Offline
Compiler to assume that calls to clEnqueueNDRangeKernel may have a non-zero global_work_offset
argument. The ``uses_global_work_offset(1)`` attribute matches the default behavior
of the offline compiler. It is recommended to apply the ``uses_global_work_offset(1)``
attribute only to kernels that are enqueued with non-zero global_work_offset arguments.
  }];
}

def OpenCLDepthDocs : Documentation {
  let Category = DocCatVariable;
  let Content = [{
You may have buffered or unbuffered channels/pipes in your kernel program. If there are
imbalances in channel/pipe read and write operations, create buffered channels/pipes to prevent
kernel stalls by including the depth attribute in your channel/pipe declaration.
Buffered channels/pipes decouple the operation of concurrent work-items executing in
different kernels.
If you expect any temporary mismatch between the consumption rate and the
production rate to the channel/pipe, set the buffer size using the depth
attribute.

Specify the depth attribute for the pipe arguments. Assign a depth attribute
value that equals to the maximum number of packets that the pipe creates to
hold in the host.
  }];
}

def OpenCLIODocs : Documentation {
  let Category = DocCatVariable;
  let Content = [{
Include an io attribute in your OpenCL channel/pipe declaration to declare a
special I/O channel/pipe to interface with input or output features of an FPGA
board. These features might include network interfaces, PCIe, cameras, or other
data capture or processing devices or protocols.

In the Intel FPGA SDK for OpenCL channels extension, the io("chan_id") attribute
specifies the I/O feature of an accelerator board with which a channel
interfaces. The chan_id argument is the name of the I/O interface listed in the
board_spec.xml file of your Custom Platform. The same I/O features can be used
to identify I/O pipes.
  }];
}

def OpenCLBufferLocationDocs : Documentation {
  let Category = DocCatVariable;
  let Content = [{
The board support package for your FPGA board can assemble a global memory space
consisting of different memory technologies (for example, DRAM or SRAM). The
board support package designates one such memory, which might consist of
multiple interfaces, as the default memory. All buffers reside there.

To instruct the host to allocate a buffer to a specific global memory type,
insert the buffer_location("<memory_type>") attribute, where <memory_type> is
the name of the global memory type provided by your board vendor.

If you do not specify the buffer_location attribute, the host allocates the
buffer to the default memory type automatically. To determine the default memory
type, consult the documentation provided by your board vendor. Alternatively, in
the board_spec.xml file of your Custom Platform, search for the memory type that
is defined first or has the attribute default=1 assigned to it.
  }];
}

def VecLenHintDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
The optional __attribute__((intel_vec_len_hint(<int>))) can be used to provide
a hint to the compiler that the kernel performs the best if vectorized to
the specified vector length.

The lengths accepted by the attribute are:

``0``
  The compiler makes heuristic-based decision whether to vectorize the kernel,
  and if so which vector length to use (default behavior).

``1``
  No vectorization by the compiler. Explicit vector data types in kernels are
  left intact.

``4,8,16``
  Disables heuristic and vectorizes to the length of 4,8,16 respectively.

Other values are invalid. An error will be reported during compilation.

An error will be given if:
  - Specified value is not in range of supported values.
  }];
}

def OpenCLBlockingDocs : Documentation {
  let Category = DocCatVariable;
  let Content = [{
By default, pipes exhibit nonblocking behavior. If you want the pipes in your
kernel to exhibit blocking behavior, specify the blocking attribute
(__attribute__((blocking))) when you declare the read and write pipes.
  }];
}

def OpenCLHostAccessibleDocs : Documentation {
  let Category = DocCatVariable;
  let Content = [{
By default OpenCL pipes cannot be accessed from host. Optional
__attribute__((intel_host_accessible)) may be applied to pipe kernel arguments
to specify that a pipe kernel argument will be connected by the host to a
host-accessible pipe, and therefore will not be connected to another kernel pipe
argument.
  }];
}

def ReadWriteModeDocs : Documentation {
 let Category = DocCatType;
  let Content = [{
The readwrite_mode attribute indicates to the compiler how the agent memory
will be accessed from outside the component.

__attribute__((readwrite_mode("readonly"))) indicates to the compiler that
users from outside the component can only read from the agent interface.
__attribute__((readwrite_mode("writeonly"))) indicates to the compiler that
users from outside the component can only write to the agent interface.
__attribute__((readwrite_mode("readwrite"))) indicates to the compiler that
users from outside the component can read from and write to the agent
interface.
  }];
}

def IntelOclBiccAVXDocs : Documentation {
  let Category = DocCatType;
  let Content = [{
On x86 targets, this attribute changes the calling convention of a function to
match with the calling convention of Intel OpenCL SVML AVX/AVX2 builtin.
 }];
}

def IntelOclBiccAVX512Docs : Documentation {
  let Category = DocCatType;
  let Content = [{
On x86 targets, this attribute changes the calling convention of a function to
match with the calling convention of Intel OpenCL SVML AVX512 builtin.
 }];
}

def AllowCPUFeatureDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
    the ``allow_cpu_features`` attribute works similarly to attribute ``target``
    in that it permits the use of intrinsic functions and architecture specific
    functionality in the attributed function. Unlike ``target``, it does NOT
    cause multiversioning, and otherwise follows normal language rules.

    The ``allow_cpu_features`` attribute accepts two integral constant expression
    arguments that evaluate to a bitmask of permissible features from the libirc
    cpu-id information. The evaluated type of these is an unsigned 64-bit integer
    which permits use of template-dependent code. The first parameter is for the
    page 1 bitmask, and the second parameter is the page 2 bitmask. The second
    parameter is optional and can be omitted.

.. code-block:: c

  #include <immintrin.h> // contains all of the _FEATURE values.
  __attribute__((allow_cpu_features(_FEATURE_AVX2)))
  void my_function() {
    // Code that either uses AVX2 features, or we want to be optimized for AVX2.
  }

  [[clang::allow_cpu_features(_FEATURE_SSE3)]]
  void my_function2() {
    // Code that either uses SSE3 features, or we want to be optimized for AVX2.
  }

  template<uint64_t FEATP1, uint64_t FEATP2>
  __attribute__((allow_cpu_features(FEATP1, FEATP2)))
  void my_dependent_function() {
    // Code that we want optimized for the included features.
  }

  }];
}
