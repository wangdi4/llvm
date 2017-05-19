; RUN: opt -VPlanDriver -disable-vplan-predicator -disable-vplan-subregions -S %s | FileCheck %s

; This test checks for a widened alloca and a wide store to the widened alloca
; CHECK: vector.ph
; CHECK:  %[[PRIV1:.*]] = alloca <4 x float>
; CHECK:  %[[PRIV1_VEC:.*]] = getelementptr float
; CHECK:  %[[PRIV1_BITCAST:.*]] = bitcast <4 x float*> %[[PRIV1_VEC]] to <4 x i32*>
; CHECK: vector.body
; CHECK:   getelementptr i32, <4 x i32*> %[[PRIV1_BITCAST]]
; CHECK:   masked.scatter
; CHECK: extractelement <4 x i32*> %[[PRIV1_BITCAST]], i32 0
; CHECK: call
; CHECK: extractelement <4 x i32*> %[[PRIV1_BITCAST]], i32 1
; CHECK: call

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64 %n1, i32 %k1, float* nocapture %accumulated_grid, i32* nocapture readonly %iarr) {
entry:
  %count = alloca i64, align 8
  %accumulated_occupancy_input = alloca float, align 4
  %a2 = alloca i32, align 4
  %accumulated_occupancy_output = alloca float, align 4
  %0 = bitcast i64* %count to i8*
  %cmp = icmp sgt i64 %n1, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i64* nonnull %count)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", float* nonnull %accumulated_occupancy_output)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* nonnull %a2)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", float* nonnull %accumulated_occupancy_input)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.precond.then
  %1 = bitcast float* %accumulated_occupancy_input to i8*
  %2 = bitcast i32* %a2 to i8*
  %3 = bitcast float* %accumulated_occupancy_input to i32*
  %4 = bitcast float* %accumulated_occupancy_output to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.QUAL.LIST.END.1
  %.omp.iv.018 = phi i64 [ 0, %DIR.QUAL.LIST.END.1 ], [ %add9, %omp.inner.for.body ]
;
  %arrayidx7 = getelementptr inbounds float, float* %accumulated_grid, i64 %.omp.iv.018
  %5 = bitcast float* %arrayidx7 to i32*
  %6 = load i32, i32* %3, align 4
;
  %addr = getelementptr i32, i32 * %3, i64 2
  store i32 %6, i32* %addr, align 4
;
  %.cast = bitcast i32 %6 to float
  %call = call float @baz(float %.cast, i32* %3) #3
  %7 = load i64, i64* %count, align 8
;
  %arrayidx8 = getelementptr inbounds float, float* %accumulated_grid, i64 %7
  store float %call, float* %arrayidx8, align 4
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
declare float @baz(float, i32*) local_unnamed_addr #1

declare void @llvm.lifetime.end(i64 , i8*)
declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opndlist(metadata , ...)

