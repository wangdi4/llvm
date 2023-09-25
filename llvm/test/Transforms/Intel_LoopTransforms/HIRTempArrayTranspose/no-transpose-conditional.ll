; REQUIRES: asserts
; RUN: opt %s -passes='hir-ssa-deconstruction,hir-temp-array-transpose' -print-after=hir-temp-array-transpose -debug-only=hir-temp-array-transpose -disable-output -hir-temp-array-transpose-allow-unknown-sizes 2>&1 | FileCheck %s

; Check that transpose fails due to the loop corresponding to the unknown dim being
; inside an IF statement. All potential transpose candidates have i4 as the outer dim
; index. In order to create the alloca, i4 Loop TC would be required to allocate and then
; used to create the copy loop. We bailout as unconditionally accessing the loop
; is unsafe behavior.

; HIR Before
;           BEGIN REGION { }
;           + DO i1 = 0, zext.i32.i64(%29) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;           |   + DO i2 = 0, sext.i32.i64(%32) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 54>
;           |   |   if (i2 + 1 != 10)
;           |   |   {
;           |   |      + DO i3 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
;           |   |      |   %62 = (null)[0:0:8(ptr:0)];
;           |   |      |
;           |   |      |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
;           |   |      |   |   %69 = (@nsarh_mp_wty_)[0:i4:432(ptr:0)][0:i2:8(ptr:54)];
;           |   |      |   |   %70 = 0.000000e+00  *  0.000000e+00;
;           |   |      |   |   %71 = 0.000000e+00  *  0.000000e+00;
;           |   |      |   + END LOOP
;           |   |      + END LOOP
;           |   |
;           |   |      %hir.de.ssa.copy5.in5 = 0.000000e+00;
;           |   |
;           |   |      + DO i3 = 0, sext.i32.i64(%1) + -1, 1   <DO_LOOP>
;           |   |      |   %88 = (null)[0:0:432(ptr:0)][0:i1:8(ptr:54)];
;           |   |      |   %hir.de.ssa.copy7.in = 0.000000e+00;
;           |   |      |
;           |   |      |   + DO i4 = 0, sext.i32.i64(%2) + -1, 1   <DO_LOOP>
;           |   |      |   |   %94 = (@eht_mp_bt_)[0:i4:0(ptr:0)][0:i3:0(ptr:0)];
;           |   |      |   |   %97 = (@nsarh_mp_qyt_)[0:i4:432(ptr:0)][0:i2:8(ptr:54)];
;           |   |      |   |   %hir.de.ssa.copy7.in = 0.000000e+00;
;           |   |      |   + END LOOP
;           |   |      |
;           |   |      |   %hir.de.ssa.copy5.in5 = 0.000000e+00;
;           |   |      + END LOOP
;           |   |   }
;           |   + END LOOP
;           + END LOOP
;     END REGION

; CHECK-COUNT: Use may not execute.
; CHECK : BEGIN REGION
; CHECK-NOT: modified

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sizes_mp_ny_ = external global i32
@sizes_mp_nx_ = external global i32
@sizes_mp_nky_ = external global i32
@sizes_mp_nkx_ = external global i32
@ens_mp_ensarh_ = external global [1000 x [1000 x double]]
;@nsarh_mp_qyt_ = external global [54 x [54 x double]]
;@nsarh_mp_wty_ = external global [54 x [54 x double]]
@nsarh_mp_qxt_ = external global [54 x [54 x double]]
@nsarh_mp_wtx_ = external global [54 x [54 x double]]
@parameters_mp_w_ = external global double
;@eht_mp_bt_ = external global [50 x [50 x double]]

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #1

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #3

; Function Attrs: nofree nosync nounwind uwtable
define fastcc void @farhim_(ptr nocapture noundef readonly %eht_mp_bt_, ptr nocapture noundef readonly %nsarh_mp_qyt_, ptr nocapture noundef readonly %nsarh_mp_wty_) #4 {
  %1 = load i32, ptr null, align 8
  %2 = load i32, ptr null, align 8
  %3 = sext i32 0 to i64
  %4 = shl nsw i64 0, 0
  %5 = icmp slt i32 0, 0
  br label %25

6:                                                ; No predecessors!
  %7 = zext i32 0 to i64
  %8 = icmp slt i32 0, 0
  %9 = add nsw i64 0, 0
  %10 = add nuw nsw i64 0, 0
  br label %20

11:                                               ; preds = %22, %11
  %12 = phi i64 [ 0, %22 ], [ 0, %11 ]
  %13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %14 = add nuw nsw i64 0, 0
  %15 = icmp eq i64 0, 0
  br label %11

16:                                               ; No predecessors!
  br label %17

17:                                               ; preds = %20, %16
  %18 = add nuw nsw i64 0, 0
  %19 = icmp eq i64 0, 0
  br label %20

20:                                               ; preds = %17, %6
  %21 = phi i64 [ 0, %6 ], [ 0, %17 ]
  br label %17

