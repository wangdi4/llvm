; REQUIRES: asserts
; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is NOT inhibited in the presence of
; the llvm.usub.sat* intrinsics.

; CHECK-NOT: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test01(i32 %x, i32 %y) {
  %tmp = tail call i32 @llvm.usub.sat.i32(i32 %x, i32 %y)
  ret void
}

define void @test02(i64 %x, i64 %y) {
  %tmp = tail call i64 @llvm.usub.sat.i64(i64 %x, i64 %y)
  ret void
}

; CHECK-LABEL: define i32 @main()
; CHECK: %mo = call i32 @mallopt(i32 3225, i32 0)

define i32 @main() {
  call void @test01(i32 2, i32 1)
  call void @test02(i64 3, i64 2)
  ret i32 0
}

declare i32 @llvm.usub.sat.i32(i32, i32)
declare i64 @llvm.usub.sat.i64(i64, i64)

!intel.dtrans.types = !{}
