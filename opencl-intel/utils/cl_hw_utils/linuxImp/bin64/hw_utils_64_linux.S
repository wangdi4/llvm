.file "hw_utils_64_linux.S"

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
#*     hw_utils_64_linux.S
#*
#*   Abstract:
#*
#*****************************************************************************

.equ ARG1_U64,%rdi

## structure definition for hw_cpuid()
        .struct 0
M_RAX:
        .struct M_RAX + 8
M_RBX:
        .struct M_RBX + 8
M_RCX:
        .struct M_RCX + 8
M_RDX:

.text

#------------------------------------------------------------------------------
#  void cdecl
#  hw_cpuid (
#       CPUID_PARAMS * ARG1_U64
#  )
#
#  Execute cpuid instruction
#
#------------------------------------------------------------------------------
.globl hw_cpuid
hw_cpuid:
        # store regs b, c, d
        pushq   %rbx
        pushq   %rcx
        pushq   %rdx
        # fill regs for cpuid
        mov     M_RAX(ARG1_U64), %rax
        mov     M_RBX(ARG1_U64), %rbx
        mov     M_RCX(ARG1_U64), %rcx
        mov     M_RDX(ARG1_U64), %rdx
        cpuid
        mov     %rax, M_RAX (ARG1_U64)
        mov     %rbx, M_RBX (ARG1_U64)
        mov     %rcx, M_RCX (ARG1_U64)
        mov     %rdx, M_RDX (ARG1_U64)
        # restore regs b, c, d
        popq    %rdx
        popq    %rcx
        popq    %rbx
        ret

#-------------------------------------------------------------------------------
# void cdecl
# hw_pause ()
# 
# Execute pause instruction
#-------------------------------------------------------------------------------
.globl hw_pause
hw_pause:
	pause
	ret

#-------------------------------------------------------------------------------
# void cdecl
# get_next_line_address()
# 
# Return address inside the current image address space.
#-------------------------------------------------------------------------------
.globl get_next_line_address
get_next_line_address:
	movq    0 (%rsp), %rax
	ret

