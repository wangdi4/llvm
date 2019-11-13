; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -disable-vplan-predicator -disable-vplan-codegen -debug -vplan-force-vf=2 2>&1 | FileCheck %s

; Verify the divergence information for the single loop for.body.

; CHECK: Printing Divergence info for Loop at depth 1 containing: BB3<header><latch><exiting>
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VAL1:%vp.*]] = phi [ i64 0, BB2 ], [ i64 [[VAL2:%vp.*]], BB3 ]
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] float* [[VAL3:%vp.*]] = getelementptr inbounds float* [[VAL4:%A]] i64 [[VAL1:%vp.*]]
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[VAL5:%vp.*]] = trunc i64 [[VAL6:%n]]
; CHECK-NEXT: Uniform: [Shape: Uniform] float [[VAL7:%vp.*]] = sitofp i32 [[VAL5:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Random] store float [[VAL7:%vp.*]] float* [[VAL3:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VAL2:%vp.*]] = add i64 [[VAL1:%vp.*]] i64 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[VAL8:%vp.*]] = icmp i64 [[VAL2:%vp.*]] i64 [[VAL6:%n]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

define void @test1(float* nocapture %A, i64 %n) {
  entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %trunc = trunc i64 %n to i32
  %cast = sitofp i32 %trunc to float
  store float %cast, float* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void
}

