/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  atomics_builtin_f.cl


\*****************************************************************************/

__kernel void atomic_xchg__globalfloat(__global float *buf_in,
                                       __global float *buf_out0,
                                       __global float *buf_out1) {
  float val = buf_in[0];

  __global float *p = ((__global float *)&buf_out0[0]);
  buf_out1[0] = atomic_xchg(p, val);
}

__kernel void atomic_xchg__localfloat(__local float *buf_in,
                                      __local float *buf_out0,
                                      __local float *buf_out1) {
  float val = buf_in[0];

  __local float *p = ((__local float *)&buf_out0[0]);
  buf_out1[0] = atomic_xchg(p, val);
}