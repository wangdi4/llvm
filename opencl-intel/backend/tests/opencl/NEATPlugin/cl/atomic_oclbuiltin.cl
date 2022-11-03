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

File Name:  atomics_builtin.cl


\*****************************************************************************/

#define KERNEL_ATOMIC_ONEARG(_func, _type, _attrib)                            \
  __kernel void _func##_attrib##_type(__global uint *buf_in,                   \
                                      __global uint *buf_out0,                 \
                                      __global uint *buf_out1) {               \
    _type *pout = (_type *)&buf_out1[0];                                       \
    _attrib _type *bufuip = ((_attrib _type *)&buf_out0[0]);                   \
    *pout = _func(bufuip);                                                     \
  }

#define KERNEL_ATOMIC_TWOARGS(_func, _type, _attrib)                           \
  __kernel void _func##_attrib##_type(__global uint *buf_in,                   \
                                      __global uint *buf_out0,                 \
                                      __global uint *buf_out1) {               \
    _type val1 = *((_type *)&buf_in[0]);                                       \
    _type *pout = (_type *)&buf_out1[0];                                       \
    _attrib _type *bufuip = ((_attrib _type *)&buf_out0[0]);                   \
    *pout = _func(bufuip, val1);                                               \
  }

#define KERNEL_ATOMIC_CMPXCHG(_func, _type, _attrib)                           \
  __kernel void _func##_attrib##_type(__global uint *buf_in,                   \
                                      __global uint *buf_out0,                 \
                                      __global uint *buf_out1) {               \
    _type val0 = *((_type *)&buf_in[0]);                                       \
    _type val1 = *((_type *)&buf_in[1]);                                       \
    _type *pout = (_type *)&buf_out1[0];                                       \
    _attrib _type *bufuip = ((_attrib _type *)&buf_out0[0]);                   \
    *pout = _func(bufuip, val0, val1);                                         \
  }

KERNEL_ATOMIC_TWOARGS(atomic_add, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_add, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_add, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_add, int, __local)

KERNEL_ATOMIC_TWOARGS(atomic_sub, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_sub, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_sub, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_sub, int, __local)

KERNEL_ATOMIC_TWOARGS(atomic_xchg, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_xchg, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_xchg, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_xchg, int, __local)

KERNEL_ATOMIC_ONEARG(atomic_inc, uint, __global)
KERNEL_ATOMIC_ONEARG(atomic_inc, uint, __local)
KERNEL_ATOMIC_ONEARG(atomic_inc, int, __global)
KERNEL_ATOMIC_ONEARG(atomic_inc, int, __local)

KERNEL_ATOMIC_ONEARG(atomic_dec, uint, __global)
KERNEL_ATOMIC_ONEARG(atomic_dec, uint, __local)
KERNEL_ATOMIC_ONEARG(atomic_dec, int, __global)
KERNEL_ATOMIC_ONEARG(atomic_dec, int, __local)

KERNEL_ATOMIC_CMPXCHG(atomic_cmpxchg, uint, __global)
KERNEL_ATOMIC_CMPXCHG(atomic_cmpxchg, uint, __local)
KERNEL_ATOMIC_CMPXCHG(atomic_cmpxchg, int, __global)
KERNEL_ATOMIC_CMPXCHG(atomic_cmpxchg, int, __local)

KERNEL_ATOMIC_TWOARGS(atomic_min, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_min, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_min, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_min, int, __local)

KERNEL_ATOMIC_TWOARGS(atomic_max, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_max, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_max, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_max, int, __local)

KERNEL_ATOMIC_TWOARGS(atomic_and, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_and, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_and, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_and, int, __local)

KERNEL_ATOMIC_TWOARGS(atomic_or, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_or, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_or, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_or, int, __local)

KERNEL_ATOMIC_TWOARGS(atomic_xor, uint, __global)
KERNEL_ATOMIC_TWOARGS(atomic_xor, uint, __local)
KERNEL_ATOMIC_TWOARGS(atomic_xor, int, __global)
KERNEL_ATOMIC_TWOARGS(atomic_xor, int, __local)