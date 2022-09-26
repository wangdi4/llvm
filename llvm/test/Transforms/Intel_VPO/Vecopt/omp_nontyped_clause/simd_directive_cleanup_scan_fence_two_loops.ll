; RUN: opt -enable-new-pm=0 -vplan-vec -vplan-force-vf=4 -vplan-force-inscan-reduction-vectorization=true -VPODirectiveCleanup -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec,vpo-directive-cleanup" -vplan-force-vf=4 -vplan-force-inscan-reduction-vectorization=true -S < %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;; Check correctness of compiler-generated block removing.
;; Fences from the scalar remainders must be removed, while a fence from
;; the user code must survive.

;; float x = 1.0f;
;; #pragma omp simd reduction(inscan, + : x)
;; for (int i=0; i<N; i++) {
;;   x += A[i];
;; #pragma omp scan inclusive(x)
;;   B[i] = x;
;; }
;; fence(); // user fence is supposed to stay.
;; #pragma omp simd reduction(inscan, + : x)
;; for (int i=0; i<N; i++) {
;;   x += C[i];
;; #pragma omp scan inclusive(x)
;;     D[i] = x;
;; }

; CHECK-LABEL:   DIR.OMP.END.SCAN{{.*}}:
; CHECK-NOT:       fence acq_rel
; CHECK:           br

; CHECK-LABEL:   DIR.OMP.END.SIMD{{.*}}:
; CHECK:           fence acq_rel
; CHECK:           br

; CHECK-LABEL:  DIR.OMP.END.SCAN{{.*}}:
; CHECK-NOT:       fence acq_rel
; CHECK:           br

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @_Z3fooPfS_S_S_i(float* %A, float* %B, float* %C, float* %D, i32 %N) {
entry:
  %x.red78 = alloca float, align 4
  %i22.linear.iv = alloca i32, align 4
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end35

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store float 1.000000e+00, float* %x.red, align 4
  br label %DIR.OMP.SIMD.1108

DIR.OMP.SIMD.1108:                                ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN"(float* %x.red, i64 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1108
  %1 = bitcast i32* %i.linear.iv to i8*
  %wide.trip.count = zext i32 %N to i64
  br label %DIR.OMP.END.SCAN.3104

DIR.OMP.END.SCAN.3104:                            ; preds = %DIR.OMP.SIMD.2, %DIR.OMP.END.SCAN.6
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %DIR.OMP.END.SCAN.6 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %3 = load float, float* %arrayidx, align 4
  %4 = load float, float* %x.red, align 4
  %add5 = fadd fast float %4, %3
  store float %add5, float* %x.red, align 4
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.OMP.END.SCAN.3104
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE"(float* %x.red, i64 1) ]
  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.OMP.SCAN.3
  fence acq_rel
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.4
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.END.SCAN.5
  %6 = load float, float* %x.red, align 4
  %7 = load i32, i32* %i.linear.iv, align 4
  %idxprom6 = sext i32 %7 to i64
  %arrayidx7 = getelementptr inbounds float, float* %B, i64 %idxprom6
  store float %6, float* %arrayidx7, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %DIR.OMP.SIMD.5, label %DIR.OMP.END.SCAN.3104

DIR.OMP.SIMD.5:                                   ; preds = %DIR.OMP.END.SCAN.6
  %.lcssa110 = phi float [ %6, %DIR.OMP.END.SCAN.6 ]
  br label %DIR.OMP.END.SIMD.7

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.SIMD.5
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.8

DIR.OMP.END.SIMD.8:                               ; preds = %DIR.OMP.END.SIMD.7
  store float %.lcssa110, float* %x.red78, align 4
  br label %fence.block

fence.block:                                      ; preds = %DIR.OMP.END.SIMD.8
  fence acq_rel
  br label %DIR.OMP.SIMD.9

DIR.OMP.SIMD.9:                                   ; preds = %fence.block
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN"(float* %x.red78, i64 1), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i22.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.10

DIR.OMP.SIMD.10:                                  ; preds = %DIR.OMP.SIMD.9
  %9 = bitcast i32* %i22.linear.iv to i8*
  br label %DIR.OMP.END.SCAN.7

DIR.OMP.END.SCAN.7:                               ; preds = %DIR.OMP.SIMD.10, %DIR.OMP.END.SCAN.14
  %indvars.iv99 = phi i64 [ 0, %DIR.OMP.SIMD.10 ], [ %indvars.iv.next100, %DIR.OMP.END.SCAN.14 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %9)
  %10 = trunc i64 %indvars.iv99 to i32
  store i32 %10, i32* %i22.linear.iv, align 4
  %arrayidx26 = getelementptr inbounds float, float* %C, i64 %indvars.iv99
  %11 = load float, float* %arrayidx26, align 4
  %12 = load float, float* %x.red78, align 4
  %add27 = fadd fast float %12, %11
  store float %add27, float* %x.red78, align 4
  br label %DIR.OMP.SCAN.11

DIR.OMP.SCAN.11:                                  ; preds = %DIR.OMP.END.SCAN.7
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE"(float* %x.red78, i64 1) ]
  br label %DIR.OMP.SCAN.12

DIR.OMP.SCAN.12:                                  ; preds = %DIR.OMP.SCAN.11
  fence acq_rel
  br label %DIR.OMP.END.SCAN.13

DIR.OMP.END.SCAN.13:                              ; preds = %DIR.OMP.SCAN.12
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.14

DIR.OMP.END.SCAN.14:                              ; preds = %DIR.OMP.END.SCAN.13
  %14 = load float, float* %x.red78, align 4
  %15 = load i32, i32* %i22.linear.iv, align 4
  %idxprom28 = sext i32 %15 to i64
  %arrayidx29 = getelementptr inbounds float, float* %D, i64 %idxprom28
  store float %14, float* %arrayidx29, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %9)
  %indvars.iv.next100 = add nuw nsw i64 %indvars.iv99, 1
  %exitcond103.not = icmp eq i64 %indvars.iv.next100, %wide.trip.count
  br i1 %exitcond103.not, label %DIR.OMP.END.SIMD.9, label %DIR.OMP.END.SCAN.7

DIR.OMP.END.SIMD.9:                               ; preds = %DIR.OMP.END.SCAN.14
  %.lcssa = phi float [ %14, %DIR.OMP.END.SCAN.14 ]
  br label %DIR.OMP.END.SIMD.15

DIR.OMP.END.SIMD.15:                              ; preds = %DIR.OMP.END.SIMD.9
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end35

omp.precond.end35:                                ; preds = %DIR.OMP.END.SIMD.15, %entry
  %x.2 = phi float [ 1.000000e+00, %entry ], [ %.lcssa, %DIR.OMP.END.SIMD.15 ]
  ret float %x.2
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
