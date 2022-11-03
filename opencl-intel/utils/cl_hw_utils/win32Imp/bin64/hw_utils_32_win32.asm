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

.CODE

;****************************************************************************
;*
;* Register usage
;*
;* Caller-saved and scratch:
;*  eax
;*  edx
;*  ecx
;*
;* Callee-saved
;*  ebp
;*  ebx
;*  esi
;*  edi
;*  esp
;*
;****************************************************************************

;------------------------------------------------------------------------------
;
; Execute assembler 'pause' instruction
;
;void ASM_FUNCTION hw_pause( void );
;------------------------------------------------------------------------------
hw_pause PROC STDCALL
    pause
    ret
hw_pause ENDP

END
