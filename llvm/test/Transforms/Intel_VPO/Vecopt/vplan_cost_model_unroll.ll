; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt < %s -disable-output -passes=vplan-vec -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 -vplan-enable-partial-sums=true -vplan-force-uf=4 -vplan-cm-unroll=true -vplan-cm-unroll-partial-sums-only=true | FileCheck %s 

; RUN: opt < %s -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' -vplan-force-vf=4 -vplan-cost-model-print-analysis-for-vf=4 -vplan-enable-partial-sums=true -vplan-force-uf=4 -vplan-cm-unroll=true -vplan-cm-unroll-partial-sums-only=true | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that unrolling heuristic decreases cost for partial-sum reduction and induction when unrolling.

define float @foo(ptr %lp, float %init) {
; CHECK-LABEL: Cost Model for VPlan foo:
; CHECK: Cost decrease due to Unroll heuristic is [[CR:[1-9]]]
entry:
  %sum.red = alloca float, align 4
  store float 0.000000e+00, ptr %sum.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %sum.red, float zeroinitializer, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:
  %sum.red.promoted = load float, ptr %sum.red, align 4
  br label %for.body

for.body:
  %1 = phi float [ %sum.red.promoted, %DIR.OMP.SIMD.2 ], [ %add1, %for.body ]
  %iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add2, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %lp, i64 %iv
  %2 = load float, ptr %arrayidx, align 4
  %add1 = fadd fast float %1, %2
  %add2 = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %add2, 1024
  br i1 %exitcond.not, label %for.exit, label %for.body

for.exit:
  %add1.lcssa = phi float [ %add1, %for.body ]
  store float %add1.lcssa, ptr %sum.red, align 4
  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %ret

ret:
  %3 = fadd fast float %add1.lcssa, %init
  ret float %3
}

; Check that unrolling heuristic has no effect without partial-sum reduction.

define float @foo_no_reduc(ptr %lp, float %init) {
; CHECK-LABEL: Cost Model for VPlan foo_no_reduc:
; CHECK-NOT: Cost decrease due to Unroll heuristic
entry:
  %sum.red = alloca float, align 4
  store float 0.000000e+00, ptr %sum.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %sum.red, float zeroinitializer, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:
  %sum.red.promoted = load float, ptr %sum.red, align 4
  br label %for.body

for.body:
  %1 = phi float [ %sum.red.promoted, %DIR.OMP.SIMD.2 ], [ %add1, %for.body ]
  %iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %add2, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %lp, i64 %iv
  %2 = load float, ptr %arrayidx, align 4
  %add1 = fadd float %1, %2
  %add2 = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %add2, 1024
  br i1 %exitcond.not, label %for.exit, label %for.body

for.exit:
  %add1.lcssa = phi float [ %add1, %for.body ]
  store float %add1.lcssa, ptr %sum.red, align 4
  br label %DIR.OMP.SIMD.3

DIR.OMP.SIMD.3:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %ret

ret:
  %3 = fadd float %add1.lcssa, %init
  ret float %3
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; end INTEL_FEATURE_SW_ADVANCED
