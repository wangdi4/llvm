; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Check that we fuse succeeds in only fusing the "good nodes" for the following test.
; Compiler was previously asserting due to having multiple paths and trying to fuse them
; all even in presence of a bad node. There also is a cycle in the fusegraph.

;        A
;      /   \
;     B     C
;      \   /
;        D
;
; Assume we want to fuse A and D, and the fusegraph says they depend on B or C, independently.
; Suppose B is not able to fuse, then there still exists a way to fuse A and D through C,
; but fusion was trying to fuse B due it not checking all the intermediate paths through B during
; the merging process.
;

; HIR Before Fusion

;      BEGIN REGION { }
;            + DO i1 = 0, 0, 1   <DO_LOOP>
;            |   %hir.de.ssa.copy3.out = %6;
;            |
;            |   + DO i2 = 0, 0, 1   <DO_LOOP>
;            |   |   %hir.de.ssa.copy4.out = 0;
;            |   + END LOOP
;            |
;            |   (null)[0] = 0;
;            |   %6 = %6  |  %hir.de.ssa.copy4.out;
;            |   (null)[0] = %hir.de.ssa.copy3.out;
;            |   (null)[0] = 0;
;            |   %5 = 0  |  0;
;            |   (null)[0] = %5;
;            |   %33 = 0;
;            |   %34 = 0;
;            |   %35 = 0;
;            |
;            |   + DO i2 = 0, 8, 1   <DO_LOOP>
;            |   |   %hir.de.ssa.copy7.out = %35;
;            |   |   %hir.de.ssa.copy6.out = %34;
;            |   |   %hir.de.ssa.copy5.out = %33;
;            |   |
;            |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
;            |   |   |   if ((@"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL")[0][i2 + 1][1] != 0)
;            |   |   |   {
;            |   |   |      %14 = (@"MIX_SUBMESO_mp_SUBMESO_SF$ML_DEPTH")[0][i2][i3]  -  0.000000e+00;
;            |   |   |   }
;            |   |   + END LOOP
;            |   |
;            |   |   %33 = 1;
;            |   |   %34 = 1;
;            |   |   %35 = 1;
;            |   + END LOOP
;            |
;            |   (null)[0] = 0;
;            |   %29 = 1  |  %hir.de.ssa.copy7.out;
;            |   (null)[0] = %29;
;            |   (null)[0] = 0;
;            |   %30 = 1  |  %hir.de.ssa.copy6.out;
;            |   (null)[0] = %4;
;            |   %31 = 1  |  %hir.de.ssa.copy5.out;
;            |   (null)[0] = %3;
;            |
;            |   + DO i2 = 0, 9, 1   <DO_LOOP>
;            |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
;            |   |   |   (%1)[i2][0] = (@"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL")[0][i2][1];
;            |   |   + END LOOP
;            |   + END LOOP
;            |
;            |
;            |   + DO i2 = 0, 9, 1   <DO_LOOP>
;            |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
;            |   |   |   if ((%1)[i2][0] != 0)
;            |   |   |   {
;            |   |   |      %54 = 0.000000e+00  *  (null)[0];
;            |   |   |   }
;            |   |   + END LOOP
;            |   + END LOOP
;            |
;            |
;            |   + DO i2 = 0, 8, 1   <DO_LOOP>
;            |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
;            |   |   |   %73 = (@"MIX_SUBMESO_mp_SUBMESO_SF$ML_DEPTH")[0][i2][i3] <=u 0.000000e+00;
;            |   |   |   %74 = zext.i1.i64(%73);
;            |   |   |   (@"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL")[0][i2][1] = 0;
;            |   |   + END LOOP
;            |   + END LOOP
;            |
;            |   %3 = %31;
;            |   %4 = %30;
;            + END LOOP
;      END REGION

; HIR After fusion

