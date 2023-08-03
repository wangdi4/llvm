; RUN: opt -disable-output -passes="vplan-vec" -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=2  -vplan-cost-model-print-analysis-for-vf=2  < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF2
; RUN: opt -disable-output -passes="vplan-vec" -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=4  -vplan-cost-model-print-analysis-for-vf=4  < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF4
; RUN: opt -disable-output -passes="vplan-vec" -mtriple=x86_64-unknown-unknown -mattr=+avx2 -vplan-force-vf=8  -vplan-cost-model-print-analysis-for-vf=8  < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF8
; RUN: opt -disable-output -passes="vplan-vec" -mtriple=x86_64-unknown-unknown -mattr=+avx512f -vplan-force-vf=16 -vplan-cost-model-print-analysis-for-vf=16 < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF16

;; Test costs for inscan reduction related instructions:
;;  * reduction-init-scalar;
;;  * reduction-final-inscan;
;;  * running-inclusive-reduction (cost is VF-dependent);
;;  * running-exclusive-reduction (cost is VF-dependent);
;;  * extract-last-vector-lane.

;; void inclusive_scan(float *A, float *B) {
;;   float x = 0.0f;
;; #pragma omp simd reduction(inscan, + : x)
;; #pragma nounroll
;;   for (int i=0; i<1024; i++) {
;;     x += A[i];
;; #pragma omp scan inclusive(x)
;;     B[i] = x;
;;   }
;; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @inclusive_scan(ptr %A, ptr %B) {
; CHECK-VF2:     Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF2:     Cost 5 for float %vp{{[0-9]+}} = running-inclusive-reduction
; CHECK-VF2:     Cost 1 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF2:     Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
; CHECK-VF4:     Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF4:     Cost 8 for float %vp{{[0-9]+}} = running-inclusive-reduction
; CHECK-VF4:     Cost 1 for float %vp{{[0-9]+}} = extract-last-vector-lane
; CHECK-VF4:     Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
; CHECK-VF8:     Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF8:     Cost 14 for float %vp{{[0-9]+}} = running-inclusive-reduction
; CHECK-VF8:     Cost 2 for float %vp{{[0-9]+}} = extract-last-vector-lane
; CHECK-VF8:     Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
; CHECK-VF16:    Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF16:    Cost 10 for float %vp{{[0-9]+}} = running-inclusive-reduction
; CHECK-VF16:    Cost 2 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF16:    Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
entry:
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store float 0.000000e+00, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.138

DIR.OMP.SIMD.138:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x.red, float zeroinitializer, i32 1, i64 1), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.VPO.END.GUARD.MEM.MOTION.426:                 ; preds = %DIR.OMP.SIMD.138, %DIR.VPO.END.GUARD.MEM.MOTION.4
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.138 ], [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.4 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.426
  %guard.start1 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.END.SCAN.335

DIR.OMP.END.SCAN.335:                             ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %2 = load float, ptr %arrayidx, align 4
  %3 = load float, ptr %x.red, align 4
  %add5 = fadd fast float %3, %2
  store float %add5, ptr %x.red, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.552

DIR.VPO.END.GUARD.MEM.MOTION.552:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  call void @llvm.directive.region.exit(token %guard.start1) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE"(ptr %x.red, i64 1) ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.OMP.SCAN.3
  fence acq_rel
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.SCAN.2
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.8

DIR.OMP.END.SCAN.8:                               ; preds = %DIR.OMP.END.SCAN.5
  %guard.start2 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.END.SCAN.3

DIR.OMP.END.SCAN.3:                               ; preds = %DIR.OMP.END.SCAN.9
  %5 = load float, ptr %x.red, align 4
  %6 = load i32, ptr %i.linear.iv, align 4
  %idxprom6 = sext i32 %6 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %B, i64 %idxprom6
  store float %5, ptr %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.8

DIR.VPO.END.GUARD.MEM.MOTION.8:                   ; preds = %DIR.OMP.END.SCAN.3
  call void @llvm.directive.region.exit(token %guard.start2) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.8
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.1, %entry
  ret void
}

;; float exclusive_scan(float *A, float *B) {
;;   float x = 1.0f;
;; #pragma omp simd reduction(inscan, + : x)
;; #pragma nounroll
;;   for (int i=0; i<1024; i++) {
;;     B[i] = x;
;; #pragma omp scan exclusive(x)
;;     x += A[i];
;;   }
;;   return x;
;; }

