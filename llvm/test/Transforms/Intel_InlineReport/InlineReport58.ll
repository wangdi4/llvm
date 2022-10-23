; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that all instances of @wolff_ are inlined due to the inline budget and
; single callsite local linkage heuristics.

; CHECK-BEFORE-NOT: call{{.*}}@wolff_
; CHECK: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK: INLINE: wolff_{{.*}}Callee has single callsite and local linkage
; CHECK-AFTER-NOT: call{{.*}}@wolff_

declare double @fmod(double %0, double %1) local_unnamed_addr

define internal fastcc void @wolff_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture %2, i32* noalias nocapture %3) unnamed_addr #0 {
  store i32 0, i32* %2, align 1
  store i32 1, i32* %3, align 1
  %5 = load double, double* %1, align 1
  %6 = fmul fast double %5, 1.680700e+04
  %7 = tail call fast double @fmod(double %6, double 0x41DFFFFFFFC00000)
  %8 = fmul fast double %7, 0x3E00000000200000
  %9 = fptrunc double %8 to float
  %10 = fmul fast double %7, 1.680700e+04
  %11 = tail call fast double @fmod(double %10, double 0x41DFFFFFFFC00000)
  store double %11, double* %1, align 1
  %12 = fmul fast double %11, 0x3E00000000200000
  %13 = fptrunc double %12 to float
  %14 = fmul fast float %9, 1.600000e+01
  %15 = fadd fast float %14, 1.000000e+00
  %16 = fptosi float %15 to i32
  %17 = icmp slt i32 %16, 16
  %18 = select i1 %17, i32 %16, i32 16
  %19 = fmul fast float %13, 1.600000e+01
  %20 = fadd fast float %19, 1.000000e+00
  %21 = fptosi float %20 to i32
  %22 = icmp slt i32 %21, 16
  %23 = select i1 %22, i32 %21, i32 16
  %24 = sext i32 %18 to i64
  %25 = sext i32 %23 to i64
  %26 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 64, i32* elementtype(i32) getelementptr inbounds ([16 x [16 x i32]], [16 x [16 x i32]]* @"_unnamed_main$$_$IZ", i64 0, i64 0, i64 0), i64 %25)
  %27 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %26, i64 %24)
  %28 = load i32, i32* %27, align 1
  %29 = sub nsw i32 0, %28
  store i32 %29, i32* %27, align 1
  %30 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 16, i32* elementtype(i32) getelementptr inbounds ([2 x [4 x i32]], [2 x [4 x i32]]* @"wolff_$NN", i64 0, i64 0, i64 0), i64 1)
  %31 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %30, i64 1)
  %32 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 16, i32* elementtype(i32) getelementptr inbounds ([2 x [4 x i32]], [2 x [4 x i32]]* @"wolff_$NN", i64 0, i64 0, i64 0), i64 2)
  %33 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %32, i64 1)
  %34 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %30, i64 2)
  %35 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %32, i64 2)
  %36 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %30, i64 3)
  %37 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %32, i64 3)
  %38 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %30, i64 4)
  %39 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %32, i64 4)
  %40 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 1024, i32* elementtype(i32) getelementptr inbounds ([2 x [256 x i32]], [2 x [256 x i32]]* @"_unnamed_main$$_$ISTACK", i64 0, i64 0, i64 0), i64 1)
  %41 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 1024, i32* elementtype(i32) getelementptr inbounds ([2 x [256 x i32]], [2 x [256 x i32]]* @"_unnamed_main$$_$ISTACK", i64 0, i64 0, i64 0), i64 2)
  br label %42

42:                                               ; preds = %99, %4
  %43 = phi i32 [ 1, %4 ], [ %91, %99 ]
  %44 = phi double [ %11, %4 ], [ %92, %99 ]
  %45 = phi i32 [ 1, %4 ], [ %93, %99 ]
  %46 = phi i32 [ 0, %4 ], [ %105, %99 ]
  %47 = phi i32 [ %18, %4 ], [ %102, %99 ]
  %48 = phi i32 [ %23, %4 ], [ %104, %99 ]
  store i32 %47, i32* %31, align 1
  %49 = sext i32 %48 to i64
  %50 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IN1", i64 0, i64 0), i64 %49)
  %51 = load i32, i32* %50, align 1
  store i32 %51, i32* %33, align 1
  store i32 %47, i32* %34, align 1
  %52 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IP1", i64 0, i64 0), i64 %49)
  %53 = load i32, i32* %52, align 1
  store i32 %53, i32* %35, align 1
  %54 = sext i32 %47 to i64
  %55 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IN1", i64 0, i64 0), i64 %54)
  %56 = load i32, i32* %55, align 1
  store i32 %56, i32* %36, align 1
  store i32 %48, i32* %37, align 1
  %57 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IP1", i64 0, i64 0), i64 %54)
  %58 = load i32, i32* %57, align 1
  store i32 %58, i32* %38, align 1
  store i32 %48, i32* %39, align 1
  br label %59

