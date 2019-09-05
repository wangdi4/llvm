; REQUIRES: asserts

; This test checks that the memory was freed when running
; lld without early exit.

; RUN: llvm-as -o %t.bc %s
; RUN: lld-link /intel-debug=mem /out:%t.exe /entry:main %t.bc \
; RUN:     /subsystem:console 2>&1 | FileCheck %s

; CHECK: lld-link: warning: Cleaning up LLD memory
; CHECK: lld-link: warning: Cleaning up LLVM memory

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  ret i32 %call1
}
