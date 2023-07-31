; Test for community vectorizer recognizing an induction with a
; loop-invariant variable step that is concealed by a uniform compare
; from the header to the latch, which our framework doesn't handle.

; RUN: opt -passes=vplan-vec -vplan-print-after-vpentity-instrs -S < %s 2>&1 \
; RUN:     | FileCheck %s

; CHECK-NOT: VPlan after insertion of VPEntities instructions:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@data_mp_atom_per_domain_ = external global i32, align 8

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define void @foo() #0 {
newFuncRoot:
  br label %DIR.OMP.SIMD.21764

DIR.OMP.SIMD.21764:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LIVEIN"(ptr @data_mp_atom_per_domain_) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %gepload710 = load i32, ptr @data_mp_atom_per_domain_, align 8
  br label %loop.337

loop.337:
  %i2.i32.0 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %1, %ifmerge.45 ]
  %t20.0 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %t20.1, %ifmerge.45 ]
  %1 = add nuw nsw i32 %i2.i32.0, 1
  %hir.cmp.45 = icmp sgt i32 %gepload710, 0
  br i1 %hir.cmp.45, label %then.45, label %ifmerge.45

then.45:
  %2 = add i32 %gepload710, %t20.0
  br label %ifmerge.45

ifmerge.45:
  %t20.1 = phi i32 [ %2, %then.45 ], [ %t20.0, %loop.337 ]
  %condloop.337.not = icmp eq i32 %1, 127
  br i1 %condloop.337.not, label %afterloop.337, label %loop.337

afterloop.337:
  br label %DIR.OMP.END.SIMD.41766

DIR.OMP.END.SIMD.41766:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

attributes #0 = { "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
