; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -disable-vplan-da=false -disable-vplan-predicator -disable-vplan-codegen -debug 2>&1 | FileCheck %s 

; Verify the divergence information for the outermost loop for.body.

; CHECK: Printing Divergence info for Loop at depth 1 containing: BB3<header>,BB4,BB5<latch><exiting>
; CHECK-NEXT: Loop at depth 2 containing: BB4<header><latch><exiting>
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [[VAL1:%vp[0-9]+]] = phi i64 0 [[VAL2:%vp[0-9]+]]
; CHECK-LABEL: Basic Block: BB4
; CHECK-NEXT: Uniform: [[VAL3:%vp[0-9]+]] = phi i64 0 [[VAL4:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL5:%vp[0-9]+]] = mul [[VAL6:%vp[0-9]+]] [[VAL1:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL7:%vp[0-9]+]] = add [[VAL5:%vp[0-9]+]] [[VAL3:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL8:%vp[0-9]+]] = trunc [[VAL7:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL9:%vp[0-9]+]] = sitofp [[VAL8:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL10:%vp[0-9]+]] = getelementptr [[VAL11:%vp[0-9]+]] [[VAL7:%vp[0-9]+]]
; CHECK-NEXT: Divergent: store [[VAL9:%vp[0-9]+]] [[VAL10:%vp[0-9]+]]
; CHECK-NEXT: Uniform: [[VAL12:%vp[0-9]+]] = add [[VAL3:%vp[0-9]+]] i64 1
; CHECK-NEXT: Uniform: [[VAL13:%vp[0-9]+]] = icmp [[VAL12:%vp[0-9]+]] [[VAL6:%vp[0-9]+]]
; CHECK-LABEL: Basic Block: BB5
; CHECK-NEXT: Divergent: [[VAL2:%vp[0-9]+]] = add [[VAL1:%vp[0-9]+]] i64 1
; CHECK-NEXT: Uniform: [[VAL14:%vp[0-9]+]] = icmp [[VAL2:%vp[0-9]+]] [[VAL6:%vp[0-9]+]]

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
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
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
