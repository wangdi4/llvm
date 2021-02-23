; This file is modified from lib/builtins/i386/chkstk.S in LLVM compiler-rt project.

; Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
; See https://llvm.org/LICENSE.txt for license information.
; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

; _chkstk routine
; This routine is windows specific
; http://msdn.microsoft.com/en-us/library/ms648426.aspx

; Translated from AT&T assembly to intel assembly syntax.

PUBLIC  ____chkstk_ms
PUBLIC  __alloca

_TEXT   SEGMENT

____chkstk_ms PROC
    push    ecx
    push    eax
    cmp     eax, 1000h
    lea     ecx, [esp + 12]
    jb      ____chkstk_ms_1
____chkstk_ms_2:
    sub     ecx, 1000h
    test    [ecx], ecx
    sub     eax, 1000h
    cmp     eax, 1000h
    ja      ____chkstk_ms_2
____chkstk_ms_1:
    sub     ecx, eax
    test    [ecx], ecx
    pop     eax
    pop     ecx
    ret
____chkstk_ms ENDP

__alloca PROC
    call    ____chkstk_ms
    ret
__alloca ENDP

_TEXT ENDS
END
