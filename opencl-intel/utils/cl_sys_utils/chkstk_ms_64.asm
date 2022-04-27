; This file is modified from lib/builtins/x86_64/chkstk.S in LLVM compiler-rt project.

; Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
; See https://llvm.org/LICENSE.txt for license information.
; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

; _chkstk routine
; This routine is windows specific
; http://msdn.microsoft.com/en-us/library/ms648426.aspx

; Notes from r227519
; MSVC x64s __chkstk and cygmings ___chkstk_ms do not adjust rsp
; themselves. It also does not clobber %rax so we can reuse it when
; adjusting rsp.

; Translated from AT&T assembly to intel assembly syntax.

PUBLIC  ___chkstk_ms

_TEXT   SEGMENT

___chkstk_ms PROC
    push    rcx
    push    rax
    cmp     rax, 1000h
    lea     rcx, [rsp + 24]
    jb      ___chkstk_ms_1
___chkstk_ms_2:
    sub     rcx, 1000h
    test    [rcx], rcx
    sub     rax, 1000h
    cmp     rax, 1000h
    ja      ___chkstk_ms_2
___chkstk_ms_1:
    sub     rcx, rax
    test    [rcx], rcx
    pop     rax
    pop     rcx
    ret
___chkstk_ms ENDP

_TEXT ENDS
END
