
.686P
.MODEL FLAT, C
.CODE

;****************************************************************************
;*
;* Register usage
;*
;* Caller-saved and scratch:
;*	eax
;*	edx
;*	ecx
;*
;* Callee-saved
;*	ebp
;*	ebx
;*	esi
;*	edi
;*	esp
;*
;****************************************************************************

;------------------------------------------------------------------------------
;
; Execute assembler 'pause' instruction
;
;void ASM_FUNCTION hw_pause( void );
;------------------------------------------------------------------------------
hw_pause PROC NEAR STDCALL
    pause
    ret
hw_pause ENDP

END
