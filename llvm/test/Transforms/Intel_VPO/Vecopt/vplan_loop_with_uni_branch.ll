; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -disable-vplan-predicator -disable-vplan-codegen -debug -vplan-force-vf=2 2>&1 | FileCheck %s

; Verify the divergence information for the loop for.body with a uniform branch inside.

; CHECK: Printing Divergence info for Loop at depth 1 containing: BB3<header>,BB4,BB5<latch><exiting>
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VAL1:%vp.*]] = phi [ i64 0, BB2 ], [ i64 [[VAL2:%vp.*]], BB5 ]
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[VAL3:%vp.*]] = trunc i64 [[VAL4:%n]]
; CHECK-LABEL: Basic Block: BB4
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[VAL5:%vp.*]] = trunc i64 [[VAL4:%n]]
; CHECK-NEXT: Uniform: [Shape: Uniform] float [[VAL6:%vp.*]] = sitofp i32 [[VAL5:%vp.*]]
; CHECK-LABEL: Basic Block: BB5
; CHECK-NEXT: Uniform: [Shape: Uniform] float [[VAL7:%vp.*]] = phi [ float [[VAL6:%vp.*]], BB4 ], [ float 4.200000e+01, BB3 ]
; CHECK-NEXT: Divergent: [Shape: Unit Stride Pointer, Stride: i64 4] float* [[VAL8:%vp.*]] = getelementptr inbounds float* [[VAL9:%ptr]] i64 [[VAL1:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Random] store float [[VAL7:%vp.*]] float* [[VAL8:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VAL9:%vp.*]] = add i64 [[VAL1:%vp.*]] i64 1
; CHECK-NEXT: Uniform: [Shape: Uniform] i1 [[VAL10:%vp.*]] = icmp i64 [[VAL9:%vp.*]] i64 [[VAL4:%n]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

define void @test1(float* nocapture %ptr, i64 %n) {
  entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %B ]
  %invar = trunc i64 %n to i1
  br i1 %invar, label %A, label %B

A:
  %trunc = trunc i64 %n to i32
  %cast = sitofp i32 %trunc to float
  br label %B

B:
  %divphi = phi float [ %cast, %A ], [ 4.200000e+01, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %ptr, i64 %indvars.iv
  store float %divphi, float* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void
}
