;; Test this for IR path only at this point, as HIR framework bails out both
;; due to unrecognized pragmas and a present atomic instruction.
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
DIR.OMP.SIMD.130:
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  store float 0.000000e+00, float* %x.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.130
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(float* %x.red, float 0.000000e+00, i32 1, i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i8* null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i8* null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.132

DIR.OMP.SIMD.132:                                 ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32* %i.linear.iv to i8*
  br label %DIR.VPO.END.GUARD.MEM.MOTION.5

DIR.VPO.END.GUARD.MEM.MOTION.5:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.533, %DIR.OMP.SIMD.132
  %indvars.iv = phi i64 [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.533 ], [ 0, %DIR.OMP.SIMD.132 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.3

DIR.VPO.GUARD.MEM.MOTION.3:                       ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.5
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(float* %x.red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.3
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %3 = load float, float* %arrayidx, align 4
  %4 = load float, float* %x.red, align 4
  %add1 = fadd fast float %4, %3
  store float %add1, float* %x.red, align 4
  br label %DIR.OMP.SCAN.5

DIR.OMP.SCAN.5:                                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE:TYPED"(float* %x.red, float 0.000000e+00, i32 1, i64 1) ]
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.OMP.SCAN.5
  fence acq_rel
  br label %DIR.OMP.END.SCAN.7

DIR.OMP.END.SCAN.7:                               ; preds = %DIR.OMP.SCAN.3
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.4

DIR.OMP.END.SCAN.4:                               ; preds = %DIR.OMP.END.SCAN.7
  %6 = load float, float* %x.red, align 4
  %7 = load i32, i32* %i.linear.iv, align 4
  %idxprom2 = sext i32 %7 to i64
  %arrayidx3 = getelementptr inbounds float, float* %B, i64 %idxprom2
  store float %6, float* %arrayidx3, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.9

DIR.VPO.END.GUARD.MEM.MOTION.9:                   ; preds = %DIR.OMP.END.SCAN.4
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.533

DIR.VPO.END.GUARD.MEM.MOTION.533:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.9
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.6, label %DIR.VPO.END.GUARD.MEM.MOTION.5

DIR.OMP.END.SIMD.6:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.533
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.634

DIR.OMP.END.SIMD.634:                             ; preds = %DIR.OMP.END.SIMD.6
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
