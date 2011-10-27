// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#ifdef CL_BUILTIN_FUNCTIONS_EXPORTS
#define VATOMICS_FUNC_DECL __declspec(dllexport)
#else
#define VATOMICS_FUNC_DECL __declspec(dllimport)
#endif
#define __global
#define __local

#else
#define VATOMICS_FUNC_DECL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
VATOMICS_FUNC_DECL int __atom_add_gi32(__global volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_sub_gi32(__global volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_xchg_gi32(__global volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_inc_gi32(__global volatile int *p);
VATOMICS_FUNC_DECL int __atom_dec_gi32(__global volatile int *p);
VATOMICS_FUNC_DECL int __atom_cmpxchg_gi32(__global volatile int *p, int cmp, int val);
VATOMICS_FUNC_DECL int __atom_min_gi32(__global volatile int *p,int val);
VATOMICS_FUNC_DECL int __atom_max_gi32(__global volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_and_gi32(__global volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_or_gi32(__global volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_xor_gi32(__global volatile int *p, int val);

VATOMICS_FUNC_DECL unsigned int __atom_add_gu32(__global volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_sub_gu32(__global volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_xchg_gu32(__global volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_inc_gu32(__global volatile unsigned int *p);
VATOMICS_FUNC_DECL unsigned int __atom_dec_gu32(__global volatile unsigned int *p);
VATOMICS_FUNC_DECL unsigned int __atom_cmpxchg_gu32(__global volatile unsigned int *p, unsigned int cmp, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_min_gu32(__global volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_max_gu32(__global volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_and_gu32(__global volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_or_gu32(__global volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_xor_gu32(__global volatile unsigned int *p, unsigned int val);

VATOMICS_FUNC_DECL int __atom_add_li32(__local volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_sub_li32(__local volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_xchg_li32(__local volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_min_li32(__local volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_max_li32(__local volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_inc_li32(__local volatile int *p);
VATOMICS_FUNC_DECL int __atom_dec_li32(__local volatile int *p);
VATOMICS_FUNC_DECL int __atom_cmpxchg_li32(__local volatile int *p, int cmp, int val);
VATOMICS_FUNC_DECL int __atom_and_li32(__local volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_or_li32(__local volatile int *p, int val);
VATOMICS_FUNC_DECL int __atom_xor_li32(__local volatile int *p, int val);

VATOMICS_FUNC_DECL unsigned int __atom_add_lu32(__local volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_sub_lu32(__local volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_xchg_lu32(__local volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_min_lu32(__local volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_max_lu32(__local volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_inc_lu32(__local volatile unsigned int *p);
VATOMICS_FUNC_DECL unsigned int __atom_dec_lu32(__local volatile unsigned int *p);
VATOMICS_FUNC_DECL unsigned int __atom_cmpxchg_lu32(__local volatile unsigned int *p, unsigned int cmp, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_and_lu32(__local volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_or_lu32(__local volatile unsigned int *p, unsigned int val);
VATOMICS_FUNC_DECL unsigned int __atom_xor_lu32(__local volatile unsigned int *p, unsigned int val);
#endif
#ifdef __cplusplus
}
#endif
