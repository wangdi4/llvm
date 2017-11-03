; RUN: opt -VPlanDriver -S %s | FileCheck %s

; This test checks for a widened alloca and a wide store to the widened alloca
; CHECK:  %[[PRIV2:.*]] = alloca <4 x float>
; CHECK:  %[[PRIV1:.*]] = alloca <4 x i32>
; CHECK: vector.ph
; CHECK:  %[[PRIV2_BITCAST:.*]] = bitcast <4 x float>* %[[PRIV2]] to <4 x i32>*
; CHECK: vector.body
; CHECK:   store <4 x i32> {{.*}}, <4 x i32>* %[[PRIV1]]
; CHECK:   store <4 x i32> {{.*}}, <4 x i32>* %[[PRIV2_BITCAST]]
; CHECK-NOT: scatter

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64 %n1, i32 %k1, float* nocapture %accumulated_grid, i32* nocapture readonly %iarr) {
entry:
  %count = alloca i64, align 8
  %accumulated_occupancy_input = alloca float, align 4
  %a2 = alloca i32, align 4
  %0 = bitcast i64* %count to i8*
  %cmp = icmp sgt i64 %n1, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i64* nonnull %count)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* nonnull %a2)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", float* nonnull %accumulated_occupancy_input)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.precond.then
  %x3 = bitcast float* %accumulated_occupancy_input to i32*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.QUAL.LIST.END.1
  %.omp.iv.018 = phi i64 [ 0, %DIR.QUAL.LIST.END.1 ], [ %add9, %omp.inner.for.body ]
  store i64 %.omp.iv.018, i64* %count, align 8
  %arrayidx = getelementptr inbounds i32, i32* %iarr, i64 %.omp.iv.018
  %x5 = load i32, i32* %arrayidx, align 4
  store i32 %x5, i32* %a2, align 4
;
  store i32 5, i32* %x3, align 4
;
  %add9 = add nuw nsw i64 %.omp.iv.018, 1
  %exitcond = icmp eq i64 %add9, %n1
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end(i64 8, i8* nonnull %0) #3
  ret void
}

declare void @llvm.lifetime.end(i64 , i8*)
declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opndlist(metadata , ...)

