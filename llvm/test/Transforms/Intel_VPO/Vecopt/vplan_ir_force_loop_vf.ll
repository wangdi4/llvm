; RUN: opt < %s -vplan-vec -S -vplan-force-vf=4 -vplan-force-loop-vf=2:2 -vplan-force-loop-vf=3:8  -debug-only=LoopVectorizationPlanner  2>&1 | FileCheck %s --check-prefixes=CHECK,LLVM
; RUN: opt < %s -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -vplan-force-vf=4 -vplan-force-loop-vf=2:2 -vplan-force-loop-vf=3:8  -debug-only=LoopVectorizationPlanner -print-after=hir-vplan-vec  2>&1 | FileCheck %s --check-prefixes=CHECK,HIR


; CHECK:      LVP: ForcedVF: 4
; CHECK-NEXT: LVP: Safelen: 4294967295
; CHECK-NEXT: LVP: MinVF: 4 MaxVF: 4
; CHECK:      Selecting VF for VPlan foo:{{(loop1.body|HIR)}}.#1
; CHECK-NEXT: There is only VPlan with VF=4, selecting it.

; CHECK:      LVP: ForcedVF: 2
; CHECK-NEXT: LVP: Safelen: 4294967295
; CHECK-NEXT: LVP: MinVF: 2 MaxVF: 2
; CHECK:      Selecting VF for VPlan foo:{{(loop2.body|HIR)}}.#2
; CHECK-NEXT: There is only VPlan with VF=2, selecting it.

; CHECK:      LVP: ForcedVF: 8
; CHECK-NEXT: LVP: Safelen: 4294967295
; CHECK-NEXT: LVP: MinVF: 8 MaxVF: 8
; CHECK:      Selecting VF for VPlan foo:{{(loop3.body|HIR)}}.#3
; CHECK-NEXT: There is only VPlan with VF=8, selecting it.

; HIR:           BEGIN REGION { modified }
; HIR-NEXT:            + DO i1 = 0, 79, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:            |   {{%.*}} = (<4 x float>*)(%ptr)[i1];
; HIR-NEXT:            + END LOOP
; HIR:           END REGION
; HIR-EMPTY:
; HIR-NEXT:      BEGIN REGION { modified }
; HIR-NEXT:            + DO i1 = 0, 79, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:            |   {{%.*}} = (<2 x float>*)(%ptr)[i1];
; HIR-NEXT:            + END LOOP
; HIR:           END REGION
; HIR-EMPTY:
; HIR-NEXT:      BEGIN REGION { modified }
; HIR-NEXT:            + DO i1 = 0, 79, 8   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:            |   {{%.*}} = (<8 x float>*)(%ptr)[i1];
; HIR-NEXT:            + END LOOP
; HIR:           ret ;
; HIR-NEXT:      END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; NOTE that loops are processed in reverse order for LLVM IR input, so the
; number only makes sense in sync with -debug-only=LoopVectorizationPlanner.

define hidden void @foo(i1 %loop1.top.test, i32* %base, float *%ptr) {
; LLVM-LABEL: @foo
loop1.preheader:
  %tok1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %loop1.body

loop1.body:
  %loop1.iv = phi i64 [ 0, %loop1.preheader ], [ %loop1.iv.next, %loop1.body ]
  %gep1 = getelementptr float, float * %ptr, i64 %loop1.iv
  %ld1 = load float, float *%gep1
; LLVM-DAG: %wide.load{{.*}} = load <8 x float>
; LLVM-DAG: load float
  %loop1.iv.next = add nsw i64 %loop1.iv, 1
  %loop1.exitcond = icmp eq i64 %loop1.iv.next, 80
  br i1 %loop1.exitcond, label %loop1.exit, label %loop1.body

loop1.exit:
  call void @llvm.directive.region.exit(token %tok1) [ "DIR.OMP.END.SIMD"() ]
  br label %loop2.preheader

loop2.preheader:
  %tok2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop2.body

loop2.body:
  %loop2.iv = phi i64 [ 0, %loop2.preheader ], [ %loop2.iv.next, %loop2.body ]
  %gep2 = getelementptr float, float * %ptr, i64 %loop2.iv
  %ld2 = load float, float *%gep2
; LLVM-DAG: %wide.load{{.*}} = load <2 x float>
; LLVM-DAG: load float
  %loop2.iv.next = add nsw i64 %loop2.iv, 1
  %loop2.exitcond = icmp eq i64 %loop2.iv.next, 80
  br i1 %loop2.exitcond, label %loop2.exit, label %loop2.body

loop2.exit:
  call void @llvm.directive.region.exit(token %tok2) [ "DIR.OMP.END.SIMD"() ]
  br label %loop3.preheader

loop3.preheader:
  %tok3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %loop3.body

loop3.body:
  %loop3.iv = phi i64 [ 0, %loop3.preheader ], [ %loop3.iv.next, %loop3.body ]
  %gep3 = getelementptr float, float * %ptr, i64 %loop3.iv
  %ld3 = load float, float *%gep3
; LLVM-DAG: %wide.load{{.*}} = load <4 x float>
; LLVM-DAG: load float
  %loop3.iv.next = add nuw nsw i64 %loop3.iv, 1
  %loop3.exitcond = icmp eq i64 %loop3.iv.next, 80
  br i1 %loop3.exitcond, label %loop3.exit, label %loop3.body

loop3.exit:
  call void @llvm.directive.region.exit(token %tok3) [ "DIR.OMP.END.SIMD"() ]
  ret void

}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
