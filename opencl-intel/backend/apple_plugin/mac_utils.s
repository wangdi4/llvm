.file "mac_utils.s"

#*****************************************************************************
#*
#*   Copyright (c)  1999 - 2007 Intel Corporation. All rights reserved
#*   This software and associated documentation (if any) is furnished
#*   under a license and monly be used or copied in accordance
#*   with the terms of the license. Except as permitted by such
#*   license, no part of this software or documentation may be
#*   reproduced, stored in a retrieval system, or transmitted in any
#*   form or by any means without the express written consent of
#*   Intel Corporation.
#*
#*
#*   Module Name:
#*
#*     linux64_utils.s
#*
#*   Abstract:
#*
#*****************************************************************************

#================= CallKernel implementation (with and without barriers) ===================
.text
#-------------------------------------------------------------------------------
# Calling convention for X86-64 Linux cdecl:
# RDI := params_size (param#1)
# RSI := pParameters (param#2)
# RDX := pEntryPoint (param#3)
#------------------------------------------------------------------------------- 

.globl CallKernel
CallKernel:
	push      %rbx
	mov       %rsp,   %rbx
	mov       %rdi,   %rcx
	sub       %rcx,   %rsp
	and       $-128,  %rsp
	lea       (%rsp), %rdi
	rep movsb
	call     *%rdx
	mov       %rbx,   %rsp
	pop       %rbx
	ret

#================= Utilities ===================

## structure definition for hw_cpuid()
.set M_RAX, 0
.set M_RBX, M_RAX + 8
.set M_RCX, M_RBX + 8
.set M_RDX, M_RCX + 8

.text
#------------------------------------------------------------------------------
#  void cdecl
#  hw_cpuid (
#       CPUID_PARAMS * %rdi
#  )
#
#  Execute cpuid instruction
#
#------------------------------------------------------------------------------
.globl _hw_cpuid
_hw_cpuid:
        # store regs b, c, d
        pushq   %rbx
        pushq   %rcx
        pushq   %rdx
        # fill regs for cpuid
        mov     M_RAX(%rdi), %rax
        mov     M_RBX(%rdi), %rbx
        mov     M_RCX(%rdi), %rcx
        mov     M_RDX(%rdi), %rdx
        cpuid
        mov     %rax, M_RAX (%rdi)
        mov     %rbx, M_RBX (%rdi)
        mov     %rcx, M_RCX (%rdi)
        mov     %rdx, M_RDX (%rdi)
        # restore regs b, c, d
        popq    %rdx
        popq    %rcx
        popq    %rbx
        ret
# end of hw_cpuid()
   

## structure definition for hw_xgetbv()
.set MEM_RAX, 0
.set MEM_RDX, MEM_RAX + 8

.text
#------------------------------------------------------------------------------
#  hw_xgetbv (
#       XGETBV_PARAMS * %rdi
#  )
#
#  Execute xgetbv instruction
#
#------------------------------------------------------------------------------
.globl _hw_xgetbv
_hw_xgetbv:
        mov		$0, %rcx
        # XGETBV return result in EDX:EAX
        .byte 15
        .byte 1
        .byte 208
        mov     %rax, MEM_RAX (%rdi)
        mov     %rdx, MEM_RDX (%rdi)
        ret
	

