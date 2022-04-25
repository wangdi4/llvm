; RUN: opt < %s -hir-ssa-deconstruction -print-after=hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print,print<hir>" 2>&1 | FileCheck %s

; Verify that the max trip count estimate of i3 loop is set as a sane value by
; taking into account that its trip count depends on (-1 * i1).

; Previously it was set as 1 by analyzing the subscript [i1 + i3 + 1], deducing
; that the max value of the component (i1 + 1) is 249 and so max value of i3 is
; 250 - 249 = 1 where 250 is the number of elements in the dimension.

; After the fix, whenever the analysis can deduce negative IV relationship, it
; uses the avg of min and max values of (i1 + 1). This is okay because MAX_TC_EST
; is used for cost modelling and the average reflects a better number than using
; the min value which would 'technically' be more correct.


; CHECK: + DO i1 = 0, 248, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, -1 * i1 + 248, 1   <DO_LOOP>  <MAX_TC_EST = 249>
; CHECK: |   |   %i523 = @llvm.smax.i64(i2 + 1,  2);
; CHECK: |   |   %i527 = (%i14)[i1 + i2 + 1][i1];
; CHECK: |   |   %i547 = 0.000000e+00;
; CHECK: |   |   if (i1 + i2 + 2 != i1 + 2)
; CHECK: |   |   {
; CHECK: |   |      %i532 = 0.000000e+00;
; CHECK: |   |
; CHECK: |   |      + DO i3 = 0, %i523 + -2, 1   <DO_LOOP>  <MAX_TC_EST = 125>
; CHECK: |   |      |   %i536 = (%i11)[0][i3];
; CHECK: |   |      |   %i538 = (%i14)[i1 + i2 + 1][i1 + i3 + 1];
; CHECK: |   |      |   %i539 = %i538  *  %i536;
; CHECK: |   |      |   %i532 = %i539  +  %i532;
; CHECK: |   |      + END LOOP
; CHECK: |   |
; CHECK: |   |      %i547 = %i532;
; CHECK: |   |   }
; CHECK: |   |   %i548 = %i547  +  %i527;
; CHECK: |   |   %i549 =  - %i548;
; CHECK: |   |   (%i14)[i1 + i2 + 1][i1] = %i549;
; CHECK: |   |   (%i11)[0][i2] = %i549;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

define void @foo() {
entry:
  %i11 = alloca [250 x double], align 8
  %i12 = alloca [62500 x double], align 8
  %i14 = getelementptr inbounds [62500 x double], [62500 x double]* %i12, i64 0, i64 0
  %i30 = getelementptr inbounds [250 x double], [250 x double]* %i11, i64 0, i64 0
  br label %bb516

bb516:                                            ; preds = %bb554, %entry
  %i517 = phi i64 [ %i555, %bb554 ], [ 2, %entry ]
  %i518 = phi i64 [ %i519, %bb554 ], [ 1, %entry ]
  %i519 = add nuw nsw i64 %i518, 1
  br label %bb520

bb520:                                            ; preds = %bb546, %bb516
  %i521 = phi i64 [ %i551, %bb546 ], [ %i517, %bb516 ]
  %i522 = phi i64 [ %i552, %bb546 ], [ 1, %bb516 ]
  %i523 = tail call i64 @llvm.smax.i64(i64 %i522, i64 2)
  %i524 = add nuw nsw i64 %i518, %i523
  %i525 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 2000, double* elementtype(double) nonnull %i14, i64 %i521)
  %i526 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %i525, i64 %i518)
  %i527 = load double, double* %i526, align 1
  %i528 = sub nuw nsw i64 %i521, %i518
  %i529 = icmp eq i64 %i521, %i519
  br i1 %i529, label %bb546, label %bb530

bb530:                                            ; preds = %bb520
  br label %bb531

bb531:                                            ; preds = %bb531, %bb530
  %i532 = phi double [ %i540, %bb531 ], [ 0.000000e+00, %bb530 ]
  %i533 = phi i64 [ %i541, %bb531 ], [ %i519, %bb530 ]
  %i534 = phi i64 [ %i542, %bb531 ], [ 1, %bb530 ]
  %i535 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %i30, i64 %i534)
  %i536 = load double, double* %i535, align 1
  %i537 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %i525, i64 %i533)
  %i538 = load double, double* %i537, align 1
  %i539 = fmul fast double %i538, %i536
  %i540 = fadd fast double %i539, %i532
  %i541 = add nuw nsw i64 %i533, 1
  %i542 = add nuw nsw i64 %i534, 1
  %i543 = icmp eq i64 %i541, %i524
  br i1 %i543, label %bb544, label %bb531

bb544:                                            ; preds = %bb531
  %i545 = phi double [ %i540, %bb531 ]
  br label %bb546

bb546:                                            ; preds = %bb544, %bb520
  %i547 = phi double [ 0.000000e+00, %bb520 ], [ %i545, %bb544 ]
  %i548 = fadd fast double %i547, %i527
  %i549 = fneg fast double %i548
  store double %i549, double* %i526, align 1
  %i550 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %i30, i64 %i528)
  store double %i549, double* %i550, align 1
  %i551 = add nuw nsw i64 %i521, 1
  %i552 = add nuw nsw i64 %i522, 1
  %i553 = icmp eq i64 %i551, 251
  br i1 %i553, label %bb554, label %bb520

bb554:                                            ; preds = %bb546
  %i555 = add nuw nsw i64 %i517, 1
  %i556 = icmp eq i64 %i555, 251
  br i1 %i556, label %bb557, label %bb516

bb557:
  ret void
}

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

declare i64 @llvm.smax.i64(i64, i64) #1

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
