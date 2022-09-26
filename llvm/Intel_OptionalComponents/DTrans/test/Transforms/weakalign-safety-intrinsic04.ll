; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is NOT inhibited in the presence of
; experimental intrinsics.

; CHECK-NOT: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @test01(float %x, float %y) {
  %val = call float @llvm.experimental.constrained.fadd.f32(float %x, float %y, metadata !"round.tonearest", metadata !"fpexcept.strict") #0
  ret float %val
}

define void @test02(i32* %p) {
  tail call void @llvm.experimental.noalias.scope.decl(metadata !0)
  %l = load i32, i32* %p
  ret void
}

; We need main() because that is where the mallopt call will be inserted.
define i32 @main() {
  ret i32 0
}
; CHECK-LABEL: define i32 @main()
; CHECK: %mo = call i32 @mallopt(i32 3225, i32 0)

declare float @llvm.experimental.constrained.fadd.f32(float, float, metadata, metadata)
declare void @llvm.experimental.noalias.scope.decl(metadata)

!0 = !{ !1 }
!1 = distinct !{ !1, !2, !"test02: var" }
!2 = distinct !{ !2, !"test02" }