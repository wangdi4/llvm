; Copyright (c) Microsoft Corporation.  All rights reserved.

.386
.model flat, c

PUBLIC _KernelExecute

; Custom Build Step, including a listing file placed in intermediate directory
; but without Source Browser information
; debug:
; ml -c -Zi "-Fl$(IntDir)\$(InputName).lst" "-Fo$(IntDir)\$(InputName).obj" "$(InputPath)"
; release:
; ml -c "-Fl$(IntDir)\$(InputName).lst" "-Fo$(IntDir)\$(InputName).obj" "$(InputPath)"
; outputs:
; $(IntDir)\$(InputName).obj

; Custom Build Step, including a listing file placed in intermediate directory
; and Source Browser information also placed in intermediate directory
; debug:
; ml -c -Zi "-Fl$(IntDir)\$(InputName).lst" "-FR$(IntDir)\$(InputName).sbr" "-Fo$(IntDir)\$(InputName).obj" "$(InputPath)"
; release:
; ml -c "-Fl$(IntDir)\$(InputName).lst" "-FR$(IntDir)\$(InputName).sbr" "-Fo$(IntDir)\$(InputName).obj" "$(InputPath)"
; outputs:
; $(IntDir)\$(InputName).obj
; $(IntDir)\$(InputName).sbr

.code

_KernelExecute PROC STDCALL KERNEL: PTR, CONTEXT: PTR, STACK_PTR:PTR
	push	edi
	mov		edi,			STACK_PTR
	mov		DWORD PTR[edi], esp
	mov		esp,			CONTEXT
	call	KERNEL
	mov		esp,			DWORD PTR[edi]
	pop		edi
; return value already in eax
	ret
_KernelExecute ENDP

end
