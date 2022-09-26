; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is NOT inhibited in the presence of
; the llvm.is.constant intrinsic.

; CHECK-NOT: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test01(i64 %in) {
  %out = sdiv i64 %in, 2
  %tmp = tail call i1 @llvm.is.constant.i64(i64 %out)
  ret void
}


define i32 @main() {
  call void @test01(i64 2)
  ret i32 0
}

declare i1 @llvm.is.constant.i64(i64)
