; This file is modified from lib/builtins/i386/chkstk2.S in LLVM compiler-rt project.

; Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
; See https://llvm.org/LICENSE.txt for license information.
; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

; _chkstk routine
; This routine is windows specific
; http://msdn.microsoft.com/en-us/library/ms648426.aspx

; Translated from AT&T assembly to intel assembly syntax.

PUBLIC ____chkstk

_TEXT   SEGMENT

____chkstk PROC
    push    ecx
    cmp     eax, 1000h
    lea     ecx, [esp + 8]  ; esp before calling this routine -> ecx
    jb      ____chkstk_1
____chkstk_2:
    sub     ecx, 1000h
    test    [ecx], ecx
    sub     eax, 1000h
    cmp     eax, 1000h
    ja      ____chkstk_2
____chkstk_1:
    sub     ecx, eax
    test    [ecx], ecx

    lea     eax, [esp + 4]  ; load pointer to the return address into eax
    mov     esp, ecx        ; install the new top of stack pointer into esp
    mov     ecx, [eax - 4]  ; restore ecx
    push    [eax]           ; push return address onto the stack
    sub     eax, esp        ; restore the original value in eax
    ret
____chkstk ENDP

_TEXT ENDS
END
