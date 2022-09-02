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

define float @_Z3fooPfS_S_S_i(ptr %A, ptr %B, ptr %C, ptr %D, i32 %N) {
entry:
  %x.red74 = alloca float, align 4
  %i22.linear.iv = alloca i32, align 4
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end35

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store float 1.000000e+00, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.194

DIR.OMP.SIMD.194:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.194
  %wide.trip.count = zext i32 %N to i64
  br label %DIR.OMP.END.SCAN.392

DIR.OMP.END.SCAN.392:                             ; preds = %DIR.OMP.SIMD.2, %DIR.OMP.END.SCAN.695
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %DIR.OMP.END.SCAN.695 ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv)
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %2 = load float, ptr %arrayidx, align 4
  %3 = load float, ptr %x.red, align 4
  %add5 = fadd fast float %3, %2
  store float %add5, ptr %x.red, align 4
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.OMP.END.SCAN.392
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE:TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1) ]
  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.OMP.SCAN.3
  fence acq_rel
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.4
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.695

DIR.OMP.END.SCAN.695:                             ; preds = %DIR.OMP.END.SCAN.5
  %5 = load float, ptr %x.red, align 4
  %6 = load i32, ptr %i.linear.iv, align 4
  %idxprom6 = sext i32 %6 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %B, i64 %idxprom6
  store float %5, ptr %arrayidx7, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %DIR.OMP.SCAN.11.lr.ph, label %DIR.OMP.END.SCAN.392

DIR.OMP.SCAN.11.lr.ph:                            ; preds = %DIR.OMP.END.SCAN.695
  %.lcssa98 = phi float [ %5, %DIR.OMP.END.SCAN.695 ]
  br label %DIR.OMP.END.SIMD.796

DIR.OMP.END.SIMD.796:                             ; preds = %DIR.OMP.SCAN.11.lr.ph
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.8

DIR.OMP.END.SIMD.8:                               ; preds = %DIR.OMP.END.SIMD.796
  store float %.lcssa98, ptr %x.red74, align 4
  br label %fence.block

fence.block:                                      ; preds = %DIR.OMP.END.SIMD.8
  fence acq_rel
  br label %DIR.OMP.SIMD.9

DIR.OMP.SIMD.9:                                   ; preds = %fence.block
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x.red74, float 0.000000e+00, i32 1, i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i22.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.SIMD.9, %DIR.OMP.END.SCAN.13
  %indvars.iv87 = phi i64 [ 0, %DIR.OMP.SIMD.9 ], [ %indvars.iv.next88, %DIR.OMP.END.SCAN.13 ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i22.linear.iv)
  %8 = trunc i64 %indvars.iv87 to i32
  store i32 %8, ptr %i22.linear.iv, align 4
  %arrayidx26 = getelementptr inbounds float, ptr %C, i64 %indvars.iv87
  %9 = load float, ptr %arrayidx26, align 4
  %10 = load float, ptr %x.red74, align 4
  %add27 = fadd fast float %10, %9
  store float %add27, ptr %x.red74, align 4
  br label %DIR.OMP.SCAN.10

DIR.OMP.SCAN.10:                                  ; preds = %DIR.OMP.END.SCAN.6
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE:TYPED"(ptr %x.red74, float 0.000000e+00, i32 1, i64 1) ]
  br label %DIR.OMP.SCAN.11

DIR.OMP.SCAN.11:                                  ; preds = %DIR.OMP.SCAN.10
  fence acq_rel
  br label %DIR.OMP.END.SCAN.12

DIR.OMP.END.SCAN.12:                              ; preds = %DIR.OMP.SCAN.11
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.13

DIR.OMP.END.SCAN.13:                              ; preds = %DIR.OMP.END.SCAN.12
  %12 = load float, ptr %x.red74, align 4
  %13 = load i32, ptr %i22.linear.iv, align 4
  %idxprom28 = sext i32 %13 to i64
  %arrayidx29 = getelementptr inbounds float, ptr %D, i64 %idxprom28
  store float %12, ptr %arrayidx29, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i22.linear.iv)
  %indvars.iv.next88 = add nuw nsw i64 %indvars.iv87, 1
  %exitcond91.not = icmp eq i64 %indvars.iv.next88, %wide.trip.count
  br i1 %exitcond91.not, label %DIR.OMP.END.SIMD.7, label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.END.SCAN.13
  %.lcssa = phi float [ %12, %DIR.OMP.END.SCAN.13 ]
  br label %DIR.OMP.END.SIMD.14

DIR.OMP.END.SIMD.14:                              ; preds = %DIR.OMP.END.SIMD.7
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end35

omp.precond.end35:                                ; preds = %DIR.OMP.END.SIMD.14, %entry
  %x.2 = phi float [ 1.000000e+00, %entry ], [ %.lcssa, %DIR.OMP.END.SIMD.14 ]
  ret float %x.2
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
