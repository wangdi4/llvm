; Check functionality of -vplan-force-loop-uf option.

; RUN: opt < %s -passes='vplan-vec,intel-ir-optreport-emitter' -S -vplan-force-vf=4 -vplan-force-loop-uf=2:4 -vplan-force-loop-uf=3:8 -intel-opt-report=medium  2>&1 | FileCheck %s
; RUN: opt < %s -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>,hir-cg,simplifycfg,intel-ir-optreport-emitter' -disable-output -vplan-force-vf=4 -vplan-force-loop-uf=2:4 -vplan-force-loop-uf=3:8 -intel-opt-report=medium 2>&1 | FileCheck %s

; CHECK-LABEL: Global optimization report for : foo

; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-NEXT:     remark #15399: vectorization support: unroll factor 4
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-NEXT:     remark #15399: vectorization support: unroll factor 8
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; NOTE that loops are processed in reverse order for LLVM IR input, so the
; number only makes sense in sync with -debug-only=LoopVectorizationPlanner.

define hidden void @foo(i1 %loop1.top.test, ptr %base, ptr %ptr) {
loop1.preheader:
  %tok1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop1.body

loop1.body:
  %loop1.iv = phi i64 [ 0, %loop1.preheader ], [ %loop1.iv.next, %loop1.body ]
  %gep1 = getelementptr float, ptr %ptr, i64 %loop1.iv
  %ld1 = load float, ptr %gep1
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
  %gep2 = getelementptr float, ptr %ptr, i64 %loop2.iv
  %ld2 = load float, ptr %gep2
  %loop2.iv.next = add nsw i64 %loop2.iv, 1
  %loop2.exitcond = icmp eq i64 %loop2.iv.next, 80
  br i1 %loop2.exitcond, label %loop2.exit, label %loop2.body

loop2.exit:
  call void @llvm.directive.region.exit(token %tok2) [ "DIR.OMP.END.SIMD"() ]
  br label %loop3.preheader

loop3.preheader:
  %tok3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop3.body

loop3.body:
  %loop3.iv = phi i64 [ 0, %loop3.preheader ], [ %loop3.iv.next, %loop3.body ]
  %gep3 = getelementptr float, ptr %ptr, i64 %loop3.iv
  %ld3 = load float, ptr %gep3
  %loop3.iv.next = add nuw nsw i64 %loop3.iv, 1
  %loop3.exitcond = icmp eq i64 %loop3.iv.next, 80
  br i1 %loop3.exitcond, label %loop3.exit, label %loop3.body

loop3.exit:
  call void @llvm.directive.region.exit(token %tok3) [ "DIR.OMP.END.SIMD"() ]
  ret void

}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
