; INTEL CONFIDENTIAL
;
; Copyright 2022 Intel Corporation.
;
; This software and the related documents are Intel copyrighted materials, and
; your use of them is governed by the express license under which they were
; provided to you (License). Unless the License provides otherwise, you may not
; use, modify, copy, publish, distribute, disclose or transmit this software or
; the related documents without Intel's prior written permission.
;
; This software and the related documents are provided as is, with no express
; or implied warranties, other than those that are expressly stated in the
; License.


; Copyright (c) Microsoft Corporation.  All rights reserved.

INCLUDELIB LIBCMTD
INCLUDELIB OLDNAMES

PUBLIC  hw_xgetbv

_TEXT  SEGMENT

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
  mov    r8, rcx
  mov    rcx, 0
  ; XGETBV return result in EDX:EAX
  xgetbv
  mov     (XGETBV_PARAMS ptr [r8]).low_part, rax
  mov     (XGETBV_PARAMS ptr [r8]).high_part, rdx
  ret
hw_xgetbv ENDP

_TEXT  ENDS
END
