//===--- IntelAttrDocs.td - Documentation for intel-specific attrs --------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

def MaxGlobalWorkDimDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
This attribute documented in _Intel FPGA SDK for OpenCL Programming Guide_
(link_). See section _Omit Hardware that Generates and Dispatches Kernel IDs_.

The ``max_global_work_dim(0)`` kernel attribute instructs the Intel FPGA SDK
for OpenCL Offline compiler to omit logic that generates and dispatches global,
local and group IDs into the compiler kernel. Semantically, the
``max_global_work_dim(0)`` kernel attribute specifies that the global work
dimension of the kernel is zero. Setting this kernel attribute means that the
kernel does not use any global, local, or group IDs. The presence of this
attribute in the kernel code serves as a guarantee to the offline compiler that
the kernel is a single work-item kernel.

Example usage:

.. code-block:: c

  channel int chan_in;
  channel int chan_out;

  __attribute__((max_global_work_dim(0)))
  __kernel void plusK(int N, int k) {
    for (int i = 0; i < N; ++i) {
      int data_in = read_channel_intel(chan_in);
      write_channel_intel(chan_out, data_in + k);
    }
  }

If your current kernel implementation has multiple work-items but does not use
global, local, or group IDs, you can use the ``max_global_work_dim(0)`` kernel
attribute if you modify the kernel code accordingly:

1. Wrap the kernel body in a ``for`` loop that iterates as many times as the
   number of work-items.

2. Launch the modified kernel with only one work-item.

.. _link: https://www.altera.com/documentation/mwh1391807965224.html#mwh1391807939093
  }];
}

def NumComputeUnitsDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
This attribute documented in _Intel FPGA SDK for OpenCL Programming Guide_
(link_). See sections _Specifying Number of Compute Units_ and _Kernel
replication using the num_compute_units(X, Y, Z) Attribute_.

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
As mentioned in _Specifying Number of Compute Units_, including the
``num_compute_units(N)`` kernel attribute in your kernel instructs the Intel
FPGA SDK for OpenCL Offline Compiler to generate multiple compute units to
process data. Since the offline compiler processes a single work-item kernel in
one compute unit, the ``num_compute_units(N)`` attribute instructs the offline
compiler to generate _N_ identical copies of the kernel in hardware.

.. _link: https://www.altera.com/documentation/mwh1391807965224.html#mwh1391807939093
  }];
}

def AutorunDocs : Documentation {
  let Category = DocCatFunction;
  let Content = [{
This attribute documented in _Intel FPGA SDK for OpenCL Programming Guide_
(link_). See section _Omit Communication Hardware between the Host and the
Kernel_.

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
retsult, the offline compiler does not need to generate the logic for
communication between the host and the kernel. Ommiting this logic reduces
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

.. _link: https://www.altera.com/documentation/mwh1391807965224.html#mwh1391807939093
  }];
}
