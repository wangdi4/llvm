; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-unroll-and-jam -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s

; Verify that we rename the liveout temp %t101 defined in innermost loop with unroll & jam.

; CHECK: Function: foo

; CHECK: + DO i1 = 0, 255, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 255, 1   <DO_LOOP>
; CHECK: |   |   %row_sum.037.i = 0.000000e+00;
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, 255, 1   <DO_LOOP>
; CHECK: |   |   |   %t101 = @llvm.fma.f64((@_ZZ4mainE9first_ref)[0][i1 + 256 * i3],  (@_ZZ4mainE10second_ref)[0][256 * i2 + i3],  %row_sum.037.i);
; CHECK: |   |   |   %row_sum.037.i = %t101;
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   (@_ZZ4mainE11product_ref)[0][i1 + 256 * i2] = %t101;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Function: foo

; CHECK: BEGIN REGION { modified }

; CHECK: + DO i1 = 0, 63, 1   <DO_LOOP>

; CHECK: |   + DO i2 = 0, 63, 1   <DO_LOOP>

; CHECK: |   |   + DO i3 = 0, 255, 1   <DO_LOOP>
; CHECK: |   |   |   %temp21 = @llvm.fma.f64((@_ZZ4mainE9first_ref)[0][4 * i1 + 256 * i3],  (@_ZZ4mainE10second_ref)[0][1024 * i2 + i3],  %temp9);
; CHECK: |   |   |   %temp9 = %temp21;

; CHECK: |   |   + END LOOP

; CHECK: |   |   (@_ZZ4mainE11product_ref)[0][4 * i1 + 1024 * i2] = %temp21;


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZZ4mainE9first_ref = internal global [65536 x double] zeroinitializer, align 16
@_ZZ4mainE10second_ref = internal global [65536 x double] zeroinitializer, align 16
@_ZZ4mainE11product_ref = internal global [65536 x double] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.cond1.preheader.i433

for.cond1.preheader.i433:                         ; preds = %for.cond.cleanup3.i434, %entry
  %indvars.iv50.i = phi i64 [ 0, %entry ], [ %indvars.iv.next51.i, %for.cond.cleanup3.i434 ]
  br label %for.cond5.preheader.i

for.cond5.preheader.i:                            ; preds = %for.cond.cleanup7.i, %for.cond1.preheader.i433
  %indvars.iv44.i = phi i64 [ 0, %for.cond1.preheader.i433 ], [ %indvars.iv.next45.i, %for.cond.cleanup7.i ]
  %t94 = shl i64 %indvars.iv44.i, 8
  br label %for.body8.i

for.body8.i:                                      ; preds = %for.body8.i, %for.cond5.preheader.i
  %indvars.iv.i435 = phi i64 [ 0, %for.cond5.preheader.i ], [ %indvars.iv.next.i437, %for.body8.i ]
  %row_sum.037.i = phi double [ 0.000000e+00, %for.cond5.preheader.i ], [ %t101, %for.body8.i ]
  %t96 = shl i64 %indvars.iv.i435, 8
  %t97 = add nuw nsw i64 %t96, %indvars.iv50.i
  %arrayidx.i436 = getelementptr inbounds [65536 x double], [65536 x double]* @_ZZ4mainE9first_ref, i64 0, i64 %t97
  %t98 = load double, double* %arrayidx.i436, align 8
  %t99 = add nuw nsw i64 %indvars.iv.i435, %t94
  %arrayidx12.i = getelementptr inbounds [65536 x double], [65536 x double]* @_ZZ4mainE10second_ref, i64 0, i64 %t99
  %t100 = load double, double* %arrayidx12.i, align 8
  %t101 = call double @llvm.fma.f64(double %t98, double %t100, double %row_sum.037.i) #16
  %indvars.iv.next.i437 = add nuw nsw i64 %indvars.iv.i435, 1
  %exitcond.i438 = icmp eq i64 %indvars.iv.next.i437, 256
  br i1 %exitcond.i438, label %for.cond.cleanup7.i, label %for.body8.i

for.cond.cleanup7.i:                              ; preds = %for.body8.i
  %.lcssa = phi double [ %t101, %for.body8.i ]
  %t95 = add nuw nsw i64 %t94, %indvars.iv50.i
  %arrayidx16.i = getelementptr inbounds [65536 x double], [65536 x double]* @_ZZ4mainE11product_ref, i64 0, i64 %t95
  store double %.lcssa, double* %arrayidx16.i, align 8
  %indvars.iv.next45.i = add nuw nsw i64 %indvars.iv44.i, 1
  %exitcond49.i = icmp eq i64 %indvars.iv.next45.i, 256
  br i1 %exitcond49.i, label %for.cond.cleanup3.i434, label %for.cond5.preheader.i

for.cond.cleanup3.i434:                           ; preds = %for.cond.cleanup7.i
  %indvars.iv.next51.i = add nuw nsw i64 %indvars.iv50.i, 1
  %exitcond52.i = icmp eq i64 %indvars.iv.next51.i, 256
  br i1 %exitcond52.i, label %for.cond1.preheader.i421.preheader, label %for.cond1.preheader.i433

for.cond1.preheader.i421.preheader:
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double @llvm.fma.f64(double, double, double)


