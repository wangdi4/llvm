; RUN: opt %s -passes='hir-ssa-deconstruction,hir-temp-array-transpose' -print-after=hir-temp-array-transpose -disable-output -hir-temp-array-transpose-allow-unknown-sizes -hir-details-dims 2>&1 | FileCheck %s

; Check that we successfully transpose the refs with one dim as constant size and the
; other using the Loop UB. Candidate @nsarh_mp_qyt_ has a known inner dimsize and uses
; the constant value in the alloca and copy loop, which the other dimension is based
; on the loop tc %2.

; HIR Before:
;        BEGIN REGION { }
;              + DO i1 = 0, zext.i32.i64(%29) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;              |   + DO i2 = 0, sext.i32.i64(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;...
;              |   |   %hir.de.ssa.copy5.in5 = 0.000000e+00;
;              |   |
;              |   |   + DO i3 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
;              |   |   |   %88 = (null)[0][i1];
;              |   |   |   %hir.de.ssa.copy7.in = 0.000000e+00;
;              |   |   |
;              |   |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
;              |   |   |   |   %94 = (%eht_mp_bt_)[0:i4:0(double:0)][0:i3:0(double:0)];
;              |   |   |   |   %97 = (%nsarh_mp_qyt_)[0:i4:432(double:0)][0:i2:8(double:54)];
;              |   |   |   |   %hir.de.ssa.copy7.in = 0.000000e+00;
;              |   |   |   + END LOOP
;              |   |   |
;              |   |   |   %hir.de.ssa.copy5.in5 = 0.000000e+00;
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;        END REGION


; CHECK: BEGIN REGION { modified }
;              %call = @llvm.stacksave.p0();
; CHECK:       %TranspTmpArr = alloca 8 * (sext.i32.i64(%1) * sext.i32.i64(%2));
;
; CHECK:       + DO i1 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
; CHECK:       |   |   (%TranspTmpArr)[0:i1:0(double:0)][0:i2:0(double:0)] = (%eht_mp_bt_)[0:i2:0(double:0)][0:i1:0(double:0)];
;              |   + END LOOP
;              + END LOOP
;
;              %call8 = @llvm.stacksave.p0();
; CHECK:       %TranspTmpArr9 = alloca 432 * sext.i32.i64(%2);
;
; CHECK:       + DO i1 = 0, 53, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
; CHECK:       |   |   (%TranspTmpArr9)[0:i1:8 * sext.i32.i64(%2)(double:0)][0:i2:8(double:0)] = (%nsarh_mp_qyt_)[0:i2:432(double:0)][0:i1:8(double:54)];
;              |   + END LOOP
;              + END LOOP
;
;              + DO i1 = 0, zext.i32.i64(%29) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;              |   + DO i2 = 0, sext.i32.i64(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;              |   |   + DO i3 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
;              |   |   |   %88 = (null)[0][i1];
;              |   |   |   %hir.de.ssa.copy7.in = 0.000000e+00;
;              |   |   |
;              |   |   |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
; CHECK:       |   |   |   |   %94 = (%TranspTmpArr)[0:i3:0(double:0)][0:i4:0(double:0)];
; CHECK:       |   |   |   |   %97 = (%TranspTmpArr9)[0:i2:8 * sext.i32.i64(%2)(double:0)][0:i4:8(double:0)];
;              |   |   |   |   %hir.de.ssa.copy7.in = 0.000000e+00;
;              |   |   |   + END LOOP
;              |   |   |
;              |   |   |   %hir.de.ssa.copy5.in5 = 0.000000e+00;
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;              @llvm.stackrestore.p0(&((%call8)[0]));
;              @llvm.stackrestore.p0(&((%call)[0]));
;        END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define fastcc void @farhim_(ptr %eht_mp_bt_, ptr %nsarh_mp_qyt_) #1 {
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
  %33 = icmp slt i32 0, 0
  %34 = icmp slt i32 0, 0
  %35 = load double, ptr null, align 8
  %36 = add nsw i32 %2, 1
  %37 = sext i32 0 to i64
  %38 = add nsw i32 %1, 1
  %39 = add nsw i32 %32, 1
  %40 = sext i32 0 to i64
  %41 = add nuw i32 %29, 1
  %42 = zext i32 %41 to i64
  %43 = sext i32 %39 to i64
  %44 = sext i32 %38 to i64
  %45 = sext i32 %36 to i64
  br label %46

46:                                               ; preds = %107, %31
  %47 = phi i64 [ 1, %31 ], [ %108, %107 ]
  br label %48

48:                                               ; preds = %46
  %49 = add i64 0, 0
  %50 = mul i64 0, 0
  br label %51

51:                                               ; preds = %104, %48
  %52 = phi i64 [ 1, %48 ], [ %105, %104 ]
  br label %55

53:                                               ; No predecessors!
  %54 = add i64 0, 0
  br label %104

55:                                               ; preds = %51
  %56 = add i64 0, 0
  br label %57

57:                                               ; preds = %78, %55
  %58 = phi i64 [ 0, %55 ], [ 0, %78 ]
  br label %59

59:                                               ; preds = %57
  %60 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %62 = load double, ptr null, align 8
  %63 = add i64 0, 0
  %64 = mul i64 0, 0
  br label %65

65:                                               ; preds = %65, %59
  %66 = phi i64 [ 0, %59 ], [ 0, %65 ]
  %67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %69 = load double, ptr null, align 8
  %70 = fmul double 0.000000e+00, 0.000000e+00
  %71 = fmul double 0.000000e+00, 0.000000e+00
  %72 = add i64 0, 0
  %73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %74 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr elementtype(double) null, i64 0)
  %75 = add i64 0, 0
  %76 = icmp eq i64 0, 0
  br i1 %76, label %77, label %65

77:                                               ; preds = %65
  br label %78

78:                                               ; preds = %77
  %79 = add i64 0, 0
  %80 = icmp eq i64 0, 0
  br i1 %80, label %81, label %57

81:                                               ; preds = %78
  br label %82

82:                                               ; preds = %100, %81
  %83 = phi i64 [ %101, %100 ], [ 1, %81 ]
  %84 = phi double [ 0.000000e+00, %100 ], [ 0.000000e+00, %81 ]
  br label %85

85:                                               ; preds = %82
  %86 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 432, ptr elementtype(double) null, i64 0)
  %87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %86, i64 %47)
  %88 = load double, ptr %87, align 8
  br label %89