define float @exclusive_scan(ptr %A, ptr %B) {
; CHECK-VF2:     Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF2:     Cost 7 for float %vp{{[0-9]+}} = running-exclusive-reduction
; CHECK-VF2:     Cost 1 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF2:     Cost 1 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF2:     Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
; CHECK-VF4:     Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF4:     Cost 10 for float %vp{{[0-9]+}} = running-exclusive-reduction
; CHECK-VF4:     Cost 1 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF4:     Cost 1 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF4:     Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
; CHECK-VF8:     Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF8:     Cost 17 for float %vp{{[0-9]+}} = running-exclusive-reduction
; CHECK-VF8:     Cost 2 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF8:     Cost 2 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF8:     Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
; CHECK-VF16:    Cost 1 for float %vp{{[0-9]+}} = reduction-init-scalar
; CHECK-VF16:    Cost 11 for float %vp{{[0-9]+}} = running-exclusive-reduction
; CHECK-VF16:    Cost 2 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF16:    Cost 2 for float %vp{{[0-9]+}} = extract-last-vector-lane float
; CHECK-VF16:    Cost 0 for float %vp{{[0-9]+}} = reduction-final-inscan
;
DIR.OMP.SIMD.1:
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  store float 1.000000e+00, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.126

DIR.OMP.SIMD.126:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.VPO.END.GUARD.MEM.MOTION.426:                 ; preds = %DIR.OMP.SIMD.126, %DIR.VPO.END.GUARD.MEM.MOTION.4
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.126 ], [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.4 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.426
  %guard.start1 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.END.SCAN.2

DIR.OMP.END.SCAN.2:                               ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv)
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %2 = load float, ptr %x.red, align 4
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  store float %2, ptr %arrayidx, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.552

DIR.VPO.END.GUARD.MEM.MOTION.552:                 ; preds = %DIR.OMP.END.SCAN.2
  call void @llvm.directive.region.exit(token %guard.start1) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.EXCLUSIVE:TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1) ]
  br label %DIR.OMP.SCAN.1

DIR.OMP.SCAN.1:                                   ; preds = %DIR.OMP.SCAN.2
  fence acq_rel
  br label %DIR.OMP.END.SCAN.4

DIR.OMP.END.SCAN.4:                               ; preds = %DIR.OMP.SCAN.1
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.9.split

DIR.OMP.END.SCAN.9.split:                         ; preds = %DIR.OMP.END.SCAN.4
  %guard.start2 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.END.SCAN.228

DIR.OMP.END.SCAN.228:                             ; preds = %DIR.OMP.END.SCAN.9.split
  %4 = load i32, ptr %i.linear.iv, align 4
  %idxprom1 = sext i32 %4 to i64
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %5 = load float, ptr %arrayidx2, align 4
  %6 = load float, ptr %x.red, align 4
  %add3 = fadd fast float %6, %5
  store float %add3, ptr %x.red, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.8

DIR.VPO.END.GUARD.MEM.MOTION.8:                   ; preds = %DIR.OMP.END.SCAN.228
  call void @llvm.directive.region.exit(token %guard.start2) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.8
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.6, label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.OMP.END.SIMD.6:                               ; preds = %DIR.OMP.END.SCAN.228
  %add3.lcssa = phi float [ %add3, %DIR.VPO.END.GUARD.MEM.MOTION.4 ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.OMP.END.SIMD.6
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.1
  ret float %add3.lcssa
}

;; min/max inclusive inscan reduction.

;; float max_inclusive(float *A, float *B) {
;;   float x = 1.0f;
;; #pragma omp simd reduction(inscan, max : x)
;; #pragma nounroll
;;   for (int i=0; i<1024; i++) {
;;     x = std::max(A[i], x);
;; #pragma omp scan inclusive(x)
;;     B[i] = x;
;;   }
;;   return x;
;; }

; CHECK-VF2:  Cost 7 for float %vp{{[0-9]+}} = running-inclusive-reduction{fmax}
; CHECK-VF4:  Cost 11 for float %vp{{[0-9]+}} = running-inclusive-reduction{fmax}
; CHECK-VF8:  Cost 22 for float %vp{{[0-9]+}} = running-inclusive-reduction{fmax}
; CHECK-VF16: Cost 25 for float %vp{{[0-9]+}} = running-inclusive-reduction{fmax}

