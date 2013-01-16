.file "hw_utils_32_linux.S"

#*****************************************************************************
#*
#*   Copyright (c)  1999 - 2007 Intel Corporation. All rights reserved
#*   This software and associated documentation (if any) is furnished
#*   under a license and may only be used or copied in accordance
#*   with the terms of the license. Except as permitted by such
#*   license, no part of this software or documentation may be
#*   reproduced, stored in a retrieval system, or transmitted in any
#*   form or by any means without the express written consent of
#*   Intel Corporation.
#*
#*
#*   Module Name:
#*
#*     hw_utils_32_linux.S
#*
#*   Abstract:
#*
#*****************************************************************************

## structure definition for hw_cpuid()
        .struct 4
M_RAX:
        .struct M_RAX + 4
M_RBX:
        .struct M_RBX + 4
M_RCX:
        .struct M_RCX + 4
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
        push   %ebx
        push   %ecx
        push   %edx
        # fill regs for cpuid
        mov     M_RAX(%ebp), %eax
        mov     M_RBX(%ebp), %ebx
        mov     M_RCX(%ebp), %ecx
        mov     M_RDX(%ebp), %edx
        cpuid
        mov     %eax, M_RAX (%ebp)
        mov     %ebx, M_RBX (%ebp)
        mov     %ecx, M_RCX (%ebp)
        mov     %edx, M_RDX (%ebp)
        # restore regs b, c, d
        pop    %edx
        pop    %ecx
        pop    %ebx
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
	mov    0 (%esp), %eax
	ret

