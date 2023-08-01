; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that the upper of i3 loop is reverse engineered in terms of smax intrinsic %max.

; CHECK: + DO i1 = 0, %t590 + -3, 1   <DO_LOOP>
; CHECK: |   if (i1 + 1 < %t17)
; CHECK: |   {
; CHECK: |      + DO i2 = 0, -1 * i1 + %t590 + -3, 1   <DO_LOOP>
; CHECK: |      |   %max = @llvm.smax.i64(i2 + 1,  2);
; CHECK: |      |   %t604 = (%t24)[i1 + i2 + 1][i1];
; CHECK: |      |   %t624 = 0.000000e+00;
; CHECK: |      |   if (i1 + i2 + 2 != i1 + 2)
; CHECK: |      |   {
; CHECK: |      |      %t609 = 0.000000e+00;
; CHECK: |      |
; CHECK: |      |      + DO i3 = 0, %max + -2, 1   <DO_LOOP>
; CHECK: |      |      |   %t613 = (%t21)[i3];
; CHECK: |      |      |   %t615 = (%t24)[i1 + i2 + 1][i1 + i3 + 1];
; CHECK: |      |      |   %t616 = %t615  *  %t613;
; CHECK: |      |      |   %t609 = %t616  +  %t609;
; CHECK: |      |      + END LOOP
; CHECK: |      |
; CHECK: |      |      %t624 = %t609;
; CHECK: |      |   }
; CHECK: |      |   %t625 = %t624  +  %t604;
; CHECK: |      |   %t626 =  - %t625;
; CHECK: |      |   (%t24)[i1 + i2 + 1][i1] = %t626;
; CHECK: |      |   (%t21)[i2] = %t626;
; CHECK: |      + END LOOP
; CHECK: |   }
; CHECK: + END LOOP


define void @foo(i64 %t17, ptr %t21, ptr %t24, i64 %t26, i64 %t590) {
entry:
  br label %loop1

loop1:                                              ; preds = %latch1, %entry
  %iv1 = phi i64 [ 2, %entry ], [ %iv1.inc, %latch1 ]
  %iv11 = phi i64 [ 1, %entry ], [ %iv11.inc, %latch1 ]
  %iv11.inc = add nuw i64 %iv11, 1
  %t595 = icmp slt i64 %iv11, %t17
  br i1 %t595, label %loop2.pre, label %latch1

loop2.pre:                                              ; preds = %loop1
  br label %loop2

loop2:                                              ; preds = %latch2, %loop2.pre
  %iv2 = phi i64 [ %iv2.inc, %latch2 ], [ %iv1, %loop2.pre ]
  %iv22 = phi i64 [ %iv22.inc, %latch2 ], [ 1, %loop2.pre ]
  %max = tail call i64 @llvm.smax.i64(i64 %iv22, i64 2)
  %t601 = add nuw nsw i64 %iv11, %max
  %t602 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %t26, ptr elementtype(double) nonnull %t24, i64 %iv2)
  %t603 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %t602, i64 %iv11)
  %t604 = load double, ptr %t603, align 1
  %t605 = sub nuw nsw i64 %iv2, %iv11
  %t606 = icmp eq i64 %iv2, %iv11.inc
  br i1 %t606, label %latch2, label %loop3.pre

loop3.pre:                                              ; preds = %loop2
  br label %loop3

loop3:                                              ; preds = %loop3, %loop3.pre
  %t609 = phi double [ %t617, %loop3 ], [ 0.000000e+00, %loop3.pre ]
  %iv3 = phi i64 [ %iv3.inc, %loop3 ], [ %iv11.inc, %loop3.pre ]
  %iv33 = phi i64 [ %iv33.inc, %loop3 ], [ 1, %loop3.pre ]
  %t612 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %t21, i64 %iv33)
  %t613 = load double, ptr %t612, align 1
  %t614 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %t602, i64 %iv3)
  %t615 = load double, ptr %t614, align 1
  %t616 = fmul fast double %t615, %t613
  %t617 = fadd fast double %t616, %t609
  %iv3.inc = add nuw nsw i64 %iv3, 1
  %iv33.inc = add nuw nsw i64 %iv33, 1
  %t620 = icmp eq i64 %iv3.inc, %t601
  br i1 %t620, label %exit3, label %loop3

exit3:                                              ; preds = %loop3
  %t622 = phi double [ %t617, %loop3 ]
  br label %latch2

latch2:                                              ; preds = %exit3, %loop2
  %t624 = phi double [ 0.000000e+00, %loop2 ], [ %t622, %exit3 ]
  %t625 = fadd fast double %t624, %t604
  %t626 = fneg fast double %t625
  store double %t626, ptr %t603, align 1
  %t627 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %t21, i64 %t605)
  store double %t626, ptr %t627, align 1
  %iv2.inc = add nuw nsw i64 %iv2, 1
  %iv22.inc = add nuw nsw i64 %iv22, 1
  %t630 = icmp eq i64 %iv2.inc, %t590
  br i1 %t630, label %exit2, label %loop2

exit2:                                              ; preds = %latch2
  br label %latch1

latch1:                                              ; preds = %exit2, %loop1
  %iv1.inc = add nuw nsw i64 %iv1, 1
  %t634 = icmp eq i64 %iv1.inc, %t590
  br i1 %t634, label %exit1, label %loop1

exit1:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0


declare i64 @llvm.smax.i64(i64, i64) #1

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
