; This test checks that whole program assume won't internalize
; wmain.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/icx-lto%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=legacy-pass-manager \
; RUN:    -plugin-opt=-whole-program-assume \
; RUN:    -plugin-opt=-print-after-all  \
; RUN:    %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/icx-lto%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=new-pass-manager \
; RUN:    -plugin-opt=-whole-program-assume \
; RUN:    -plugin-opt=-print-after-all  \
; RUN:    %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; Check that main, add and sub aren't internal

; CHECK: define i32 @add
; CHECK: define i32 @wmain
; CHECK: declare i32 @sub

; Check that main and sub aren't internal, but add is internal
; CHECK: define internal i32 @add
; CHECK: define i32 @wmain
; CHECK: declare i32 @sub

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

declare i32 @sub(i32 %a)

define i32 @wmain(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