22:                                               ; No predecessors!
  %23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  br label %11

24:                                               ; No predecessors!
  br label %25

25:                                               ; preds = %24, %0
  %26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %27 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %28 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %29 = load i32, ptr null, align 8
  %30 = icmp slt i32 0, 0
  br label %31

31:                                               ; preds = %25
  %32 = load i32, ptr null, align 8
  %33 = icmp slt i32 0, 1
  %34 = icmp slt i32 0, 0
  %35 = load double, ptr null, align 8
  %36 = add nuw nsw i32 %2, 1
  %37 = sext i32 0 to i64
  %38 = add nuw nsw i32 %1, 1
  %39 = add nuw nsw i32 %32, 1
  %40 = sext i32 0 to i64
  %41 = add nuw nsw i32 %29, 1
  %42 = zext i32 %41 to i64
  %43 = sext i32 %39 to i64
  %44 = sext i32 %38 to i64
  %45 = sext i32 %36 to i64
  br label %46

46:                                               ; preds = %109, %31
  %47 = phi i64 [ 1, %31 ], [ %110, %109 ]
  br i1 true, label %109, label %48

48:                                               ; preds = %46
  %49 = add nsw i64 0, 0
  %50 = mul nsw i64 0, 0
  br label %51

51:                                               ; preds = %105, %48
  %52 = phi i64 [ 1, %48 ], [ %106, %105 ]
  %cond = icmp eq i64 %52, 10
  br i1 %cond, label %53, label %55

53:                                               ; preds = %51
  %54 = add nsw i64 0, 0
  br label %105

55:                                               ; preds = %51
  %56 = add nsw i64 0, 0
  br label %57

57:                                               ; preds = %78, %55
  %58 = phi i64 [ 1, %55 ], [ %79, %78 ]
  br label %59

59:                                               ; preds = %57
  %60 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %61 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %62 = load double, ptr null, align 8
  %63 = add nsw i64 0, 0
  %64 = mul nsw i64 0, 0
  br label %65

65:                                               ; preds = %65, %59
  %66 = phi i64 [ 1, %59 ], [ %75, %65 ]
  %67 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 432, ptr nonnull elementtype(double) %nsarh_mp_wty_, i64 %66)
  %68 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %67, i64 %52)
  %69 = load double, ptr %68, align 8
  %70 = fmul double 0.000000e+00, 0.000000e+00
  %71 = fmul double 0.000000e+00, 0.000000e+00
  %72 = add nsw i64 0, 0
  %73 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %74 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(double) null, i64 0)
  %75 = add nuw nsw i64 %66, 1
  %76 = icmp eq i64 %75, %45
  br i1 %76, label %77, label %65

77:                                               ; preds = %65
  br label %78

78:                                               ; preds = %77, %57
  %79 = add nuw nsw i64 %58, 1
  %80 = icmp eq i64 %79, %44
  br i1 %80, label %81, label %57

81:                                               ; preds = %78
  br label %82

82:                                               ; preds = %101, %81
  %83 = phi i64 [ %102, %101 ], [ 1, %81 ]
  %84 = phi double [ 0.000000e+00, %101 ], [ 0.000000e+00, %81 ]
  br label %85

85:                                               ; preds = %82
  %86 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 432, ptr nonnull elementtype(double) null, i64 0)
  %87 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %86, i64 %47)
  %88 = load double, ptr %87, align 8
  br label %89

89:                                               ; preds = %89, %85
  %90 = phi i64 [ 1, %85 ], [ %98, %89 ]
  %91 = phi double [ 0.000000e+00, %85 ], [ 0.000000e+00, %89 ]
  %92 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 0, ptr nonnull elementtype(double) %eht_mp_bt_, i64 %90)
  %93 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr nonnull elementtype(double) %92, i64 %83)
  %94 = load double, ptr %93, align 8
  %95 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 432, ptr nonnull elementtype(double) %nsarh_mp_qyt_, i64 %90)
  %96 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %95, i64 %52)
  %97 = load double, ptr %96, align 8
  %98 = add nuw nsw i64 %90, 1
  %99 = icmp eq i64 %98, %45
  br i1 %99, label %100, label %89

100:                                              ; preds = %89
  br label %101

101:                                              ; preds = %100, %82
  %102 = add nuw nsw i64 %83, 1
  %103 = icmp eq i64 %102, %44
  br i1 %103, label %104, label %82

104:                                              ; preds = %101
  br label %105

105:                                              ; preds = %104, %53
  %106 = add nuw nsw i64 %52, 1
  %107 = icmp eq i64 %106, %43
  br i1 %107, label %108, label %51

108:                                              ; preds = %105
  br label %109

109:                                              ; preds = %108, %46
  %110 = add nuw nsw i64 %47, 1
  %111 = icmp eq i64 %110, %42
  br i1 %111, label %112, label %46

112:                                              ; preds = %109
  ret void
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nocallback nofree nosync nounwind willreturn }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #4 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
