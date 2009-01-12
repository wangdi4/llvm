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
WG_CALL_PARAMS ENDS

WI_CALL_PARAMS STRUCT
	WG_PARAM	DWORD PTR			?
	WG_INFO		DWORD PTR			?
	WI_INFO		DWORD PTR			?
WI_CALL_PARAMS ENDS

.code

;_KernelExecute PROC kernelAddress:DWORD, paramAddress:DWORD, paramLength:DWORD
_KernelObsoleteExecute PROC STDCALL CALL_PARAM: PTR
	push	ecx
	push	esi
	push	ebx
	mov		esi,	CALL_PARAM
	; Push Work-Item inforamtion
	mov		eax,	(WI_CALL_PARAMS PTR [esi]).WI_INFO
	push	eax
	; Push Work-Item inforamtion
	mov		eax,	(WI_CALL_PARAMS PTR [esi]).WG_INFO
	push	eax
	; Push Work-Group execution parameters
	mov		esi,	(WI_CALL_PARAMS PTR [esi]).WG_PARAM
	mov		ecx,	(WG_CALL_PARAMS PTR [esi]).PARAM_SIZE
	mov		ebx,	ecx
	test	ecx,	ecx
	jz		call_kernel
	
	shr		ecx,	2			; Parameter alway comes in DWORD
	mov		esi,	(WG_CALL_PARAMS PTR [CALL_PARAM]).PARAMS
@@:
;	mov		eax,	(WG_CALL_PARAMS PTR [esi]).PARAMS[4*ecx-4]
	mov		eax,	DWORD PTR[esi+4*ecx-4]
	push	eax
	loop	@B
	
call_kernel:
	call	(WG_CALL_PARAMS PTR [esi]).KRNL_PTR
	inc		ebx							
	inc		ebx
	add		esp,	ebx
	pop		ebx
	pop		esi
	pop		ecx
; return value already in eax
	ret
_KernelObsoleteExecute ENDP

_KernelExecute PROC STDCALL CALL_PARAM: PTR
	push	ecx
	push	esi
	push	edi
	push	ebx
	mov		esi,	CALL_PARAM
	; Push Work-Group execution parameters
	mov		esi,	(WI_CALL_PARAMS PTR [esi]).WG_PARAM
	mov		edi,	esi
	mov		ecx,	(WG_CALL_PARAMS PTR [esi]).PARAM_SIZE
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
