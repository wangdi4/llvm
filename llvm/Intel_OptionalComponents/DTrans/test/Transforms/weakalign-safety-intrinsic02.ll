; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is inhibited in the presence of
; mmx intrinsics.

; CHECK: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = global double zeroinitializer, align 8
@b = global double zeroinitializer, align 8


define internal double @test01(double %a, double %b) {
  %a.mmx = bitcast double %a to x86_mmx
  %b.mmx = bitcast double %b to x86_mmx
  %res.mmx = tail call x86_mmx @llvm.x86.mmx.palignr.b(x86_mmx %a.mmx, x86_mmx %b.mmx, i8 4)
  %res = bitcast x86_mmx %res.mmx to double
  ret double %res
}

define i32 @main() {
  %a.val = load double, double* @a
  %b.val = load double, double* @b
  %c = call double @test01(double %a.val, double %b.val)
  ret i32 0
}

declare x86_mmx @llvm.x86.mmx.palignr.b(x86_mmx, x86_mmx, i8)
