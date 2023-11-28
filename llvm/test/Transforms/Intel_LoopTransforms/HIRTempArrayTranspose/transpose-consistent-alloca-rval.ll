; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-array-transpose,print<hir>" -hir-temp-array-transpose-allow-unknown-sizes -disable-output 2>&1 | FileCheck %s

; Check that we successfully transpose the loop with one constant size dim and one unknown dim.
; Previously this test failed due to verification for inconsistent Rval in the alloca.

; HIR before transformation

;     BEGIN REGION { }
;           + DO i1 = 0, %load + -1, 1   <DO_LOOP>  <MAX_TC_EST = 75>
;           |   + DO i2 = 0, %load + -1, 1   <DO_LOOP>
;           |   |   %phi120.in5 = 0.000000e+00;
;           |   |
;           |   |   + DO i3 = 0, %load + -1, 1   <DO_LOOP>  <MAX_TC_EST = 75>
;           |   |   |   %load124 = (%global.8)[i3][i1];
;           |   |   |   %load126 = (%A)[0][i3];
;           |   |   |   %phi120.in5 = 0.000000e+00;
;           |   |   + END LOOP
;           |   + END LOOP
;           + END LOOP
;     END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:    @llvm.stacksave.p0();
; CHECK:    %TranspTmpArr = alloca 600 * %load;
;
; CHECK:    + DO i1 = 0, 74, 1   <DO_LOOP>
; CHECK:    |   + DO i2 = 0, %load + -1, 1   <DO_LOOP>  <MAX_TC_EST = 75>
; CHECK:    |   |   (%TranspTmpArr)[i1][i2] = (%global.8)[i2][i1];
;           |   + END LOOP
;           + END LOOP
;
;
; CHECK:    + DO i1 = 0, %load + -1, 1   <DO_LOOP>  <MAX_TC_EST = 75>
; CHECK:    |   + DO i2 = 0, %load + -1, 1   <DO_LOOP>
;           |   |   %phi120.in5 = 0.000000e+00;
;           |   |
; CHECK:    |   |   + DO i3 = 0, %load + -1, 1   <DO_LOOP>  <MAX_TC_EST = 75>
; CHECK:    |   |   |   %load124 = (%TranspTmpArr)[i1][i3];
;           |   |   |   %load126 = (%A)[0][i3];
;           |   |   |   %phi120.in5 = 0.000000e+00;
;           |   |   + END LOOP
;           |   + END LOOP
;           + END LOOP
;
;           @llvm.stackrestore.p0(&((%call22)[0]));
;     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @wombat(i64 %load, ptr %global.8, ptr %A) {
bb64:                                             ; preds = %bb45, %bb1
  br i1 false, label %bb101, label %bb356

bb101:                                            ; preds = %bb64
  %add107 = add i64 %load, 1
  br label %bb111

bb111:                                            ; preds = %bb135, %bb101
  %phi112 = phi i64 [ %add140, %bb135 ], [ 1, %bb101 ]
  br label %bb114

bb114:                                            ; preds = %bb131, %bb111
  %phi115 = phi i64 [ %add133, %bb131 ], [ 1, %bb111 ]
  %call118 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 600, ptr elementtype(double) %A, i64 0)
  br label %bb119

bb119:                                            ; preds = %bb119, %bb114
  %phi120 = phi double [ 0.000000e+00, %bb119 ], [ 0.000000e+00, %bb114 ]
  %phi121 = phi i64 [ %add129, %bb119 ], [ 1, %bb114 ]
  %call122 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 600, ptr elementtype(double) %global.8, i64 %phi121)
  %call123 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %call122, i64 %phi112)
  %load124 = load double, ptr %call123, align 8
  %call125 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %call118, i64 %phi121)
  %load126 = load double, ptr %call125, align 8
  %add129 = add i64 %phi121, 1
  %icmp130 = icmp eq i64 %add129, %add107
  br i1 %icmp130, label %bb131, label %bb119

bb131:                                            ; preds = %bb119
  %add133 = add i64 %phi115, 1
  %icmp134 = icmp eq i64 %add133, %add107
  br i1 %icmp134, label %bb135, label %bb114

bb135:                                            ; preds = %bb131
  %add140 = add i64 %phi112, 1
  %icmp141 = icmp eq i64 %add140, %add107
  br i1 %icmp141, label %bb144, label %bb111

bb144:                                            ; preds = %bb135
  br label %bb356

bb356:                                            ; preds = %bb321, %bb229
  ret void
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