89:                                               ; preds = %89, %85
  %90 = phi i64 [ 1, %85 ], [ %98, %89 ]
  %91 = phi double [ 0.000000e+00, %85 ], [ 0.000000e+00, %89 ]
  %92 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 0, ptr elementtype(double) %eht_mp_bt_, i64 %90)
  %93 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr elementtype(double) %92, i64 %83)
  %94 = load double, ptr %93, align 8
  %95 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 432, ptr elementtype(double) %nsarh_mp_qyt_, i64 %90)
  %96 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %95, i64 %52)
  %97 = load double, ptr %96, align 8
  %98 = add i64 %90, 1
  %99 = icmp eq i64 %98, %45
  br i1 %99, label %100, label %89

100:                                              ; preds = %89
  %101 = add i64 %83, 1
  %102 = icmp eq i64 %101, %44
  br i1 %102, label %103, label %82

103:                                              ; preds = %100
  br label %104

104:                                              ; preds = %103, %53
  %105 = add i64 %52, 1
  %106 = icmp eq i64 %105, %43
  br i1 %106, label %107, label %51

107:                                              ; preds = %104
  %108 = add i64 %47, 1
  %109 = icmp eq i64 %108, %42
  br i1 %109, label %110, label %46

110:                                              ; preds = %107
  ret void
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { "intel-lang"="fortran" }