; CHECK:       BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 0, 1   <DO_LOOP>
;                  |   %hir.de.ssa.copy3.out = %6;
;                  |
;                  |   + DO i2 = 0, 0, 1   <DO_LOOP>
;                  |   |   %hir.de.ssa.copy4.out = 0;
;                  |   + END LOOP
;                  |
;                  |   (null)[0] = 0;
;                  |   %6 = %6  |  %hir.de.ssa.copy4.out;
;                  |   (null)[0] = %hir.de.ssa.copy3.out;
;                  |   (null)[0] = 0;
;                  |   %5 = 0  |  0;
;                  |   (null)[0] = %5;
;                  |   %33 = 0;
;                  |   %34 = 0;
;                  |   %35 = 0;
;                  |   (null)[0] = 0;
;                  |
;                  |   + DO i2 = 0, 8, 1   <DO_LOOP>
;                  |   |   %hir.de.ssa.copy7.out = %35;
;                  |   |   %hir.de.ssa.copy6.out = %34;
;                  |   |   %hir.de.ssa.copy5.out = %33;
;                  |   |
;                  |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
;                  |   |   |   if ((@"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL")[0][i2 + 1][1] != 0)
;                  |   |   |   {
;                  |   |   |      %14 = (@"MIX_SUBMESO_mp_SUBMESO_SF$ML_DEPTH")[0][i2][i3]  -  0.000000e+00;
;                  |   |   |   }
;                  |   |   |   %73 = (@"MIX_SUBMESO_mp_SUBMESO_SF$ML_DEPTH")[0][i2][i3] <=u 0.000000e+00;
;                  |   |   |   %74 = zext.i1.i64(%73);
;                  |   |   |   (@"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL")[0][i2][1] = 0;
;                  |   |   + END LOOP
;                  |   |
;                  |   |   %33 = 1;
;                  |   |   %34 = 1;
;                  |   |   %35 = 1;
;                  |   + END LOOP
;                  |   %29 = 1  |  %hir.de.ssa.copy7.out;
;                  |   (null)[0] = %29;
;                  |   (null)[0] = 0;
;                  |   (null)[0] = %4;
;                  |   (null)[0] = %3;
;                  |
; CHECK:           |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
; CHECK:           |   |   |   (%1)[i2][0] = (@"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL")[0][i2][1];
; CHECK:           |   |   + END LOOP
; CHECK:           |   |
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
; CHECK:           |   |   |   if ((%1)[i2][0] != 0)
; CHECK:           |   |   |   {
; CHECK:           |   |   |      %54 = 0.000000e+00  *  (null)[0];
; CHECK:           |   |   |   }
; CHECK:           |   |   + END LOOP
;                  |
;                  |   %30 = 1  |  %hir.de.ssa.copy6.out;
;                  |   %31 = 1  |  %hir.de.ssa.copy5.out;
;                  |   %3 = %31;
;                  |   %4 = %30;
;                  + END LOOP
;            END REGION


; CHECK:           |   + END LOOP


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30146"

@"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL" = external global [8 x [54 x i32]]
@"MIX_SUBMESO_mp_SUBMESO_SF$ML_DEPTH" = external global [8 x [54 x double]]

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define void @MIX_SUBMESO_mp_SUBMESO_SF() {
  %1 = alloca [432 x i32], i32 0, align 16
  br label %2

2:                                                ; preds = %84, %0
  %3 = phi i64 [ %31, %84 ], [ 0, %0 ]
  %4 = phi i64 [ %30, %84 ], [ 0, %0 ]
  %5 = phi i64 [ %12, %84 ], [ 0, %0 ]
  %6 = phi i64 [ %11, %84 ], [ 0, %0 ]
  br label %7

7:                                                ; preds = %7, %2
  %8 = phi i64 [ 0, %2 ], [ 0, %7 ]
  %9 = icmp eq i64 0, 0
  br i1 %9, label %10, label %7

10:                                               ; preds = %7
  store i64 0, ptr null, align 8
  %11 = or i64 %6, %8
  store i64 %6, ptr null, align 8
  store i64 0, ptr null, align 8
  %12 = or i64 %5, 0
  store i64 %12, ptr null, align 8
  br label %32

13:                                               ; preds = %18
  %14 = fsub double %23, 0.000000e+00
  br label %15

15:                                               ; preds = %18, %13
  %16 = add i64 %19, 1
  %17 = icmp eq i64 1, 1
  br i1 %17, label %25, label %18

18:                                               ; preds = %32, %15
  %19 = phi i64 [ 1, %32 ], [ %16, %15 ]
  %20 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %37, i64 1)
  %21 = load i32, ptr %20, align 4
  %22 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %38, i64 %19)
  %23 = load double, ptr %22, align 8
  %24 = icmp eq i32 %21, 0
  br i1 %24, label %15, label %13

25:                                               ; preds = %15
  %26 = add i64 %36, 1
  %27 = icmp eq i64 %36, 9
  br i1 %27, label %28, label %32

