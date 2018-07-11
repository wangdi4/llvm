; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -disable-vplan-da=false -disable-vplan-predicator -disable-vplan-codegen -debug 2>&1 | FileCheck %s 

; Verify the divergence information for the single loop for.body.

; CHECK: Printing Divergence info for Loop at depth 1 containing: BB3<header><latch><exiting>
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [[VAL1:%vp[0-9]+]] = phi i64 0 [[VAL2:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL3:%vp[0-9]+]] = getelementptr [[VAL4:%vp[0-9]+]] [[VAL1:%vp[0-9]+]]
; CHECK-NEXT: Divergent: store float 4.200000e+01 [[VAL3:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL4:%vp[0-9]+]] = add [[VAL1:%vp[0-9]+]] i64 1
; CHECK-NEXT: Uniform: [[VAL5:%vp[0-9]+]] = icmp [[VAL4:%vp[0-9]+]] [[VAL6:%vp[0-9]+]]

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
  store float 4.200000e+01, float* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void
}
