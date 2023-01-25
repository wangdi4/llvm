; RUN: opt -S -passes="vplan-vec" -vplan-force-vf=2 < %s 2>&1 | FileCheck %s

;; Original source for reference:

;; void my_add(float &lhs, const float &rhs) { lhs += rhs; }
;; void my_init(float &t) { t = 0; }
;;
;; #pragma omp declare reduction(my_scan_add: float : my_add(omp_out, omp_in)) initializer(my_init(omp_priv))
;;
;; float uds(float *A, float *B)
;; {
;;   float red = 1.0f;
;;     #pragma omp simd reduction(inscan, my_scan_add: red)
;;     #pragma nounroll
;;     for (int i = 0; i < 1024; ++i) {
;;         B[i] = red;
;;         #pragma omp scan exclusive(red)
;;         my_add(red, A[i]);
;;     }
;;     return red;
;; }

; CHECK-LABEL: DIR.OMP.SIMD.1:
; CHECK:         [[RED_ORIG:%.*]] = alloca float, align 4
; CHECK:         [[RED_VEC_TEMP:%.*]] = alloca <2 x float>, align 8
; CHECK-NEXT:    [[RED_VEC_TEMP_BASE_ADDR:%.*]] = getelementptr float, ptr [[RED_VEC_TEMP]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x ptr> [[RED_VEC_TEMP_BASE_ADDR]], i32 0
; CHECK-NEXT:    [[RED_VEC:%.*]] = alloca <2 x float>, align 8
; CHECK-NEXT:    [[RED_VEC_BASE_ADDR:%.*]] = getelementptr float, ptr [[RED_VEC]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x ptr> [[RED_VEC_BASE_ADDR]], i32 1
; CHECK-NEXT:    [[RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x ptr> [[RED_VEC_BASE_ADDR]], i32 0

; CHECK-LABEL: VPlannedBB1:
; CHECK:         call void @llvm.lifetime.start.p0(i64 8, ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @llvm.lifetime.start.p0(i64 8, ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]])

; CHECK-LABEL: vector.body:
; CHECK:         call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]], ptr [[RED_ORIG]])

; CHECK-LABEL: VPlannedBB14:
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_ORIG]], ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]], ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]])
; CHECK-NEXT:    call void @.omp_initializer.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_VEC_BASE_ADDR_EXTRACT_1]], ptr [[RED_ORIG]])
; CHECK-NEXT:    call void @.omp_combiner.(ptr [[RED_ORIG]], ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    br label

; CHECK-LABEL: VPlannedBB19:
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 8, ptr [[RED_VEC_TEMP_BASE_ADDR_EXTRACT_0]])
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 8, ptr [[RED_VEC_BASE_ADDR_EXTRACT_0]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @_Z6my_addRfRKf(ptr %lhs, ptr %rhs)

declare void @_Z7my_initRf(ptr %t)

define float @_Z3udsPfS_(ptr %A, ptr %B) {
DIR.OMP.SIMD.1:
  %red.red = alloca float, align 4
  %i.linear.iv = alloca i32, align 4
  store float 0.000000e+00, ptr %red.red, align 4
  br label %DIR.OMP.SIMD.127

DIR.OMP.SIMD.127:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr %red.red, float 0.000000e+00, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer., i64 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.VPO.END.GUARD.MEM.MOTION.426:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4, %DIR.OMP.SIMD.127
  %indvars.iv = phi i64 [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.4 ], [ 0, %DIR.OMP.SIMD.127 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2.split

DIR.VPO.GUARD.MEM.MOTION.2.split:                 ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.426
  %guard1.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %red.red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.2.split
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %i.linear.iv, align 4
  %2 = load float, ptr %red.red, align 4
  %arrayidx = getelementptr inbounds float, ptr %B, i64 %indvars.iv
  store float %2, ptr %arrayidx, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.552

DIR.VPO.END.GUARD.MEM.MOTION.552:                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  call void @llvm.directive.region.exit(token %guard1.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.552
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.EXCLUSIVE"(ptr %red.red, i64 1) ]
  br label %DIR.OMP.SCAN.2

DIR.OMP.SCAN.2:                                   ; preds = %DIR.OMP.SCAN.4
  fence acq_rel
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.SCAN.2
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.8

DIR.OMP.END.SCAN.8:
  %guard2.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.END.SCAN.3

DIR.OMP.END.SCAN.3:                               ; preds = %DIR.OMP.END.SCAN.8
  %4 = load i32, ptr %i.linear.iv, align 4
  %idxprom1 = sext i32 %4 to i64
  %arrayidx2 = getelementptr inbounds float, ptr %A, i64 %idxprom1
  %5 = load float, ptr %arrayidx2, align 4
  %6 = load float, ptr %red.red, align 4
  %add.i = fadd fast float %6, %5
  store float %add.i, ptr %red.red, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.8

DIR.VPO.END.GUARD.MEM.MOTION.8:                   ; preds = %DIR.OMP.END.SCAN.3
  call void @llvm.directive.region.exit(token %guard2.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.8
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.5, label %DIR.VPO.END.GUARD.MEM.MOTION.426

DIR.OMP.END.SIMD.5:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.529

DIR.OMP.END.SIMD.529:                             ; preds = %DIR.OMP.END.SIMD.5
  %7 = load float, ptr %red.red, align 4
  %add.i.i = fadd fast float %7, 1.000000e+00
  ret float %add.i.i
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_combiner.(ptr %0, ptr %1)

declare void @.omp_initializer.(ptr %0, ptr %1)