28:                                               ; preds = %25
  store i64 0, ptr null, align 8
  %29 = or i64 1, %35
  store i64 %29, ptr null, align 8
  store i64 0, ptr null, align 8
  %30 = or i64 1, %34
  store i64 %4, ptr null, align 8
  %31 = or i64 1, %33
  store i64 %3, ptr null, align 8
  br label %47

32:                                               ; preds = %25, %10
  %33 = phi i64 [ 0, %10 ], [ 1, %25 ]
  %34 = phi i64 [ 0, %10 ], [ 1, %25 ]
  %35 = phi i64 [ 0, %10 ], [ 1, %25 ]
  %36 = phi i64 [ 1, %10 ], [ %26, %25 ]
  %37 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 216, ptr elementtype(i32) @"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL", i64 %36)
  %38 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 432, ptr elementtype(double) @"MIX_SUBMESO_mp_SUBMESO_SF$ML_DEPTH", i64 %36)
  br label %18

39:                                               ; preds = %47, %39
  %40 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %49, i64 1)
  %41 = load i32, ptr %40, align 4
  %42 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %50, i64 0)
  store i32 %41, ptr %42, align 4
  %43 = icmp eq i64 0, 0
  br i1 %43, label %44, label %39

44:                                               ; preds = %39
  %45 = add i64 %48, 1
  %46 = icmp eq i64 %48, 9
  br i1 %46, label %51, label %47

47:                                               ; preds = %44, %28
  %48 = phi i64 [ 0, %28 ], [ %45, %44 ]
  %49 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 216, ptr elementtype(i32) @"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL", i64 %48)
  %50 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 216, ptr elementtype(i32) %1, i64 %48)
  br label %39

51:                                               ; preds = %44
  br label %64

52:                                               ; preds = %57
  %53 = load double, ptr null, align 1
  %54 = fmul double 0.000000e+00, %53
  br label %55

55:                                               ; preds = %57, %52
  %56 = icmp eq i64 0, 0
  br i1 %56, label %61, label %57

57:                                               ; preds = %64, %55
  %58 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %66, i64 0)
  %59 = load i32, ptr %58, align 4
  %60 = icmp eq i32 %59, 0
  br i1 %60, label %55, label %52

61:                                               ; preds = %55
  %62 = add i64 %65, 1
  %63 = icmp eq i64 %65, 9
  br i1 %63, label %67, label %64

64:                                               ; preds = %61, %51
  %65 = phi i64 [ 0, %51 ], [ %62, %61 ]
  %66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 216, ptr elementtype(i32) %1, i64 %65)
  br label %57

67:                                               ; preds = %61
  br label %80

68:                                               ; preds = %80, %68
  %69 = phi i64 [ 1, %80 ], [ %75, %68 ]
  %70 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 4, ptr elementtype(i32) %82, i64 1)
  %71 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %83, i64 %69)
  %72 = load double, ptr %71, align 8
  %73 = fcmp ule double %72, 0.000000e+00
  %74 = zext i1 %73 to i64
  store i32 0, ptr %70, align 4
  %75 = add i64 %69, 1
  %76 = icmp eq i64 0, 0
  br i1 %76, label %77, label %68

77:                                               ; preds = %68
  %78 = add i64 %81, 1
  %79 = icmp eq i64 %81, 9
  br i1 %79, label %84, label %80

80:                                               ; preds = %77, %67
  %81 = phi i64 [ 1, %67 ], [ %78, %77 ]
  %82 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 216, ptr elementtype(i32) @"MIX_SUBMESO_mp_SUBMESO_SF$CONTINUE_INTEGRAL", i64 %81)
  %83 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 432, ptr elementtype(double) @"MIX_SUBMESO_mp_SUBMESO_SF$ML_DEPTH", i64 %81)
  br label %68

84:                                               ; preds = %77
  %85 = icmp eq i64 0, 0
  br i1 %85, label %86, label %2

86:                                               ; preds = %84
  %87 = phi i64 [ %74, %84 ]
  ret void

; uselistorder directives
  uselistorder i64 %36, { 0, 1, 3, 2 }
  uselistorder i64 %48, { 0, 1, 3, 2 }
  uselistorder i64 %65, { 0, 2, 1 }
  uselistorder i64 %81, { 0, 1, 3, 2 }
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
