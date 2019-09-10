; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -disable-vplan-codegen -debug 2>&1 | FileCheck %s 

; Verify the divergence information for the outermost loop for.body.

; CHECK: Printing Divergence info for Loop at depth 1 containing: BB3<header>,BB4,BB5<latch><exiting>
; CHECK-NEXT: Loop at depth 2 containing: BB4<header><latch><exiting>
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VAL1:%vp.*]] = phi [ i64 0, BB2 ], [ i64 [[VAL2:%vp.*]], BB5 ]
; CHECK-LABEL: Basic Block: BB4
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[VAL3:%vp.*]] = phi [ i64 0, BB3 ], [ i64 [[VAL4:%vp.*]], BB4 ]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: ?] i64 [[VAL5:%vp.*]] = mul i64 [[VAL6:%n]] i64 [[VAL1:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: ?] i64 [[VAL7:%vp.*]] = add i64 [[VAL5:%vp.*]] i64 [[VAL3:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Random] i32 [[VAL8:%vp.*]] = trunc i64 [[VAL7:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Random] float [[VAL9:%vp.*]] = sitofp i32 [[VAL8:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: ?] float* [[VAL10:%vp.*]] = getelementptr inbounds float* [[VAL11:%ptr]] i64 [[VAL7:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Random] store float [[VAL9:%vp.*]] float* [[VAL10:%vp.*]]
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[VAL11:%vp.*]] = add i64 [[VAL3:%vp.*]] i64 1
; CHECK-NEXT: Divergent: [Shape: Random] i1 [[VAL12:%vp.*]] = icmp i64 [[VAL4:%vp.*]] i64 [[VAL1:%vp.*]]
; CHECK-LABEL: Basic Block: BB5
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VAL13:%vp.*]] = add i64 [[VAL1:%vp.*]] i64 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[VAL14:%vp.*]] = icmp i64 [[VAL13:%vp.*]] i64 [[VAL6:%n]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

define void @test1(float* nocapture %ptr, i64 %n) {
  entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  br label %for.body2

for.body2:
  %indvars.iv2 = phi i64 [ 0, %for.body ], [ %indvars.iv.next2, %for.body2 ]
  %row = mul i64 %n, %indvars.iv
  %idx = add i64 %row, %indvars.iv2
  %trunc = trunc i64 %idx to i32
  %val = sitofp i32 %trunc to float
  %arrayidx = getelementptr inbounds float, float* %ptr, i64 %idx
  store float %val, float* %arrayidx, align 4
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp sge i64 %indvars.iv.next2, %indvars.iv
  br i1 %exitcond2, label %for.latch, label %for.body2

for.latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}
