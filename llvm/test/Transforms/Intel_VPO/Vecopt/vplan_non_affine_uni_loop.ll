; REQUIRES: asserts
; RUN: opt %s -VPlanDriver -vplan-force-build -disable-vplan-da=false -disable-vplan-predicator -disable-vplan-codegen -debug 2>&1 | FileCheck %s 
; XFAIL: *
; Note: At the moment, we do not build a VPlan for this loop nest since there are multiple loop exit blocks. When -vplan-force-build is thrown, HCFG construction
; also asserts for the same reason. For now just mark as XFAIL and once the VPlan is built for such cases this test should pass.

; Verify the divergence information for the outermost loop region.

; CHECK: Printing Divergence info for Loop at depth 1 containing: BB3<header>,BB4,BB6,BB7,BB8,BB5<latch><exiting>
; CHECK-NEXT: Loop at depth 2 containing: BB6<header>,BB7,BB8<latch><exiting>
; CHECK-NEXT: Loop at depth 3 containing: BB7<header><latch><exiting>
; CHECK-LABEL: Basic Block: BB3
; CHECK-NEXT: Divergent: [[VAL1:%vp[0-9]+]] = phi i64 0 [[VAL2:%vp[0-9]+]]
; CHECK-LABEL: Basic Block: BB6
; CHECK-NEXT: Uniform: [[VAL3:%vp[0-9]+]] = phi [[VAL4:%vp[0-9]+]] i32 2
; CHECK-NEXT: Uniform: [[VAL5:%vp[0-9]+]] = phi [[VAL3:%vp[0-9]+]] i32 1
; CHECK-NEXT: Uniform: [[VAL6:%vp[0-9]+]] = sext [[VAL3:%vp[0-9]+]]
; CHECK-NEXT: Uniform: [[VAL7:%vp[0-9]+]] = sext [[VAL5:%vp[0-9]+]]
; CHECK-LABEL: Basic Block: BB7
; CHECK-NEXT: Uniform: [[VAL8:%vp[0-9]+]] = phi i64 0 [[VAL:%vp[0-9]+]]
; CHECK-NEXT: Uniform: [[VAL10:%vp[0-9]+]] = add [[VAL8:%vp[0-9]+]] [[VAL7:%vp[0-9]+]]
; CHECK-NEXT: Uniform: [[VAL11:%vp[0-9]+]] = mul [[VAL10:%vp[0-9]+]] [[VAL12:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL13:%vp[0-9]+]] = add [[VAL11:%vp[0-9]+]] [[VAL1:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL14:%vp[0-9]+]] = getelementptr [[VAL15:%vp[0-9]+]] [[VAL13:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL16:%vp[0-9]+]] = load [[VAL14:%vp[0-9]+]]
; CHECK-NEXT: Uniform: [[VAL17:%vp[0-9]+]] = mul [[VAL8:%vp[0-9]+]] [[VAL12:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL18:%vp[0-9]+]] = add [[VAL17:%vp[0-9]+]] [[VAL1:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL19:%vp[0-9]+]] = getelementptr [[VAL15:%vp[0-9]+]] [[VAL18:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL20:%vp[0-9]+]] = load [[VAL19:%vp[0-9]+]]
; CHECK-NEXT: Divergent: [[VAL21:%vp[0-9]+]] = fadd [[VAL16:%vp[0-9]+]] [[VAL20:%vp[0-9]+]]
; CHECK-NEXT: Divergent: store [[VAL21:%vp[0-9]+]] [[VAL19:%vp[0-9]+]]
; CHECK-NEXT: Uniform: [[VAL9:%vp[0-9]+]] = add [[VAL8:%vp[0-9]+]] [[VAL6:%vp[0-9]+]]
; CHECK-NEXT: Uniform: [[VAL22:%vp[0-9]+]] = icmp [[VAL9:%vp[0-9]+]] [[VAL12:%vp[0-9]+]]
; CHECK-LABEL: Basic Block: BB8
; CHECK-NEXT: Uniform: [[VAL4:%vp[0-9]+]] = shl [[VAL3:%vp[0-9]+]] i32 1
; CHECK-NEXT: Uniform: [[VAL23:%vp[0-9]+]] = icmp [[VAL4:%vp[0-9]+]] [[VAL24:%vp[0-9]+]]
; CHECK-LABEL: Basic Block: BB5
; CHECK-NEXT: Divergent: [[VAL2:%vp[0-9]+]] = add [[VAL1:%vp[0-9]+]] i64 1
; CHECK-NEXT: Uniform: [[VAL25:%vp[0-9]+]] = icmp [[VAL2:%vp[0-9]+]] [[VAL26:%vp[0-9]+]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

define void @foo(double* nocapture %A, i32 %n) local_unnamed_addr {
entry:
  %cmp45 = icmp sgt i32 %n, 0
  br i1 %cmp45, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp242 = icmp sgt i32 %n, 2
  %0 = sext i32 %n to i64
  %wide.trip.count = zext i32 %n to i64
  br label %for.body.lr.ph2

for.body.lr.ph2:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.cond.cleanup3, %for.body.lr.ph2
  %indvars.iv53 = phi i64 [ 0, %for.body.lr.ph2 ], [ %indvars.iv.next54, %for.cond.cleanup3 ]
  br i1 %cmp242, label %for.body8.lr.ph.preheader, label %for.cond.cleanup3

for.body8.lr.ph.preheader:                        ; preds = %for.body
  br label %for.body8.lr.ph

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7, %for.body
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond = icmp eq i64 %indvars.iv.next54, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.body8.lr.ph:                                  ; preds = %for.body8.lr.ph.preheader, %for.cond.cleanup7
  %mul44 = phi i32 [ %mul, %for.cond.cleanup7 ], [ 2, %for.body8.lr.ph.preheader ]
  %len.043 = phi i32 [ %mul44, %for.cond.cleanup7 ], [ 1, %for.body8.lr.ph.preheader ]
  %1 = sext i32 %mul44 to i64
  %2 = sext i32 %len.043 to i64
  br label %for.body8

for.cond.cleanup7:                                ; preds = %for.body8
  %mul = shl nsw i32 %mul44, 1
  %cmp2 = icmp slt i32 %mul, %n
  br i1 %cmp2, label %for.body8.lr.ph, label %for.cond.cleanup3

for.body8:                                        ; preds = %for.body8.lr.ph, %for.body8
  %indvars.iv = phi i64 [ 0, %for.body8.lr.ph ], [ %indvars.iv.next, %for.body8 ]
  %3 = add nsw i64 %indvars.iv, %2
  %4 = mul nsw i64 %3, %0
  %5 = add nsw i64 %4, %indvars.iv53
  %arrayidx = getelementptr inbounds double, double* %A, i64 %5
  %6 = load double, double* %arrayidx, align 8
  %7 = mul nsw i64 %indvars.iv, %0
  %8 = add nsw i64 %7, %indvars.iv53
  %arrayidx14 = getelementptr inbounds double, double* %A, i64 %8
  %9 = load double, double* %arrayidx14, align 8
  %add15 = fadd double %6, %9
  store double %add15, double* %arrayidx14, align 8
  %indvars.iv.next = add i64 %indvars.iv, %1
  %cmp6 = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp6, label %for.body8, label %for.cond.cleanup7

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void
}
