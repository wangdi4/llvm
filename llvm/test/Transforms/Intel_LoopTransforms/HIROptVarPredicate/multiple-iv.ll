
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>,hir-cg" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the analysis process doesn't produce an
; assertion when analyzing an If (instruction <15>) condition inside
; a nested loop, and the condition use the IV of the inner loop.

; HIR before transformation

; <0>    BEGIN REGION { }
; <44>         + DO i1 = 0, -1 * %load.b + %load.a, 1   <DO_LOOP>
; <10>         |      %phi.max = 0.000000e+00;
; <45>         |   + DO i2 = 0, -1 * i1 + -1 * %load.b + %load.a + -1, 1   <DO_LOOP>
; <13>         |   |   %phi.max.out = %phi.max;
; <15>         |   |   if (i1 + i2 + 1 >u -1 * %load.b + %load.a)
; <15>         |   |   {
; <20>         |   |      %load.c = (%c)[%n + -1];
; <21>         |   |      %abs.48 = @llvm.fabs.f64(%load.c);
; <22>         |   |      %rel.14 = @llvm.experimental.constrained.fcmp.f64(%abs.48,  %phi.max.out,  !"ogt",  !"fpexcept.strict");
; <23>         |   |      %phi.max = (%rel.14 != 0) ? %abs.48 : %phi.max;
; <15>         |   |   }
; <45>         |   + END LOOP
; <44>         + END LOOP
; <0>    END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, -1 * %load.b + %load.a, 1   <DO_LOOP>
; CHECK:       |      %phi.max = 0.000000e+00;
; CHECK:       |   + DO i2 = 0, -1 * i1 + -1 * %load.b + %load.a + -1, 1   <DO_LOOP>
; CHECK:       |   |   %phi.max.out = %phi.max;
; CHECK:       |   |   if (i1 + i2 + 1 >u -1 * %load.b + %load.a)
; CHECK:       |   |   {
; CHECK:       |   |      %load.c = (%c)[%n + -1];
; CHECK:       |   |      %abs.48 = @llvm.fabs.f64(%load.c);
; CHECK:       |   |      %rel.14 = @llvm.experimental.constrained.fcmp.f64(%abs.48,  %phi.max.out,  !"ogt",  !"fpexcept.strict");
; CHECK:       |   |      %phi.max = (%rel.14 != 0) ? %abs.48 : %phi.max;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION


;Module Before HIR
; ModuleID = '/tmp/ifx0896834780xyofHP/ifxiUmZcE.bc'
source_filename = "/tmp/ifx0896834780xyofHP/ifxiUmZcE.bc"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"


define void @orien_(ptr noalias nocapture readonly %a, ptr noalias nocapture readonly %b, ptr noalias nocapture readonly %c, i64 %n, i64 %t) {
do.body38.preheader:
  %load.a = load i64, ptr %a, align 1
  %sub.b = tail call ptr @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(i64) %b, i64 %t)
  %load.b = load i64, ptr %sub.b, align 1
  %rel.9 = icmp slt i64 %load.a, %load.b
  %temp1 = sub i64 %load.a, %load.b
  %temp2 = add i64 %load.a, 2
  %temp3 = sub i64 %temp2, %load.b
  br label %do.body38

do.body38:                                        ; preds = %do.body38.preheader, %do.end_do45
  %indvars.iv = phi i64 [ 1, %do.body38.preheader ], [ %indvars.iv.next, %do.end_do45 ]
  %phi.b = phi i64 [ %load.b, %do.body38.preheader ], [ %add.5, %do.end_do45 ]
  %add.5 = add nsw i64 %phi.b, 1
  %rel.10.not = icmp sgt i64 %load.a, %phi.b
  br i1 %rel.10.not, label %do.body48.preheader, label %do.end_do45

do.body48.preheader:                              ; preds = %do.body38
  br label %do.body48

do.body48:                                        ; preds = %do.body48.preheader, %bb2
  %indvars.iv48 = phi i64 [ %indvars.iv.next49, %bb2 ], [ %indvars.iv, %do.body48.preheader ]
  %phi.add = phi i64 [ %add.7, %bb2 ], [ %add.5, %do.body48.preheader ]
  %phi.max = phi double [ %phi.select, %bb2 ], [ 0.000000e+00, %do.body48.preheader ]
  %.not = icmp ugt i64 %indvars.iv48, %temp1
  br i1 %.not, label %do.end_do49, label %bb2

do.end_do49:                                      ; preds = %do.body48
  %sub.c = tail call ptr @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %c, i64 %n)
  %load.c = load double, ptr %sub.c, align 1
  %abs.48 = tail call double @llvm.fabs.f64(double %load.c)
  %rel.14 = tail call i1 @llvm.experimental.constrained.fcmp.f64(double %abs.48, double %phi.max, metadata !"ogt", metadata !"fpexcept.strict")
  %spec.select = select i1 %rel.14, double %abs.48, double %phi.max
  br label %bb2

bb2:                                              ; preds = %do.body48, %do.end_do49
  %phi.select = phi double [ %spec.select, %do.end_do49 ], [ %phi.max, %do.body48 ]
  %add.7 = add nsw i64 %phi.add, 1
  %rel.15.not.not = icmp slt i64 %phi.add, %load.a
  %indvars.iv.next49 = add i64 %indvars.iv48, 1
  br i1 %rel.15.not.not, label %do.body48, label %do.end_do45.loopexit

do.end_do45.loopexit:                             ; preds = %bb2
  br label %do.end_do45

do.end_do45:                                      ; preds = %do.end_do45.loopexit, %do.body38
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %temp3
  br i1 %exitcond.not, label %do.end_do32.loopexit, label %do.body38

do.end_do32.loopexit:                             ; preds = %do.end_do45
  br label %do.end_do32

do.end_do32:                                      ; preds = %do.end_do32.loopexit
  ret void
}

declare ptr @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8, i64, i64, ptr, i64)

declare ptr @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, ptr, i64)

declare double @llvm.fabs.f64(double)

declare i1 @llvm.experimental.constrained.fcmp.f64(double, double, metadata, metadata)