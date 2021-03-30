; RUN: opt -VPlanDriver -S < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -hir-cg -S < %s 2>&1 | FileCheck %s
;
; VPlan vectorizers are currently not setup to deal with reductions/inductions
; on vector types. For now, bail out for such cases. Test checks that it does
; not crash and that the loop is not vectorized.
; TODO - add needed support in entities lowering and CG.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@larr = dso_local local_unnamed_addr global [400 x i64] zeroinitializer, align 16

; CHECK-LABEL: @foo(
; CHECK-NOT: <16 x i64>
define <4 x i64> @foo(<4 x i64>* nocapture readonly byval(<4 x i64>) align 32 %0) {
entry:
  %in = load <4 x i64>, <4 x i64>* %0, align 32
  br label %for.ph

for.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %in.addr.07 = phi <4 x i64> [ %in, %for.ph ], [ %add, %for.body ]
  %l1.06 = phi i64 [ 0, %for.ph ], [ %inc, %for.body ]
  %mul = shl nsw i64 %l1.06, 2
  %arrayidx = getelementptr inbounds [400 x i64], [400 x i64]* @larr, i64 0, i64 %mul
  %1 = bitcast i64* %arrayidx to <4 x i64>*
  %2 = load <4 x i64>, <4 x i64>* %1, align 32
  %add = add <4 x i64> %2, %in.addr.07
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret <4 x i64> %add
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)