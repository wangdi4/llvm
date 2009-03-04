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

; Define structure for calling kernel
WG_CALL_PARAMS STRUCT
	KRNL_PTR		DWORD PTR	?
	PARAM_SIZE		DWORD		?
	PARAMS			DWORD PTR	?
	EXP_LCL_COUNT	DWORD		?
	LCL_MEM_PTR		DWORD PTR	?
	IMP_LCL_COUNT	DWORD		?
	WI_INFO_STRUCT	BYTE		?
WG_CALL_PARAMS ENDS

WI_CALL_PARAMS STRUCT
	WG_PARAM	DWORD PTR			?
	WG_INFO		DWORD PTR			?
	WI_INFO		DWORD PTR			?
WI_CALL_PARAMS ENDS

.code

_KernelExecute PROC STDCALL CALL_PARAM: PTR
	push	ecx
	push	esi
	push	edi
	push	ebx
	mov		esi,	CALL_PARAM
	; Push Work-Group execution parameters
	mov		esi,	(WI_CALL_PARAMS PTR [esi]).WG_PARAM
	mov		edi,	esi
	xor		ecx,	ecx
	mov		cl,		(WG_CALL_PARAMS PTR [esi]).WI_INFO_STRUCT
	add		ecx,	(WG_CALL_PARAMS PTR [esi]).IMP_LCL_COUNT
	shl		ecx,	2											; Adjust param_count to param_size
	add		ecx,	(WG_CALL_PARAMS PTR [esi]).PARAM_SIZE
	mov		ebx,	ecx
	test	ecx,	ecx
	jz		call_kernel

	shr		ecx,	2											; Parameters alway comes in DWORD	
	mov		esi,	(WG_CALL_PARAMS PTR [esi]).PARAMS
@@:
;	mov		eax,	(WG_CALL_PARAMS PTR [esi]).PARAMS[4*ecx-4]
	mov		eax,	DWORD PTR[esi+4*ecx-4]
	push	eax
	loop	@B
	
call_kernel:
	call	(WG_CALL_PARAMS PTR [edi]).KRNL_PTR
	add		esp,	ebx
	pop		ebx
	pop		edi
	pop		esi
	pop		ecx
; return value already in eax
	ret
_KernelExecute ENDP

end
