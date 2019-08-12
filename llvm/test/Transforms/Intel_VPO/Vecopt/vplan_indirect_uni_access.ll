; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -vplan-force-build -disable-vplan-predicator -disable-vplan-codegen -debug 2>&1 | FileCheck %s 
; XFAIL: *
; See explanation in vplan_non_affine_uni_loop.ll.

; Verify the divergence information for the single loop for.body.

; CHECK: Printing Divergence info for Printing Divergence info for Loop at depth 1 containing: BB3<header>,BB4,BB6,BB5<latch><exiting>
; CHECK-NEXT: Loop at depth 2 containing: BB6<header><latch><exiting>
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [[VAL1:%vp.*]] = phi i64 0 [[VAL2:%vp.*]]
; CHECK-LABEL: Basic Block: BB6
; CHECK-NEXT: Uniform: [[VAL3:%vp.*]] = phi [[VAL4:%vp.*]] i64 0
; CHECK-NEXT: Divergent: [[VAL5:%vp.*]] = phi [[VAL6:%vp.*]] double 0.000000e+00
; CHECK-NEXT: Uniform: [[VAL7:%vp.*]] = getelementptr [[VAL8:%vp.*]] [[VAL3:%vp.*]]
; CHECK-NEXT: Uniform: [[VAL9:%vp.*]] = load [[VAL7:%vp.*]]
; CHECK-NEXT: Uniform: [[VAL10:%vp.*]] = sext [[VAL9:%vp.*]]
; CHECK-NEXT: Uniform: [[VAL11:%vp.*]] = getelementptr [[VAL12:%vp.*]] [[VAL10:%vp.*]]
; CHECK-NEXT: Uniform: [[VAL13:%vp.*]] = load [[VAL11:%vp.*]]
; CHECK-NEXT: Divergent: [[VAL14:%vp.*]] = getelementptr [[VAL13:%vp.*]] [[VAL1:%vp.*]]
; CHECK-NEXT: Divergent: [[VAL15:%vp.*]] = load [[VAL14:%vp.*]]
; CHECK-NEXT: Divergent: [[VAL6:%vp.*]] = fadd [[VAL5:%vp.*]] [[VAL15:%vp.*]]
; CHECK-NEXT: Uniform: [[VAL4:%vp.*]] = add %[[VAL3:vp[0-9]+]] i64 1
; CHECK-NEXT: Uniform: [[VAL16:%vp.*]] = icmp [[VAL4:%vp.*]] [[VAL17:%vp.*]]
; CHECK-LABEL: Basic Block: BB5
; CHECK-NEXT: Divergent: [[VAL18:%vp.*]] = phi double 0.000000e+00 [[VAL6:%vp.*]]
; CHECK-NEXT: Divergent: [[VAL19:%vp.*]] = getelementptr [[VAL20:%vp.*]] [[VAL1:%vp.*]]
; CHECK-NEXT: Divergent: store [[VAL18:%vp.*]] [[VAL19:%vp.*]]
; CHECK-NEXT: Divergent: [[VAL2:%vp.*]] = add [[VAL1:%vp.*]] i64 1
; CHECK-NEXT: Uniform: [[VAL21:%vp.*]] = icmp [[VAL2:%vp.*]] [[VAL22:%vp.*]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: norecurse nounwind uwtable
define void @test(i32* nocapture readonly %Index, double** nocapture readonly %A, double* nocapture %C, i32 %m, i32 %n) {
entry:
  %cmp27 = icmp sgt i32 %n, 0
  br i1 %cmp27, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp224 = icmp sgt i32 %m, 0
  %wide.trip.count = zext i32 %m to i64
  %wide.trip.count31 = zext i32 %n to i64
  br label %for.body.lr.ph2

for.body.lr.ph2:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.cond.cleanup3, %for.body.lr.ph2
  %indvars.iv29 = phi i64 [ 0, %for.body.lr.ph2 ], [ %indvars.iv.next30, %for.cond.cleanup3 ]
  br i1 %cmp224, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4, %for.body
  %x.0.lcssa = phi double [ 0.000000e+00, %for.body ], [ %add, %for.body4 ]
  %arrayidx10 = getelementptr inbounds double, double* %C, i64 %indvars.iv29
  store double %x.0.lcssa, double* %arrayidx10, align 8
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond32 = icmp eq i64 %indvars.iv.next30, %wide.trip.count31
  br i1 %exitcond32, label %for.end, label %for.body

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.body4.preheader ]
  %x.025 = phi double [ %add, %for.body4 ], [ 0.000000e+00, %for.body4.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %Index, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %idxprom5 = sext i32 %0 to i64
  %arrayidx6 = getelementptr inbounds double*, double** %A, i64 %idxprom5
  %1 = load double*, double** %arrayidx6, align 8
  %arrayidx8 = getelementptr inbounds double, double* %1, i64 %indvars.iv29
  %2 = load double, double* %arrayidx8, align 8
  %add = fadd double %x.025, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void
}