59:                                               ; preds = %90, %42
  %60 = phi i32 [ %91, %90 ], [ %43, %42 ]
  %61 = phi double [ %92, %90 ], [ %44, %42 ]
  %62 = phi i64 [ %95, %90 ], [ 1, %42 ]
  %63 = phi i32 [ %93, %90 ], [ %45, %42 ]
  %64 = phi i32 [ %94, %90 ], [ %46, %42 ]
  %65 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %30, i64 %62)
  %66 = load i32, i32* %65, align 1
  %67 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %32, i64 %62)
  %68 = load i32, i32* %67, align 1
  %69 = sext i32 %66 to i64
  %70 = sext i32 %68 to i64
  %71 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 64, i32* elementtype(i32) nonnull getelementptr inbounds ([16 x [16 x i32]], [16 x [16 x i32]]* @"_unnamed_main$$_$IZ", i64 0, i64 0, i64 0), i64 %70)
  %72 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %71, i64 %69)
  %73 = load i32, i32* %72, align 1
  %74 = icmp eq i32 %28, %73
  br i1 %74, label %75, label %90

75:                                               ; preds = %59
  %76 = add nsw i32 %63, 1
  %77 = fmul fast double %61, 1.680700e+04
  %78 = tail call fast double @fmod(double %77, double 0x41DFFFFFFFC00000)
  store double %78, double* %1, align 1
  %79 = fmul fast double %78, 0x3E00000000200000
  %80 = fptrunc double %79 to float
  %81 = fpext float %80 to double
  %82 = load double, double* %0, align 1
  %83 = fcmp fast olt double %82, %81
  br i1 %83, label %90, label %84

84:                                               ; preds = %75
  store i32 %29, i32* %72, align 1
  %85 = add nsw i32 %60, 1
  store i32 %85, i32* %3, align 1
  %86 = add nsw i32 %64, 1
  %87 = sext i32 %86 to i64
  %88 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %40, i64 %87)
  store i32 %66, i32* %88, align 1
  %89 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %41, i64 %87)
  store i32 %68, i32* %89, align 1
  br label %90

90:                                               ; preds = %84, %75, %59
  %91 = phi i32 [ %60, %59 ], [ %60, %75 ], [ %85, %84 ]
  %92 = phi double [ %61, %59 ], [ %78, %75 ], [ %78, %84 ]
  %93 = phi i32 [ %63, %59 ], [ %76, %75 ], [ %76, %84 ]
  %94 = phi i32 [ %64, %59 ], [ %64, %75 ], [ %86, %84 ]
  %95 = add nuw nsw i64 %62, 1
  %96 = icmp eq i64 %95, 5
  br i1 %96, label %97, label %59

97:                                               ; preds = %90
  %98 = icmp eq i32 %94, 0
  br i1 %98, label %106, label %99

99:                                               ; preds = %97
  %100 = sext i32 %94 to i64
  %101 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %40, i64 %100)
  %102 = load i32, i32* %101, align 1
  %103 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %41, i64 %100)
  %104 = load i32, i32* %103, align 1
  %105 = add nsw i32 %94, -1
  br label %42

106:                                              ; preds = %97
  store i32 %93, i32* %2, align 1
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr #0 {
L00:
  %t02 = alloca double, align 8
  %t03 = alloca double, align 8
  %t04 = alloca i32, align 8
  %t05 = alloca i32, align 8
  br label %L01
L01:
  %t10 = phi i32 [ 0, %L00 ], [ %t12, %L01 ]
  call fastcc void @wolff_(double* nonnull %t03, double* nonnull %t02, i32* nonnull %t05, i32* nonnull %t04)
  %t11 = load i32, i32* %t05, align 8
  %t12 = add nsw i32 %t11, %t10
  %t13 = icmp slt i32 %t12, 256
  br i1 %t13, label %L01, label %L02
L02:
  %t20 = phi i32 [ 0, %L01 ], [ %t22, %L02 ]
  call fastcc void @wolff_(double* nonnull %t03, double* nonnull %t02, i32* nonnull %t05, i32* nonnull %t04)
  %t21 = load i32, i32* %t05, align 8
  %t22 = add nsw i32 %t21, %t10
  %t23 = icmp slt i32 %t22, 256
  br i1 %t23, label %L02, label %L03
L03:
  %t30 = phi i32 [ 0, %L02 ], [ %t32, %L03 ]
  call fastcc void @wolff_(double* nonnull %t03, double* nonnull %t02, i32* nonnull %t05, i32* nonnull %t04)
  %t31 = load i32, i32* %t05, align 8
  %t32 = add nsw i32 %t31, %t10
  %t33 = icmp slt i32 %t32, 256
  br i1 %t33, label %L03, label %L04
L04:
  ret void
}

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* %3, i64 %4)

@"_unnamed_main$$_$IN1" = internal global [16 x i32] zeroinitializer, align 16
@"_unnamed_main$$_$IP1" = internal global [16 x i32] zeroinitializer, align 16
@"_unnamed_main$$_$ISTACK" = internal global [2 x [256 x i32]] zeroinitializer, align 16
@"_unnamed_main$$_$IZ" = internal global [16 x [16 x i32]] zeroinitializer, align 16

@"wolff_$NN" = internal unnamed_addr global [2 x [4 x i32]] zeroinitializer, align 16

attributes #0 = { "intel-lang"="fortran" }

; end INTEL_FEATURE_SW_ADVANCED