define float @max_inclusive(ptr %A, ptr %B) {
DIR.OMP.SIMD.1:
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  store float 1.000000e+00, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.127

DIR.OMP.SIMD.127:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX:INSCAN.TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0)
, "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.VPO.END.GUARD.MEM.MOTION.426:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4, %DIR.OMP.SIMD.127
  %indvars.iv = phi i64 [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.4 ], [ 0, %DIR.OMP.SIMD.127 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2.split

DIR.VPO.GUARD.MEM.MOTION.2.split:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.426
  %guard.start1 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.2.split
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %arrayidx = getelementptr inbounds float, ptr %A, i64 %indvars.iv
  %2 = load float, ptr %arrayidx, align 4
  %3 = load float, ptr %x.red, align 4
  %cmp.i = fcmp fast olt float %2, %3
  %4 = select i1 %cmp.i, float %3, float %2
  store float %4, ptr %x.red, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.552

DIR.VPO.END.GUARD.MEM.MOTION.552:                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  call void @llvm.directive.region.exit(token %guard.start1) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.INCLUSIVE"(ptr %x.red, i64 1) ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.OMP.SCAN.4
  fence acq_rel
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.SCAN.2
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.9.split

DIR.OMP.END.SCAN.9.split:                         ; preds = %DIR.OMP.END.SCAN.6
  %guard.start2 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.END.SCAN.3

DIR.OMP.END.SCAN.3:                               ; preds = %DIR.OMP.END.SCAN.9.split
  %6 = load float, ptr %x.red, align 4
  %7 = load i32, ptr %i.linear.iv, align 4
  %idxprom1 = sext i32 %7 to i64
  %arrayidx2 = getelementptr inbounds float, ptr %B, i64 %idxprom1
  store float %6, ptr %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.8

DIR.VPO.END.GUARD.MEM.MOTION.8:                   ; preds = %DIR.OMP.END.SCAN.3
  call void @llvm.directive.region.exit(token %guard.start2) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.8
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.5, label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.OMP.END.SIMD.5:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.529

DIR.OMP.END.SIMD.529:                             ; preds = %DIR.OMP.END.SIMD.5
  %8 = load float, ptr %x.red, align 4
  ret float %8
}

;; float max_exclusive(float *A, float *B) {
;;   float x = 1.0f;
;; #pragma omp simd reduction(inscan, max : x)
;; #pragma nounroll
;;   for (int i=0; i<1024; i++) {
;;     B[i] = x;
;; #pragma omp scan exclusive(x)
;;     x = std::max(A[i], x);
;;   }
;;   return x;
;; }

; CHECK-VF2: Cost 9 for float %vp{{[0-9]+}} = running-exclusive-reduction{fmax}
; CHECK-VF2: Cost 2 for float %vp{{[0-9]+}} = fmax float

; CHECK-VF4: Cost 13 for float %vp{{[0-9]+}} = running-exclusive-reduction{fmax}
; CHECK-VF4: Cost 2 for float %vp{{[0-9]+}} = fmax float

; CHECK-VF8: Cost 25 for float %vp{{[0-9]+}} = running-exclusive-reduction{fmax}
; CHECK-VF8: Cost 3 for float %vp{{[0-9]+}} = fmax float

; CHECK-VF16: Cost 26 for float %vp{{[0-9]+}} = running-exclusive-reduction{fmax}
; CHECK-VF16: Cost 4 for float %vp{{[0-9]+}} = fmax float

define float @max_exclusive(ptr %A, ptr %B) {
DIR.OMP.SIMD.1:
  %x.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  store float 1.000000e+00, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.127

DIR.OMP.SIMD.127:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.MAX:INSCAN.TYPED"(ptr %x.red, float 0.000000e+00, i32 1, i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0)
, "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.VPO.END.GUARD.MEM.MOTION.426:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4, %DIR.OMP.SIMD.127
  %indvars.iv = phi i64 [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.4 ], [ 0, %DIR.OMP.SIMD.127 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2.split

DIR.VPO.GUARD.MEM.MOTION.2.split:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.426
  %guard.start1 = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.2.split
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %2 = load float, ptr %x.red, align 4
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  store float %2, ptr %arrayidx, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.552

DIR.VPO.END.GUARD.MEM.MOTION.552:                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  call void @llvm.directive.region.exit(token %guard.start1) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.EXCLUSIVE"(ptr %x.red, i64 1) ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.OMP.SCAN.4
  fence acq_rel
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.SCAN.2
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.9.split

DIR.OMP.END.SCAN.9.split:                         ; preds = %DIR.OMP.SCAN.2
  %guard.start2 =  call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  br label %DIR.OMP.END.SCAN.3

DIR.OMP.END.SCAN.3:                               ; preds = %DIR.OMP.END.SCAN.9.split
  %4 = load i32, ptr %i.linear.iv, align 4
  %idxprom1 = sext i32 %4 to i64
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %5 = load float, ptr %arrayidx2, align 4
  %6 = load float, ptr %x.red, align 4
  %cmp.i = fcmp fast olt float %5, %6
  %7 = select i1 %cmp.i, float %6, float %5
  store float %7, ptr %x.red, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.8

DIR.VPO.END.GUARD.MEM.MOTION.8:                   ; preds = %DIR.OMP.END.SCAN.3
  call void @llvm.directive.region.exit(token %guard.start2) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.8
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.5, label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.OMP.END.SIMD.5:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.529

DIR.OMP.END.SIMD.529:                             ; preds = %DIR.OMP.END.SIMD.5
  %8 = load float, ptr %x.red, align 4
  ret float %8
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
