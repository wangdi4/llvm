; RUN: opt -passes=vplan-vec -vplan-force-vf=4 -S %s | FileCheck %s

; Deprecated the llvm.intel.directive* representation.
; TODO: Update this test to use llvm.directive.region.entry/exit instead.
; XFAIL: *

; This test checks for a widened alloca and a wide store to the widened alloca
; CHECK:  %[[PRIV2:.*]] = alloca <4 x float>
; CHECK:  %[[PRIV1:.*]] = alloca <4 x i32>
; CHECK: vector.ph
; CHECK: vector.body
; CHECK:   store <4 x i32> {{.*}}, ptr %[[PRIV1]]
; CHECK:   store <4 x i32> {{.*}}, ptr %[[PRIV2]]
; CHECK-NOT: scatter

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64 %n1, i32 %k1, ptr nocapture %accumulated_grid, ptr nocapture readonly %iarr) {
entry:
  %count = alloca i64, align 8
  %accumulated_occupancy_input = alloca float, align 4
  %a2 = alloca i32, align 4
  %cmp = icmp sgt i64 %n1, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", ptr nonnull %count)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", ptr nonnull %a2)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", ptr nonnull %accumulated_occupancy_input)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.precond.then
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.QUAL.LIST.END.1
  %.omp.iv.018 = phi i64 [ 0, %DIR.QUAL.LIST.END.1 ], [ %add9, %omp.inner.for.body ]
  store i64 %.omp.iv.018, ptr %count, align 8
  %arrayidx = getelementptr inbounds i32, ptr %iarr, i64 %.omp.iv.018
  %x5 = load i32, ptr %arrayidx, align 4
  store i32 %x5, ptr %a2, align 4
;
  store i32 5, ptr %accumulated_occupancy_input, align 4
;
  %add9 = add nuw nsw i64 %.omp.iv.018, 1
  %exitcond = icmp eq i64 %add9, %n1
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end(i64 8, ptr nonnull %count) #3
  ret void
}

declare void @llvm.lifetime.end(i64 , ptr)
declare void @llvm.intel.directive(metadata)
declare void @llvm.intel.directive.qual.opndlist(metadata , ...)

