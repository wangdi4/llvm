;
; LIT test to check that we scale cost of main vector loop when unrolling the same
;
; RUN: opt -disable-output -passes=vplan-vec -debug-only=LoopVectorizationPlanner -mattr=+avx2 -vplan-enable-masked-vectorized-remainder=0 -vplan-enable-non-masked-vectorized-remainder=0 -vplan-force-uf=1 < %s 2>&1 | FileCheck %s --check-prefix=IR-UF1-CHECK
; RUN: opt -disable-output -passes=vplan-vec -debug-only=LoopVectorizationPlanner -mattr=+avx2 -vplan-enable-masked-vectorized-remainder=0 -vplan-enable-non-masked-vectorized-remainder=0 -vplan-force-uf=3 < %s 2>&1 | FileCheck %s --check-prefix=IR-UF3-CHECK
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec' -debug-only=LoopVectorizationPlanner -mattr=+avx2 -vplan-enable-masked-vectorized-remainder=0 -vplan-enable-non-masked-vectorized-remainder=0 -vplan-force-uf=1 < %s 2>&1 | FileCheck %s --check-prefix=HIR-UF1-CHECK
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vplan-vec' -debug-only=LoopVectorizationPlanner -mattr=+avx2 -vplan-enable-masked-vectorized-remainder=0 -vplan-enable-non-masked-vectorized-remainder=0 -vplan-force-uf=2 < %s 2>&1 | FileCheck %s --check-prefix=HIR-UF2-CHECK


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; IR-UF1-CHECK:  Cost of Scalar VPlan: 4096
; IR-UF1-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 512 x 7.125 + 0 + 0 = 3648
; IR-UF1-CHECK:   VectorCostWithoutPeel = 512 x 7.125 + 0 = 3648
; IR-UF1-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 256 x 7.375 + 0 + 0 = 1888
; IR-UF1-CHECK:   VectorCostWithoutPeel = 256 x 7.375 + 0 = 1888
; IR-UF3-CHECK:  Cost of Scalar VPlan: 4096
; IR-UF3-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 170 x 21.375 + 0 + 16 = 3649.75
; IR-UF3-CHECK:   VectorCostWithoutPeel = 170 x 21.375 + 0 = 3649.75
; IR-UF3-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 85 x 22.125 + 0 + 16 = 1896.625
; IR-UF3-CHECK:   VectorCostWithoutPeel = 85 x 22.125 + 0 = 1896.625
; HIR-UF1-CHECK:  Cost of Scalar VPlan: 4096
; HIR-UF1-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 512 x 7.125 + 0 + 0 = 3648
; HIR-UF1-CHECK:   VectorCostWithoutPeel = 512 x 7.125 + 0 = 3648
; HIR-UF1-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 256 x 7.375 + 0 + 0 = 1888
; HIR-UF1-CHECK:   VectorCostWithoutPeel = 256 x 7.375 + 0 = 1888
; HIR-UF1-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 128 x 14.75 + 0 + 0 = 1888
; HIR-UF1-CHECK:   VectorCostWithoutPeel = 128 x 14.75 + 0 = 1888
; HIR-UF1-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 64 x 29.5 + 0 + 0 = 1888
; HIR-UF1-CHECK:   VectorCostWithoutPeel = 64 x 29.5 + 0 = 1888
; HIR-UF1-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 32 x 59 + 0 + 0 = 1888
; HIR-UF1-CHECK:   VectorCostWithoutPeel = 32 x 59 + 0 = 1888
; HIR-UF2-CHECK:  Cost of Scalar VPlan: 4096
; HIR-UF2-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 256 x 14.25 + 0 + 0 = 3648
; HIR-UF2-CHECK:   VectorCostWithoutPeel = 256 x 14.25 + 0 = 3648
; HIR-UF2-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 128 x 14.75 + 0 + 0 = 1888
; HIR-UF2-CHECK:   VectorCostWithoutPeel = 128 x 14.75 + 0 = 1888
; HIR-UF2-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 64 x 29.5 + 0 + 0 = 1888
; HIR-UF2-CHECK:   VectorCostWithoutPeel = 64 x 29.5 + 0 = 1888
; HIR-UF2-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 32 x 59 + 0 + 0 = 1888
; HIR-UF2-CHECK:   VectorCostWithoutPeel = 32 x 59 + 0 = 1888
; HIR-UF2-CHECK:  Scalar Cost = 1024 x 4 = 4096 > VectorCost = 0 + 16 x 118 + 0 + 0 = 1888
; HIR-UF2-CHECK:   VectorCostWithoutPeel = 16 x 118 + 0 = 1888
define void @baz(ptr %lparr) {
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %index.04 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nuw nsw i64 %index.04, 1111
  %arrayidx = getelementptr inbounds i64, ptr %lparr, i64 %index.04
  store i64 %add, ptr %arrayidx, align 8
  %inc = add nuw nsw i64 %index.04, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
