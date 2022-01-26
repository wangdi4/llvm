; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=8 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=8 -disable-output < %s 2>&1 | FileCheck %s

; Candidate HLLoop before VPlan (foo):
; <11>         + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>          |   %0 = (@a)[0][i1];
; <4>          |   %min.011 = @llvm.minnum.f32(%0,  %min.011); <Safe Reduction>
; <11>         + END LOOP

; CHECK:          BEGIN REGION { modified }
; CHECK-NEXT:               %red.init = %min.011;
; CHECK-NEXT:               %phi.temp = %red.init;
; CHECK:               + DO i1 = 0, 1023, 8   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:               |   %.vec = (<8 x float>*)(@a)[0][i1];
; CHECK-NEXT:               |   %llvm.minnum.v8f32 = @llvm.minnum.v8f32(%.vec,  %phi.temp);
; CHECK-NEXT:               |   %phi.temp = %llvm.minnum.v8f32;
; CHECK-NEXT:               + END LOOP
; CHECK:               %min.011 = @llvm.vector.reduce.fmin.v8f32(%llvm.minnum.v8f32);
; CHECK-NEXT:          END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16

define dso_local float @foo() local_unnamed_addr {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %.lcssa = phi float [ %1, %for.body ]
  ret float %.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %min.011 = phi float [ 2.550000e+02, %entry ], [ %1, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @a, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %1 = tail call fast float @llvm.minnum.f32(float %0, float %min.011)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %cmp.not, label %for.cond.cleanup, label %for.body
}

declare float @llvm.minnum.f32(float, float)
