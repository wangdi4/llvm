; RUN: opt -add-implicit-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; AddImplicitArgs pass test
; This tests checks that
; 1. new function has same attributes like old function
; 2. new function arguments has same attributes like old function arguments
; 3. new function linkage has attributes like old function linkage
; 4. old function became a declaration with no linkage

define internal void @foo(<2 x i8>* byval align 8 %x) nounwind {
entry:
  ret void
}

; CHECK: declare void @__foo_original(<2 x i8>* byval align 8) nounwind
; CHECK: define internal void @foo(<2 x i8>* byval align 8 %x,
; CHECK:  nounwind {
