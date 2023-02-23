; REQUIRES: asserts
; RUN: opt %s -passes='hir-ssa-deconstruction,hir-temp-array-transpose' -print-after=hir-temp-array-transpose -debug-only=hir-temp-array-transpose -disable-output -hir-details-dims 2>&1 | FileCheck %s

; Check that we bail out for mismatch between %2 UBCE and the stride CEs for @nsarh_mp_wty_
; Candidate transpose ref. Were transpose to happen, the Innerloop TC would have become
; the dimsize for the inner dimension, which would then be used for the stride size
; of the outer dim. %2 is of i32 type while all the subscript indices are i64 type, which
; triggers an assert later if we do not bailout or convert the blob.

; CHECK: [Illegal] Type Mismatch between InnerLoop CE and Dim2.

; HIR before Transformation
; CHECK: BEGIN REGION
; CHECK-NOT: modified
;             + DO i1 = 0, zext.i32.i64(%29) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;             |   + DO i2 = 0, sext.i32.i64(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;             |   |   + DO i3 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
;             |   |   |   %49 = (null)[0:0:432(ptr:0)][0:i1:8(ptr:54)];
;             |   |   |
;             |   |   |   + DO i32 i4 = 0, %2, 1   <DO_LOOP>
;             |   |   |   | <RVAL-REG> LINEAR i32 %2 {sb:13}
;             |   |   |   |   %54 = (@nsarh_mp_wty_)[1:i4:432(ptr:0)][0:i2:8(ptr:54)];
;             |   |   |   |   <LVAL-REG> NON-LINEAR double %54 {sb:18}
;             |   |   |   |   <RVAL-REG> {al:8}(LINEAR ptr @nsarh_mp_wty_)[i64 1:LINEAR sext.i32.i64(i4):i64 432(ptr:0)][i64 0:LINEAR i64 i2:i64 8(ptr:54)] inbounds  {sb:28}
;             |   |   |   |      <BLOB> LINEAR ptr @nsarh_mp_wty_ {sb:17}
;             |   |   |   + END LOOP
;             |   |   + END LOOP
;             |   + END LOOP
;             + END LOOP
;       END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@nsarh_mp_wty_ = external global [54 x [54 x double]]

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define fastcc void @farhim_() #1 {
  %1 = load i32, ptr null, align 8
  %2 = load i32, ptr null, align 8
  %3 = sext i32 0 to i64
  %4 = shl i64 0, 0
  %5 = icmp slt i32 0, 0
  br label %25

6:                                                ; No predecessors!
  %7 = zext i32 0 to i64
  %8 = icmp slt i32 0, 0
  %9 = add i64 0, 0
  %10 = add i64 0, 0
  br label %20

11:                                               ; preds = %22, %11
  %12 = phi i64 [ 0, %22 ], [ 0, %11 ]
  %13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %14 = add i64 0, 0
  %15 = icmp eq i64 0, 0
  br label %11

16:                                               ; No predecessors!
  br label %17

17:                                               ; preds = %20, %16
  %18 = add i64 0, 0
  %19 = icmp eq i64 0, 0
  br label %20

20:                                               ; preds = %17, %6
  %21 = phi i64 [ 0, %6 ], [ 0, %17 ]
  br label %17

22:                                               ; No predecessors!
  %23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  br label %11

24:                                               ; No predecessors!
  br label %25

25:                                               ; preds = %24, %0
  %26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %29 = load i32, ptr null, align 8
  %30 = icmp slt i32 0, 0
  br label %31

31:                                               ; preds = %25
  %32 = load i32, ptr null, align 8
  %33 = add nsw i32 %2, 1
  %34 = add nsw i32 %1, 1
  %35 = add nsw i32 %32, 1
  %36 = add nuw i32 %29, 1
  %37 = zext i32 %36 to i64
  %38 = sext i32 %35 to i64
  %39 = sext i32 %34 to i64
  %40 = sext i32 %33 to i64
  br label %41

41:                                               ; preds = %63, %31
  %42 = phi i64 [ 1, %31 ], [ %64, %63 ]
  br label %43

43:                                               ; preds = %60, %41
  %44 = phi i64 [ 1, %41 ], [ %61, %60 ]
  br label %45

45:                                               ; preds = %57, %43
  %46 = phi i64 [ 1, %43 ], [ %58, %57 ]
  %47 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 432, ptr elementtype(double) null, i64 0)
  %48 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %47, i64 %42)
  %49 = load double, ptr %48, align 8
  br label %50

50:                                               ; preds = %50, %45
  %51 = phi i32 [ 0, %45 ], [ %55, %50 ]
  %ext = sext i32 %51 to i64
  %52 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 432, ptr elementtype(double) @nsarh_mp_wty_, i64 %ext)
  %53 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %52, i64 %44)
  %54 = load double, ptr %53, align 8
  %55 = add i32 %51, 1
  %56 = icmp eq i32 %55, %33
  br i1 %56, label %57, label %50

57:                                               ; preds = %50
  %58 = add i64 %46, 1
  %59 = icmp eq i64 %58, %39
  br i1 %59, label %60, label %45

60:                                               ; preds = %57
  %61 = add i64 %44, 1
  %62 = icmp eq i64 %61, %38
  br i1 %62, label %63, label %43

63:                                               ; preds = %60
  %64 = add i64 %42, 1
  %65 = icmp eq i64 %64, %37
  br i1 %65, label %66, label %41

66:                                               ; preds = %63
  ret void
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { "intel-lang"="fortran" }
