; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; Inline report
; RUN: opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-NPM,CHECK-AFTER
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-NPM,CHECK-AFTER

; Check that all instances of @wolff_ are inlined due to the inline budget and
; single callsite local linkage heuristics. This is the same test case as
; Intel_InlineReport58.ll, but it checks when an argument is a load instruction.

; CHECK-BEFORE-NOT: call{{.*}}@wolff_
; CHECK-LEGACY: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-LEGACY: INLINE: wolff_{{.*}}Callee has single callsite and local linkage
; CHECK-LEGACY: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-AFTER-NOT: call{{.*}}@wolff_

; CHECK-NPM: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-NPM: INLINE: wolff_{{.*}}Has inline budget for small application
; CHECK-NPM: INLINE: wolff_{{.*}}Callee has single callsite and local linkage
; CHECK-NPM-NOT: call{{.*}}@wolff_

declare double @fmod(double %0, double %1) local_unnamed_addr

define internal fastcc void @wolff_(double %0, double* noalias nocapture %1, i32* noalias nocapture %2, i32* noalias nocapture %3) unnamed_addr #0 {
  store i32 0, i32* %2, align 1
  store i32 1, i32* %3, align 1
  %5 = load double, double* %1, align 1
  %6 = fmul fast double %5, 1.680700e+04
  %7 = frem fast double %6, 0x41DFFFFFFFC00000
  %8 = fmul fast double %7, 0x3E00000000200000
  %9 = fptrunc double %8 to float
  %10 = fmul fast double %7, 1.680700e+04
  %11 = frem fast double %10, 0x41DFFFFFFFC00000
  store double %11, double* %1, align 1
  %12 = fmul fast double %11, 0x3E00000000200000
  %13 = fptrunc double %12 to float
  %14 = fmul fast float %9, 1.600000e+01
  %15 = fadd fast float %14, 1.000000e+00
  %16 = fptosi float %15 to i32
  %17 = icmp slt i32 16, %16
  %18 = select i1 %17, i32 16, i32 %16
  %19 = fmul fast float %13, 1.600000e+01
  %20 = fadd fast float %19, 1.000000e+00
  %21 = fptosi float %20 to i32
  %22 = icmp slt i32 16, %21
  %23 = select i1 %22, i32 16, i32 %21
  %24 = sext i32 %18 to i64
  %25 = sext i32 %23 to i64
  %26 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 64, i32* elementtype(i32) nonnull getelementptr inbounds ([16 x [16 x i32]], [16 x [16 x i32]]* @"_unnamed_main$$_$IZ", i64 0, i64 0, i64 0), i64 %25)
  %27 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %26, i64 %24)
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
  %40 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 1024, i32* elementtype(i32) nonnull getelementptr inbounds ([2 x [256 x i32]], [2 x [256 x i32]]* @"_unnamed_main$$_$ISTACK", i64 0, i64 0, i64 0), i64 1)
  %41 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 1024, i32* elementtype(i32) nonnull getelementptr inbounds ([2 x [256 x i32]], [2 x [256 x i32]]* @"_unnamed_main$$_$ISTACK", i64 0, i64 0, i64 0), i64 2)
  br label %42

42:                                               ; preds = %99, %4
  %43 = phi i32 [ 1, %4 ], [ %90, %99 ]
  %44 = phi double [ %11, %4 ], [ %91, %99 ]
  %45 = phi i32 [ 1, %4 ], [ %92, %99 ]
  %46 = phi i32 [ 0, %4 ], [ %105, %99 ]
  %47 = phi i32 [ %18, %4 ], [ %102, %99 ]
  %48 = phi i32 [ %23, %4 ], [ %104, %99 ]
  store i32 %47, i32* %31, align 1
  %49 = sext i32 %48 to i64
  %50 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IN1", i64 0, i64 0), i64 %49)
  %51 = load i32, i32* %50, align 1
  store i32 %51, i32* %33, align 1
  store i32 %47, i32* %34, align 1
  %52 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IP1", i64 0, i64 0), i64 %49)
  %53 = load i32, i32* %52, align 1
  store i32 %53, i32* %35, align 1
  %54 = sext i32 %47 to i64
  %55 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IN1", i64 0, i64 0), i64 %54)
  %56 = load i32, i32* %55, align 1
  store i32 %56, i32* %36, align 1
  store i32 %48, i32* %37, align 1
  %57 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull getelementptr inbounds ([16 x i32], [16 x i32]* @"_unnamed_main$$_$IP1", i64 0, i64 0), i64 %54)
  %58 = load i32, i32* %57, align 1
  store i32 %58, i32* %38, align 1
  store i32 %48, i32* %39, align 1
  br label %59

