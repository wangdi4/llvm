; Copyright (c) Microsoft Corporation.  All rights reserved.

INCLUDELIB LIBCMTD
INCLUDELIB OLDNAMES

PUBLIC  hw_xgetbv

_TEXT	SEGMENT

Emit_VZeroUpper PROC
	; Encode VZEROUPPER. MASM does not know this instruction.
    DB 197, 248, 119
    ret 0
Emit_VZeroUpper ENDP

; XGETBV result
UINT64  typedef qword
XGETBV_PARAMS struc
    low_part    UINT64  ?
    high_part   UINT64  ?
XGETBV_PARAMS ends 

; XGETBV call
hw_xgetbv PROC
	mov		r8, rcx
	mov		rcx, 0
	; XGETBV return result in EDX:EAX
	xgetbv
	mov     (XGETBV_PARAMS ptr [r8]).low_part, rax
	mov     (XGETBV_PARAMS ptr [r8]).high_part, rdx
	ret
hw_xgetbv ENDP

_TEXT	ENDS
END
