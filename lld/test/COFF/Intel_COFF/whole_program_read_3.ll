; REQUIRES: assert
; This test checks that whole program wasn't achieved because the
; definition for @sub is missing in IR and libraries.
; NOTE: lld will throw a exit error since the definition of sub
; is not present during link time.

; RUN: llvm-as -o %T/wp3.bc %s
; RUN: lld-link /out:%T/wp3.exe /entry:main %T/wp3.bc /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis /force:unresolved \
; RUN:     2>&1 | FileCheck %s

; CHECK:   Main definition seen
; CHECK:   LIBFUNCS NOT FOUND: 1
; CHECK:       sub
; CHECK:   VISIBLE OUTSIDE LTO: 1
; CHECK:       sub
; CHECK:   WHOLE PROGRAM NOT DETECTED
; CHECK:   WHOLE PROGRAM SAFE is *NOT* determined:
; CHECK:       whole program not seen;
; CHECK:       whole program not read;

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

declare i32 @sub(i32 %a)

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
