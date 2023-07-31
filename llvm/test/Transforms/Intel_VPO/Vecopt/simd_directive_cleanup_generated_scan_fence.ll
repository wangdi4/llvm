; RUN: opt -passes="vplan-vec,vpo-directive-cleanup" -vplan-force-vf=4 -S < %s 2>&1 | FileCheck %s

; Ensure that compiler generated fence instruction between scan reduction
; directives is removed by VPODirectiveCleanup pass.

;; void foo(float *A, float *B) {
;;   float x = 0.0f;
;; #pragma omp simd reduction(inscan, + : x)
;;   for (int i=0; i<1025; i++) {
;;     x += A[i];
;; #pragma omp scan inclusive(x)
;;     B[i] = x;
;;   }
;; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @omp_scan(ptr %A, ptr %B) {
;
; CHECK-LABEL:  vector.body:
; CHECK-NOT:     fence acq_rel
; CHECK:         ret void
;
DIR.OMP.SIMD.1:
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  store float 0.000000e+00, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.124

DIR.OMP.SIMD.124:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.VPO.END.GUARD.MEM.MOTION.426:                 ; preds = %DIR.OMP.SIMD.124, %DIR.VPO.END.GUARD.MEM.MOTION.4
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.124 ], [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.4 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.426
  %guard.start1 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.SIMD.139

DIR.OMP.SIMD.139:                                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  br label %DIR.OMP.END.SCAN.2

DIR.OMP.END.SCAN.2:                               ; preds = %DIR.OMP.SIMD.139
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv)
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %2 = load float, ptr %arrayidx, align 4
  %3 = load float, ptr %x.red, align 4
  %add1 = fadd fast float %3, %2
  store float %add1, ptr %x.red, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.552

DIR.VPO.END.GUARD.MEM.MOTION.552:                 ; preds = %DIR.OMP.END.SCAN.2
  call void @llvm.directive.region.exit(token %guard.start1) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE:TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1) ]
  br label %DIR.OMP.SCAN.1

DIR.OMP.SCAN.1:                                   ; preds = %DIR.OMP.SCAN.2
  fence acq_rel
  br label %DIR.OMP.END.SCAN.4

DIR.OMP.END.SCAN.4:                               ; preds = %DIR.OMP.SCAN.1
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.9.split

DIR.OMP.END.SCAN.9.split:                         ; preds = %DIR.OMP.END.SCAN.4
  %guard.start2 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.END.SCAN.226

DIR.OMP.END.SCAN.226:                             ; preds = %DIR.OMP.END.SCAN.9.split
  %5 = load float, ptr %x.red, align 4
  %6 = load i32, ptr %i.linear.iv, align 4
  %idxprom2 = sext i32 %6 to i64
  %arrayidx3 = getelementptr inbounds float, ptr %B, i64 %idxprom2
  store float %5, ptr %arrayidx3, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.8

DIR.VPO.END.GUARD.MEM.MOTION.8:                   ; preds = %DIR.OMP.END.SCAN.226
  call void @llvm.directive.region.exit(token %guard.start2) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.8
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1025
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.3, label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.327

DIR.OMP.END.SIMD.327:                             ; preds = %DIR.OMP.END.SIMD.3
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
