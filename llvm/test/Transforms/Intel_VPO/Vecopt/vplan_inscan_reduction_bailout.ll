;; Test this for IR path only at this point, as HIR framework bails out both
;; due to unrecognized pragmas and a present atomic instruction.
; RUN: opt -disable-output -vplan-vec -debug-only=vpo-ir-loop-vectorize-legality -debug-only=vplan-vec  < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes="vplan-vec" -debug-only=vpo-ir-loop-vectorize-legality -debug-only=vplan-vec  < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

;; void foo(float *A, float *B) {
;;   float x = 0.0f;
;; #pragma omp simd reduction(inscan, + : x)
;;   for (int i=0; i<1024; i++) {
;;     x += A[i];
;; #pragma omp scan inclusive(x)
;;     B[i] = x;
;;   }
;; }

; CHECK: VPlan LLVM-IR Driver for Function: omp_scan
; CHECK: Inscan reduction is not supported.
; CHECK: VD: Not vectorizing: Cannot prove legality.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @omp_scan(float* %A, float* %B) {
entry:
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store float 0.000000e+00, float* %x.red, align 4
  br label %DIR.OMP.SIMD.138

DIR.OMP.SIMD.138:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN"(float* %x.red, i64 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.139

DIR.OMP.SIMD.139:                                 ; preds = %DIR.OMP.SIMD.138
  br label %DIR.OMP.END.SCAN.335

DIR.OMP.END.SCAN.335:                             ; preds = %DIR.OMP.END.SCAN.3, %DIR.OMP.SIMD.139
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.139 ], [ %indvars.iv.next, %DIR.OMP.END.SCAN.3 ]
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4
  %3 = load float, float* %x.red, align 4
  %add5 = fadd fast float %3, %2
  store float %add5, float* %x.red, align 4
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.OMP.END.SCAN.335
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE"(float* %x.red, i64 1) ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.OMP.SCAN.3
  fence acq_rel
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.2
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.3

DIR.OMP.END.SCAN.3:                               ; preds = %DIR.OMP.END.SCAN.5
  %5 = load float, float* %x.red, align 4
  %6 = load i32, i32* %i.linear.iv, align 4
  %idxprom6 = sext i32 %6 to i64
  %arrayidx7 = getelementptr inbounds float, float* %B, i64 %idxprom6
  store float %5, float* %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %DIR.OMP.END.SCAN.335, !llvm.loop !0

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.OMP.END.SIMD.7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.4, %entry
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.vectorize.enable", i1 true}
!2 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