59:                                               ; preds = %89, %42
  %60 = phi i32 [ %90, %89 ], [ %43, %42 ]
  %61 = phi double [ %91, %89 ], [ %44, %42 ]
  %62 = phi i64 [ %94, %89 ], [ 1, %42 ]
  %63 = phi i32 [ %92, %89 ], [ %45, %42 ]
  %64 = phi i32 [ %93, %89 ], [ %46, %42 ]
  %65 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %30, i64 %62)
  %66 = load i32, i32* %65, align 1
  %67 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %32, i64 %62)
  %68 = load i32, i32* %67, align 1
  %69 = sext i32 %66 to i64
  %70 = sext i32 %68 to i64
  %71 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 64, i32* elementtype(i32) nonnull getelementptr inbounds ([16 x [16 x i32]], [16 x [16 x i32]]* @"_unnamed_main$$_$IZ", i64 0, i64 0, i64 0), i64 %70)
  %72 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %71, i64 %69)
  %73 = load i32, i32* %72, align 1
  %74 = icmp eq i32 %28, %73
  br i1 %74, label %75, label %89

75:                                               ; preds = %59
  %76 = add nsw i32 %63, 1
  %77 = fmul fast double %61, 1.680700e+04
  %78 = frem fast double %77, 0x41DFFFFFFFC00000
  store double %78, double* %1, align 1
  %79 = fmul fast double %78, 0x3E00000000200000
  %80 = fptrunc double %79 to float
  %81 = fpext float %80 to double
  %82 = fcmp fast olt double %0, %81
  br i1 %82, label %89, label %83

83:                                               ; preds = %75
  store i32 %29, i32* %72, align 1
  %84 = add nsw i32 %60, 1
  store i32 %84, i32* %3, align 1
  %85 = add nsw i32 %64, 1
  %86 = sext i32 %85 to i64
  %87 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %40, i64 %86)
  store i32 %66, i32* %87, align 1
  %88 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %41, i64 %86)
  store i32 %68, i32* %88, align 1
  br label %89

89:                                               ; preds = %83, %75, %59
  %90 = phi i32 [ %60, %59 ], [ %60, %75 ], [ %84, %83 ]
  %91 = phi double [ %61, %59 ], [ %78, %75 ], [ %78, %83 ]
  %92 = phi i32 [ %63, %59 ], [ %76, %75 ], [ %76, %83 ]
  %93 = phi i32 [ %64, %59 ], [ %64, %75 ], [ %85, %83 ]
  %94 = add nuw nsw i64 %62, 1
  %95 = icmp eq i64 %94, 5
  br i1 %95, label %96, label %59

96:                                               ; preds = %89
  %97 = icmp eq i32 %93, 0
  br i1 %97, label %98, label %99

98:                                               ; preds = %96
  store i32 %92, i32* %2, align 1
  ret void

99:                                               ; preds = %96
  %100 = sext i32 %93 to i64
  %101 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %40, i64 %100)
  %102 = load i32, i32* %101, align 1
  %103 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %41, i64 %100)
  %104 = load i32, i32* %103, align 1
  %105 = add nsw i32 %93, -1
  br label %42
}

define dso_local void @MAIN__() local_unnamed_addr #0 {
L00:
  %t02 = alloca double, align 8
  %t03 = alloca double, align 8
  %t04 = alloca i32, align 8
  %t05 = alloca i32, align 8
  %t06 = alloca double, align 8
  br label %L01
L01:
  %t10 = phi i32 [ 0, %L00 ], [ %t12, %L01 ]
  %tload = load double, double* %t06
  call fastcc void @wolff_(double %tload, double* nonnull %t02, i32* nonnull %t05, i32* nonnull %t04)
  %t11 = load i32, i32* %t05, align 8
  %t12 = add nsw i32 %t11, %t10
  %t13 = icmp slt i32 %t12, 256
  br i1 %t13, label %L01, label %L02
L02:
  %t20 = phi i32 [ 0, %L01 ], [ %t22, %L02 ]
  call fastcc void @wolff_(double %tload, double* nonnull %t02, i32* nonnull %t05, i32* nonnull %t04)
  %t21 = load i32, i32* %t05, align 8
  %t22 = add nsw i32 %t21, %t10
  %t23 = icmp slt i32 %t22, 256
  br i1 %t23, label %L02, label %L03
L03:
  %t30 = phi i32 [ 0, %L02 ], [ %t32, %L03 ]
  call fastcc void @wolff_(double %tload, double* nonnull %t02, i32* nonnull %t05, i32* nonnull %t04)
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
