.file "linux64_utils.s"

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
.equ ARG1,%rdi

## structure definition for hw_xgetbv()
        .struct 0
MEM_RAX:
        .struct MEM_RAX + 8
MEM_RDX:

.text
#------------------------------------------------------------------------------
#  hw_xgetbv (
#       XGETBV_PARAMS * ARG1
#  )
#
#  Execute xgetbv instruction
#
#------------------------------------------------------------------------------
.globl hw_xgetbv
hw_xgetbv:
        mov		$0, %rcx
        # XGETBV return result in EDX:EAX
        .byte 15
        .byte 1
        .byte 208
        mov     %rax, MEM_RAX (ARG1)
        mov     %rdx, MEM_RDX (ARG1)
        ret
	
.macro EMIT_VZEROUPPER
        .ascii "\xC5\xF8\x77"
.endm

.globl Emit_VZeroUpper
Emit_VZeroUpper:
      EMIT_VZEROUPPER
      ret

.section .note.GNU-stack,"",@progbits
