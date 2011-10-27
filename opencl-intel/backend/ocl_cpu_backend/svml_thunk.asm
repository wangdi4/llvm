PUBLIC	SvmlThunk

; Thunk from Win64 stack frame ABI to that of SVML functions:
;  o Win64 allocates 32-byte long red-zone after parameter area
;  o SVML expects parameters on stack immediately before return address
; The Thunk saves its state (RBP and return address) in the save-state area 
; provided by the Thunk's caller, repositions RSP above red-zone and calls 
; SVML function by pointer which was stored by the Thunk's caller in the 
; save-state area.
; Upon getting control back from the SVML function Thunk restores its state 
; from the save-state area and returns the control to its caller.
; The only wrapper parameter is the state-save area pointer in 1st param.
;
;   The stack layout after Thunk invocation 
;   =======================================
;   <params to SVML - original>
;   <State-save area pointer>
;   <State-save area pointer - Copy>  ; FOR 16-byte alignment
;   <"Red Zone": 32 bytes>
;   <ret addr from thunk>
;   ------------------------ <RSP on entry to thunk>
;   
;   The stack layout after SVML function invocation 
;   =================================================  
;   <params to SVML - original>
;   <ret addr from SVML function>
;   ------------------------ <RSP on entry to SVML function>

THUNK_STATE_SAVE   struc
    svml_addr  DQ  ?
    _rbp       DQ  ?
    _ret       DQ  ?
THUNK_STATE_SAVE   ends

; We will reposition RSP upto SVML params, hence the offset is composed from
;      <ret addr from thunk: 8 bytes>
;      <"Red Zone": 32 bytes>
;      <State-save area pointer: 8 bytes>
SZ_RED_ZONE         EQU 32
STATE_SAFE_PTR_OFFS EQU 8 + SZ_RED_ZONE
RSP_OFFSET          EQU STATE_SAFE_PTR_OFFS + 16

_TEXT	SEGMENT

SvmlThunk   PROC
            ; save return address and RBP value in the state-save area  
            mov     r11, qword ptr STATE_SAFE_PTR_OFFS[rsp]
            mov     rax, qword ptr [rsp]
            mov     (THUNK_STATE_SAVE ptr [r11])._rbp, rbp
            mov     (THUNK_STATE_SAVE ptr [r11])._ret, rax
            ; save state-save area address in RBP
            mov     rbp, r11
            ; reposition RSP and call SVML function
            add     rsp, RSP_OFFSET
            call    (THUNK_STATE_SAVE ptr [r11]).svml_addr
            ; reposition RSP back and restore RBP
            sub     rsp, RSP_OFFSET-8 ; RSP is positioned immediately before "Red zone" - because no ret, but jmp
            mov     r11, rbp
            mov     rbp, (THUNK_STATE_SAVE ptr [r11])._rbp
            ; return to the Thunk caller            
            jmp     (THUNK_STATE_SAVE ptr [r11])._ret                   
SvmlThunk   ENDP
    
_TEXT	ENDS
END
