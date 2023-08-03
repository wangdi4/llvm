; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-interchange" -aa-pipeline="basic-aa" -print-after=hir-loop-interchange -debug-only=hir-loop-interchange -disable-output  < %s 2>&1 | FileCheck %s

; Check that we bail out of interchange for outer SIMD loops.

; CHECK: Skipping SIMD loop

; CHECK-NOT: modified

; CHECK: %entry.region = @llvm.directive.region.entry(); [ DIR.OMP.SIMD() ]

; CHECK: + DO i1 = 0, 1000, 1   <DO_LOOP> <simd>
; CHECK: |   + DO i2 = 0, 95, 1   <DO_LOOP>
; CHECK: |   |   %0 = (@A)[0][i2 + 1][i1 + 2];
; CHECK: |   |   %add5 = %0  +  1.000000e+00;
; CHECK: |   |   (@A)[0][i2 + 1][i1 + 1] = %add5;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%entry.region); [ DIR.OMP.END.SIMD() ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x float]] zeroinitializer, align 16
@B = common global [1000 x [1000 x float]] zeroinitializer, align 16

define void @sub3(i64 %n) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.cond.loopexit, %entry
  %i.019 = phi i64 [ 1, %entry ], [ %add, %for.cond.loopexit ]
  %add = add nuw nsw i64 %i.019, 1
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %j.018 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc, %for.body.3 ]
  %arrayidx4 = getelementptr inbounds [1000 x [1000 x float]], ptr @A, i64 0, i64 %j.018, i64 %add
  %0 = load float, ptr %arrayidx4, align 4
  %add5 = fadd float %0, 1.000000e+00
  %arrayidx7 = getelementptr inbounds [1000 x [1000 x float]], ptr @A, i64 0, i64 %j.018, i64 %i.019
  store float %add5, ptr %arrayidx7, align 4
  %inc = add nuw nsw i64 %j.018, 1
  %exitcond = icmp eq i64 %inc, 97
  br i1 %exitcond, label %for.cond.loopexit, label %for.body.3

for.cond.loopexit:                                ; preds = %for.body.3
  %exitcond20 = icmp eq i64 %add, 1002
  br i1 %exitcond20, label %for.end.10, label %for.cond.1.preheader

for.end.10:                                       ; preds = %for.cond.loopexit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
