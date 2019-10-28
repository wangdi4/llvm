; Test to check that DA propagates stride information for selects with external uniform conditions generating
; a strided VectorShape.

; RUN: opt %s -VPlanDriver -debug-only=vplan-divergence-analysis -enable-vp-value-codegen=false -vplan-force-vf=2 -S 2>&1 | FileCheck %s
; RUN: opt %s -VPlanDriver -debug-only=vplan-divergence-analysis -enable-vp-value-codegen=true -vplan-force-vf=2 -S 2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK-LABEL: Printing Divergence info for Loop at depth 1
; CHECK: Basic Block: [[HEADER:BB.*]]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[IV_PHI:%vp.*]] = phi  [ i64 0, [[PREHEADER:BB.*]] ],  [ i64 [[IV_ADD:%.*]], [[HEADER]] ]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i32* [[SRC_GEP_0:%vp.*]] = getelementptr inbounds [1024 x i32]* %src i64 0 i64 [[IV_PHI]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i32* [[SRC_GEP_1:%vp.*]] = getelementptr inbounds [1024 x i32]* %src i64 1 i64 [[IV_PHI]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i32* [[SRC_SELECT:%vp.*]] = select i1 %cond i32* [[SRC_GEP_0]] i32* [[SRC_GEP_1]]
; CHECK-NEXT: Divergent: [Shape: Random] i32 [[SRC_LOAD:%vp.*]] = load i32* [[SRC_SELECT]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i32* [[DEST_GEP_0:%vp.*]] = getelementptr inbounds [1024 x i32]* %dest i64 0 i64 [[IV_PHI]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 2] i64 [[IV_STRIDED:%vp.*]] = mul i64 [[IV_PHI]] i64 2
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 8] i32* [[DEST_GEP_1:%vp.*]] = getelementptr inbounds [1024 x i32]* %dest i64 1 i64 [[IV_STRIDED]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: ?] i32* [[DEST_SELECT:%vp.*]] = select i1 %cond i32* [[DEST_GEP_0]] i32* [[DEST_GEP_1]]
; CHECK-NEXT: Divergent: [Shape: Random] store i32 [[SRC_LOAD]] i32* [[DEST_SELECT]]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[IV_ADD]] = add i64 [[IV_PHI]] i64 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[IV_CMP:%vp.*]] = icmp i64 [[IV_ADD]] i64 1024


define void @foo([1024 x i32]* %src, [1024 x i32]* %dest, i1 %cond) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %entry, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx1 = getelementptr inbounds [1024 x i32], [1024 x i32]* %src, i64 0, i64 %indvars.iv
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* %src, i64 1, i64 %indvars.iv
  %select.src = select i1 %cond, i32* %arrayidx1, i32* %arrayidx2
  %load = load i32, i32* %select.src, align 8
  %arrayidx3 = getelementptr inbounds [1024 x i32], [1024 x i32]* %dest, i64 0, i64 %indvars.iv
  %iv.2.times = mul i64 %indvars.iv, 2
  %arrayidx4 = getelementptr inbounds [1024 x i32], [1024 x i32]* %dest, i64 1, i64 %iv.2.times
  %select.dest = select i1 %cond, i32* %arrayidx3, i32* %arrayidx4
  store i32 %load, i32* %select.dest, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.loop.exit
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

