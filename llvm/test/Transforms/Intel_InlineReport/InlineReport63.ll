; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that all no instances of @daxpy are inlined, because they fail to pass
; the matching actual parameter test for the callsites.

; CHECK-BEFORE: call{{.*}}daxpy_
; CHECK-BEFORE: call{{.*}}daxpy_
; CHECK-BEFORE: call{{.*}}daxpy_
; CHECK-NOT: INLINE: daxpy_{{.*}}Has inline budget for small application
; CHECK-NOT: INLINE: daxpy_{{.*}}Callee has single callsite and local linkage
; CHECK-AFTER: call{{.*}}daxpy_
; CHECK-AFTER: call{{.*}}daxpy_
; CHECK-AFTER: call{{.*}}daxpy_

define internal fastcc void @dscal_(i32* noalias nocapture readonly %0, double* noalias nocapture readonly %1, double* noalias nocapture %2) unnamed_addr #0 {
  %4 = load i32, i32* %0, align 1
  %5 = icmp sgt i32 %4, 0
  br i1 %5, label %49, label %52

6:                                                ; preds = %45, %6
  %7 = phi i64 [ 1, %45 ], [ %11, %6 ]
  %8 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %7)
  %9 = load double, double* %8, align 1
  %10 = fmul fast double %9, %46
  store double %10, double* %8, align 1
  %11 = add nuw nsw i64 %7, 1
  %12 = icmp eq i64 %11, %48
  br i1 %12, label %13, label %6

13:                                               ; preds = %6
  %14 = icmp sgt i32 %4, 4
  %15 = icmp sgt i32 %4, %50
  %16 = and i1 %14, %15
  br i1 %16, label %17, label %52

17:                                               ; preds = %49, %13
  %18 = load double, double* %1, align 1
  %19 = add nuw nsw i32 %50, 1
  %20 = zext i32 %19 to i64
  br label %21

21:                                               ; preds = %21, %17
  %22 = phi i64 [ %20, %17 ], [ %42, %21 ]
  %23 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %22)
  %24 = load double, double* %23, align 1
  %25 = fmul fast double %24, %18
  store double %25, double* %23, align 1
  %26 = add nuw nsw i64 %22, 1
  %27 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %26)
  %28 = load double, double* %27, align 1
  %29 = fmul fast double %28, %18
  store double %29, double* %27, align 1
  %30 = add nuw nsw i64 %22, 2
  %31 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %30)
  %32 = load double, double* %31, align 1
  %33 = fmul fast double %32, %18
  store double %33, double* %31, align 1
  %34 = add nuw nsw i64 %22, 3
  %35 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %34)
  %36 = load double, double* %35, align 1
  %37 = fmul fast double %36, %18
  store double %37, double* %35, align 1
  %38 = add nuw nsw i64 %22, 4
  %39 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %38)
  %40 = load double, double* %39, align 1
  %41 = fmul fast double %40, %18
  store double %41, double* %39, align 1
  %42 = add nuw i64 %22, 5
  %43 = trunc i64 %42 to i32
  %44 = icmp slt i32 %4, %43
  br i1 %44, label %52, label %21

45:                                               ; preds = %49
  %46 = load double, double* %1, align 1
  %47 = add nuw nsw i32 %50, 1
  %48 = zext i32 %47 to i64
  br label %6

49:                                               ; preds = %3
  %50 = urem i32 %4, 5
  %51 = icmp eq i32 %50, 0
  br i1 %51, label %17, label %45

52:                                               ; preds = %21, %13, %3
  ret void
}

define internal fastcc void @daxpy_(i32* noalias nocapture readonly %0, double* noalias nocapture readonly %1, double* noalias nocapture readonly %2, double* noalias nocapture %3) unnamed_addr #0 {
  %5 = load i32, i32* %0, align 1
  %6 = icmp sgt i32 %5, 0
  br i1 %6, label %62, label %65

7:                                                ; preds = %56, %7
  %8 = phi i64 [ 1, %56 ], [ %15, %7 ]
  %9 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %3, i64 %8)
  %10 = load double, double* %9, align 1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %8)
  %12 = load double, double* %11, align 1
  %13 = fmul fast double %12, %63
  %14 = fadd fast double %13, %10
  store double %14, double* %9, align 1
  %15 = add nuw nsw i64 %8, 1
  %16 = icmp eq i64 %15, %58
  br i1 %16, label %17, label %7

17:                                               ; preds = %7
  %18 = icmp slt i32 %5, 4
  br i1 %18, label %65, label %21

19:                                               ; preds = %59
  %20 = icmp sgt i32 %5, 3
  br i1 %20, label %21, label %65

21:                                               ; preds = %19, %17
  %22 = add nuw nsw i32 %60, 1
  %23 = zext i32 %22 to i64
  %24 = zext i32 %5 to i64
  br label %25

25:                                               ; preds = %25, %21
  %26 = phi i64 [ %23, %21 ], [ %54, %25 ]
  %27 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %3, i64 %26)
  %28 = load double, double* %27, align 1
  %29 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %26)
  %30 = load double, double* %29, align 1
  %31 = fmul fast double %30, %63
  %32 = fadd fast double %31, %28
  store double %32, double* %27, align 1
  %33 = add nuw nsw i64 %26, 1
  %34 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %3, i64 %33)
  %35 = load double, double* %34, align 1
  %36 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %33)
  %37 = load double, double* %36, align 1
  %38 = fmul fast double %37, %63
  %39 = fadd fast double %38, %35
  store double %39, double* %34, align 1
  %40 = add nuw nsw i64 %26, 2
  %41 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %3, i64 %40)
  %42 = load double, double* %41, align 1
  %43 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %40)
  %44 = load double, double* %43, align 1
  %45 = fmul fast double %44, %63
  %46 = fadd fast double %45, %42
  store double %46, double* %41, align 1
  %47 = add nuw nsw i64 %26, 3
  %48 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %3, i64 %47)
  %49 = load double, double* %48, align 1
  %50 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %2, i64 %47)
  %51 = load double, double* %50, align 1
  %52 = fmul fast double %51, %63
  %53 = fadd fast double %52, %49
  store double %53, double* %48, align 1
  %54 = add nuw nsw i64 %26, 4
  %55 = icmp ugt i64 %54, %24
  br i1 %55, label %65, label %25

56:                                               ; preds = %59
  %57 = add nuw nsw i32 %60, 1
  %58 = zext i32 %57 to i64
  br label %7

59:                                               ; preds = %62
  %60 = and i32 %5, 3
  %61 = icmp eq i32 %60, 0
  br i1 %61, label %19, label %56

62:                                               ; preds = %4
  %63 = load double, double* %1, align 1
  %64 = fcmp fast ueq double %63, 0.000000e+00
  br i1 %64, label %65, label %59

65:                                               ; preds = %62, %25, %19, %17, %4
  ret void
}

define internal fastcc void @dgefa_(i32* noalias nocapture %0) unnamed_addr #0 {
  %2 = alloca double, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 0, i32* %0, align 1
  %5 = bitcast double* %2 to i64*
  br label %6

6:                                                ; preds = %83, %1
  %7 = phi i64 [ 1, %1 ], [ %9, %83 ]
  %8 = phi i64 [ 2, %1 ], [ %86, %83 ]
  %9 = add nuw i64 %7, 1
  %10 = trunc i64 %7 to i32
  %11 = sub nsw i32 2500, %10
  %12 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %7)
  %13 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %12, i64 %7)
  %14 = icmp slt i32 %10, 2501
  br i1 %14, label %35, label %37

15:                                               ; preds = %28, %15
  %16 = phi i64 [ 2, %28 ], [ %26, %15 ]
  %17 = phi double [ %31, %28 ], [ %23, %15 ]
  %18 = phi i32 [ 1, %28 ], [ %25, %15 ]
  %19 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %13, i64 %16)
  %20 = load double, double* %19, align 1
  %21 = tail call fast double @llvm.fabs.f64(double %20)
  %22 = fcmp fast ogt double %21, %17
  %23 = select i1 %22, double %21, double %17
  %24 = trunc i64 %16 to i32
  %25 = select i1 %22, i32 %24, i32 %18
  %26 = add nuw nsw i64 %16, 1
  %27 = icmp eq i64 %26, %34
  br i1 %27, label %37, label %15

28:                                               ; preds = %35
  %29 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %13, i64 1)
  %30 = load double, double* %29, align 1
  %31 = tail call fast double @llvm.fabs.f64(double %30)
  %32 = shl i64 %7, 32
  %33 = sub i64 10746008174592, %32
  %34 = ashr exact i64 %33, 32
  br label %15

35:                                               ; preds = %6
  %36 = icmp eq i32 %11, 0
  br i1 %36, label %37, label %28

37:                                               ; preds = %35, %15, %6
  %38 = phi i32 [ 1, %35 ], [ 0, %6 ], [ %25, %15 ]
  %39 = add i32 %10, -1
  %40 = add i32 %39, %38
  %41 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([2500 x i32], [2500 x i32]* @"linpk_$IPVT", i64 0, i64 0), i64 %7)
  store i32 %40, i32* %41, align 1
  %42 = sext i32 %40 to i64
  %43 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %12, i64 %42)
  %44 = load double, double* %43, align 1
  %45 = fcmp fast oeq double %44, 0.000000e+00
  br i1 %45, label %79, label %80

46:                                               ; preds = %80
  store double %44, double* %2, align 8
  %47 = bitcast double* %13 to i64*
  %48 = load i64, i64* %47, align 1
  %49 = bitcast double* %43 to i64*
  store i64 %48, i64* %49, align 1
  store double %44, double* %13, align 1
  br label %50

50:                                               ; preds = %80, %46
  %51 = load double, double* %13, align 1
  %52 = fdiv fast double -1.000000e+00, %51
  store double %52, double* %2, align 8
  %53 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %12, i64 %9)
  store i32 %11, i32* %3, align 4
  call fastcc void @dscal_(i32* nonnull %3, double* nonnull %2, double* %53) #5
  %54 = icmp slt i64 %7, 2500
  br i1 %54, label %55, label %83

55:                                               ; preds = %50
  br i1 %82, label %56, label %69

56:                                               ; preds = %56, %55
  %57 = phi i64 [ %66, %56 ], [ %8, %55 ]
  %58 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) nonnull getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %57)
  %59 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %58, i64 %42)
  %60 = bitcast double* %59 to i64*
  %61 = load i64, i64* %60, align 1
  store i64 %61, i64* %5, align 8
  %62 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %58, i64 %7)
  %63 = bitcast double* %62 to i64*
  %64 = load i64, i64* %63, align 1
  store i64 %64, i64* %60, align 1
  store i64 %61, i64* %63, align 1
  %65 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %58, i64 %9)
  store i32 %11, i32* %4, align 4
  call fastcc void @daxpy_(i32* nonnull %3, double* nonnull %2, double* nonnull %2, double* %65) #5
  %66 = add i64 %57, 1
  %67 = trunc i64 %66 to i32
  %68 = icmp sgt i32 %67, 2500
  br i1 %68, label %83, label %56

69:                                               ; preds = %69, %55
  %70 = phi i64 [ %76, %69 ], [ %8, %55 ]
  %71 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) nonnull getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %70)
  %72 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %71, i64 %42)
  %73 = bitcast double* %72 to i64*
  %74 = load i64, i64* %73, align 1
  store i64 %74, i64* %5, align 8
  %75 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %71, i64 %9)
  store i32 %11, i32* %4, align 4
  call fastcc void @daxpy_(i32* nonnull %4, double* nonnull %2, double* nonnull %53, double* %75) #5
  %76 = add i64 %70, 1
  %77 = trunc i64 %76 to i32
  %78 = icmp sgt i32 %77, 2500
  br i1 %78, label %83, label %69

79:                                               ; preds = %37
  store i32 %10, i32* %0, align 1
  br label %83

80:                                               ; preds = %37
  %81 = zext i32 %40 to i64
  %82 = icmp ne i64 %7, %81
  br i1 %82, label %46, label %50

83:                                               ; preds = %79, %69, %56, %50
  %84 = trunc i64 %9 to i32
  %85 = icmp slt i32 %84, 2500
  %86 = add i64 %8, 1
  br i1 %85, label %6, label %87

87:                                               ; preds = %83
  %88 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([2500 x i32], [2500 x i32]* @"linpk_$IPVT", i64 0, i64 0), i64 2500)
  store i32 2500, i32* %88, align 1
  %89 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 2500)
  %90 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %89, i64 2500)
  %91 = load double, double* %90, align 1
  %92 = fcmp fast oeq double %91, 0.000000e+00
  br i1 %92, label %93, label %94

93:                                               ; preds = %87
  store i32 2500, i32* %0, align 1
  br label %94

94:                                               ; preds = %93, %87
  ret void
}

define internal fastcc void @dgesl_() unnamed_addr #0 {
  %1 = alloca double, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = bitcast double* %1 to i64*
  br label %5

5:                                                ; preds = %0, %19
  %6 = phi i64 [ 1, %0 ], [ %22, %19 ]
  %7 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([2500 x i32], [2500 x i32]* @"linpk_$IPVT", i64 0, i64 0), i64 %6)
  %8 = load i32, i32* %7, align 1
  %9 = sext i32 %8 to i64
  %10 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %9)
  %11 = bitcast double* %10 to i64*
  %12 = load i64, i64* %11, align 1
  store i64 %12, i64* %4, align 8
  %13 = zext i32 %8 to i64
  %14 = icmp eq i64 %6, %13
  br i1 %14, label %19, label %15

15:                                               ; preds = %5
  %16 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %6)
  %17 = bitcast double* %16 to i64*
  %18 = load i64, i64* %17, align 1
  store i64 %18, i64* %11, align 1
  store i64 %12, i64* %17, align 1
  br label %19

19:                                               ; preds = %15, %5
  %20 = trunc i64 %6 to i32
  %21 = sub nsw i32 2500, %20
  %22 = add nuw i64 %6, 1
  %23 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %6)
  %24 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %23, i64 %22)
  %25 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %22)
  store i32 %21, i32* %2, align 4
  call fastcc void @daxpy_(i32* nonnull %2, double* nonnull %24, double* %24, double* %25) #5
  %26 = trunc i64 %22 to i32
  %27 = icmp slt i32 %26, 2500
  br i1 %27, label %5, label %28

28:                                               ; preds = %19
  %29 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 1)
  br label %30

30:                                               ; preds = %30, %28
  %31 = phi i64 [ 1, %28 ], [ %45, %30 ]
  %32 = trunc i64 %31 to i32
  %33 = shl i64 %31, 32
  %34 = sub i64 10741713207296, %33
  %35 = ashr exact i64 %34, 32
  %36 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %35)
  %37 = load double, double* %36, align 1
  %38 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %35)
  %39 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %38, i64 %35)
  %40 = load double, double* %39, align 1
  %41 = fdiv fast double %37, %40
  store double %41, double* %36, align 1
  %42 = fneg fast double %41
  store double %42, double* %1, align 8
  %43 = sub i32 2500, %32
  %44 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %38, i64 1)
  store i32 %43, i32* %3, align 4
  call fastcc void @daxpy_(i32* nonnull %3, double* nonnull %1, double* %44, double* %29) #5
  %45 = add nuw i64 %31, 1
  %46 = icmp sgt i64 %45, 2500
  br i1 %46, label %47, label %30

47:                                               ; preds = %30
  ret void
}

define internal fastcc void @dmxpy_() unnamed_addr #3 {
  %1 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 1)
  %2 = load double, double* %1, align 1
  %3 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 1)
  %4 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 2)
  %5 = load double, double* %4, align 1
  %6 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 2)
  %7 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 3)
  %8 = load double, double* %7, align 1
  %9 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 3)
  %10 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 4)
  %11 = load double, double* %10, align 1
  %12 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 4)
  br label %13

13:                                               ; preds = %0, %13
  %14 = phi i64 [ 1, %0 ], [ %33, %13 ]
  %15 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %14)
  %16 = load double, double* %15, align 1
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %3, i64 %14)
  %18 = load double, double* %17, align 1
  %19 = fmul fast double %18, %2
  %20 = fadd fast double %19, %16
  %21 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %6, i64 %14)
  %22 = load double, double* %21, align 1
  %23 = fmul fast double %22, %5
  %24 = fadd fast double %20, %23
  %25 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %9, i64 %14)
  %26 = load double, double* %25, align 1
  %27 = fmul fast double %26, %8
  %28 = fadd fast double %24, %27
  %29 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %12, i64 %14)
  %30 = load double, double* %29, align 1
  %31 = fmul fast double %30, %11
  %32 = fadd fast double %28, %31
  store double %32, double* %15, align 1
  %33 = add nuw nsw i64 %14, 1
  %34 = icmp eq i64 %33, 2501
  br i1 %34, label %35, label %13

35:                                               ; preds = %13, %170
  %36 = phi i64 [ %171, %170 ], [ 20, %13 ]
  %37 = add nsw i64 %36, -15
  %38 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %37)
  %39 = load double, double* %38, align 1
  %40 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %37)
  %41 = add nsw i64 %36, -14
  %42 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %41)
  %43 = load double, double* %42, align 1
  %44 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %41)
  %45 = add nsw i64 %36, -13
  %46 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %45)
  %47 = load double, double* %46, align 1
  %48 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %45)
  %49 = add nsw i64 %36, -12
  %50 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %49)
  %51 = load double, double* %50, align 1
  %52 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %49)
  %53 = add nsw i64 %36, -11
  %54 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %53)
  %55 = load double, double* %54, align 1
  %56 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %53)
  %57 = add nsw i64 %36, -10
  %58 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %57)
  %59 = load double, double* %58, align 1
  %60 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %57)
  %61 = add nsw i64 %36, -9
  %62 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %61)
  %63 = load double, double* %62, align 1
  %64 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %61)
  %65 = add nsw i64 %36, -8
  %66 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %65)
  %67 = load double, double* %66, align 1
  %68 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %65)
  %69 = add nsw i64 %36, -7
  %70 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %69)
  %71 = load double, double* %70, align 1
  %72 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %69)
  %73 = add nsw i64 %36, -6
  %74 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %73)
  %75 = load double, double* %74, align 1
  %76 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %73)
  %77 = add nsw i64 %36, -5
  %78 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %77)
  %79 = load double, double* %78, align 1
  %80 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %77)
  %81 = add nsw i64 %36, -4
  %82 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %81)
  %83 = load double, double* %82, align 1
  %84 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %81)
  %85 = add nsw i64 %36, -3
  %86 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %85)
  %87 = load double, double* %86, align 1
  %88 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %85)
  %89 = add nsw i64 %36, -2
  %90 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %89)
  %91 = load double, double* %90, align 1
  %92 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %89)
  %93 = add nsw i64 %36, -1
  %94 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %93)
  %95 = load double, double* %94, align 1
  %96 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %93)
  %97 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %36)
  %98 = load double, double* %97, align 1
  %99 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %36)
  br label %100

100:                                              ; preds = %100, %35
  %101 = phi i64 [ 1, %35 ], [ %168, %100 ]
  %102 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %101)
  %103 = load double, double* %102, align 1
  %104 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %40, i64 %101)
  %105 = load double, double* %104, align 1
  %106 = fmul fast double %105, %39
  %107 = fadd fast double %106, %103
  %108 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %44, i64 %101)
  %109 = load double, double* %108, align 1
  %110 = fmul fast double %109, %43
  %111 = fadd fast double %107, %110
  %112 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %48, i64 %101)
  %113 = load double, double* %112, align 1
  %114 = fmul fast double %113, %47
  %115 = fadd fast double %111, %114
  %116 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %52, i64 %101)
  %117 = load double, double* %116, align 1
  %118 = fmul fast double %117, %51
  %119 = fadd fast double %115, %118
  %120 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %56, i64 %101)
  %121 = load double, double* %120, align 1
  %122 = fmul fast double %121, %55
  %123 = fadd fast double %119, %122
  %124 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %60, i64 %101)
  %125 = load double, double* %124, align 1
  %126 = fmul fast double %125, %59
  %127 = fadd fast double %123, %126
  %128 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %64, i64 %101)
  %129 = load double, double* %128, align 1
  %130 = fmul fast double %129, %63
  %131 = fadd fast double %127, %130
  %132 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %68, i64 %101)
  %133 = load double, double* %132, align 1
  %134 = fmul fast double %133, %67
  %135 = fadd fast double %131, %134
  %136 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %72, i64 %101)
  %137 = load double, double* %136, align 1
  %138 = fmul fast double %137, %71
  %139 = fadd fast double %135, %138
  %140 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %76, i64 %101)
  %141 = load double, double* %140, align 1
  %142 = fmul fast double %141, %75
  %143 = fadd fast double %139, %142
  %144 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %80, i64 %101)
  %145 = load double, double* %144, align 1
  %146 = fmul fast double %145, %79
  %147 = fadd fast double %143, %146
  %148 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %84, i64 %101)
  %149 = load double, double* %148, align 1
  %150 = fmul fast double %149, %83
  %151 = fadd fast double %147, %150
  %152 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %88, i64 %101)
  %153 = load double, double* %152, align 1
  %154 = fmul fast double %153, %87
  %155 = fadd fast double %151, %154
  %156 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %92, i64 %101)
  %157 = load double, double* %156, align 1
  %158 = fmul fast double %157, %91
  %159 = fadd fast double %155, %158
  %160 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %96, i64 %101)
  %161 = load double, double* %160, align 1
  %162 = fmul fast double %161, %95
  %163 = fadd fast double %159, %162
  %164 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %99, i64 %101)
  %165 = load double, double* %164, align 1
  %166 = fmul fast double %165, %98
  %167 = fadd fast double %163, %166
  store double %167, double* %102, align 1
  %168 = add nuw nsw i64 %101, 1
  %169 = icmp eq i64 %168, 2501
  br i1 %169, label %170, label %100

170:                                              ; preds = %100
  %171 = add nuw nsw i64 %36, 16
  %172 = icmp ugt i64 %36, 2484
  br i1 %172, label %173, label %35

173:                                              ; preds = %170
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr #0 {
  %1 = alloca [8 x i64], align 16
  %2 = alloca i32, align 8
  %3 = alloca [2 x i8], align 1
  %4 = alloca [4 x i8], align 1
  %5 = alloca { double }, align 8
  %6 = alloca [4 x i8], align 1
  %7 = alloca { double }, align 8
  %8 = alloca [4 x i8], align 1
  %9 = alloca { double }, align 8
  %10 = alloca [4 x i8], align 1
  %11 = alloca { double }, align 8
  %12 = alloca [4 x i8], align 1
  %13 = alloca { double }, align 8
  %14 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.179c04c108271c6ee1aba768bef6092b.0) #5
  br label %15

15:                                               ; preds = %31, %0
  %16 = phi i64 [ 1, %0 ], [ %32, %31 ]
  %17 = phi i32 [ 1325, %0 ], [ %23, %31 ]
  %18 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %16) #5
  br label %19

19:                                               ; preds = %19, %15
  %20 = phi i64 [ 1, %15 ], [ %29, %19 ]
  %21 = phi i32 [ %17, %15 ], [ %23, %19 ]
  %22 = mul nsw i32 %21, 3125
  %23 = srem i32 %22, 65536
  %24 = sitofp i32 %23 to float
  %25 = fmul fast float %24, 0x3F10000000000000
  %26 = fadd fast float %25, -2.000000e+00
  %27 = fpext float %26 to double
  %28 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %18, i64 %20) #5
  store double %27, double* %28, align 1
  %29 = add nuw nsw i64 %20, 1
  %30 = icmp eq i64 %29, 2501
  br i1 %30, label %31, label %19

31:                                               ; preds = %19
  %32 = add nuw nsw i64 %16, 1
  %33 = icmp eq i64 %32, 2501
  br i1 %33, label %34, label %15

34:                                               ; preds = %34, %31
  %35 = phi i64 [ %37, %34 ], [ 1, %31 ]
  %36 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %35) #5
  store double 0.000000e+00, double* %36, align 1
  %37 = add nuw nsw i64 %35, 1
  %38 = icmp eq i64 %37, 2501
  br i1 %38, label %39, label %34

39:                                               ; preds = %51, %34
  %40 = phi i64 [ %52, %51 ], [ 1, %34 ]
  %41 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %40) #5
  br label %42

42:                                               ; preds = %42, %39
  %43 = phi i64 [ 1, %39 ], [ %49, %42 ]
  %44 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %43) #5
  %45 = load double, double* %44, align 1
  %46 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %41, i64 %43) #5
  %47 = load double, double* %46, align 1
  %48 = fadd fast double %47, %45
  store double %48, double* %44, align 1
  %49 = add nuw nsw i64 %43, 1
  %50 = icmp eq i64 %49, 2501
  br i1 %50, label %51, label %42

51:                                               ; preds = %42
  %52 = add nuw nsw i64 %40, 1
  %53 = icmp eq i64 %52, 2501
  br i1 %53, label %54, label %39

54:                                               ; preds = %51
  call fastcc void @dgefa_(i32* nonnull %2) #5
  tail call fastcc void @dgesl_() #5
  br label %55

55:                                               ; preds = %55, %54
  %56 = phi i64 [ %62, %55 ], [ 1, %54 ]
  %57 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %56)
  %58 = bitcast double* %57 to i64*
  %59 = load i64, i64* %58, align 1
  %60 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %56)
  %61 = bitcast double* %60 to i64*
  store i64 %59, i64* %61, align 1
  %62 = add nuw nsw i64 %56, 1
  %63 = icmp eq i64 %62, 2501
  br i1 %63, label %64, label %55

64:                                               ; preds = %84, %55
  %65 = phi double [ %81, %84 ], [ 0.000000e+00, %55 ]
  %66 = phi i64 [ %85, %84 ], [ 1, %55 ]
  %67 = phi i32 [ %74, %84 ], [ 1325, %55 ]
  %68 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %66) #5
  br label %69

69:                                               ; preds = %69, %64
  %70 = phi i64 [ 1, %64 ], [ %82, %69 ]
  %71 = phi double [ %65, %64 ], [ %81, %69 ]
  %72 = phi i32 [ %67, %64 ], [ %74, %69 ]
  %73 = mul nsw i32 %72, 3125
  %74 = srem i32 %73, 65536
  %75 = sitofp i32 %74 to float
  %76 = fmul fast float %75, 0x3F10000000000000
  %77 = fadd fast float %76, -2.000000e+00
  %78 = fpext float %77 to double
  %79 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %68, i64 %70) #5
  store double %78, double* %79, align 1
  %80 = fcmp fast ole double %71, %78
  %81 = select fast i1 %80, double %78, double %71
  %82 = add nuw nsw i64 %70, 1
  %83 = icmp eq i64 %82, 2501
  br i1 %83, label %84, label %69

84:                                               ; preds = %69
  %85 = add nuw nsw i64 %66, 1
  %86 = icmp eq i64 %85, 2501
  br i1 %86, label %87, label %64

87:                                               ; preds = %87, %84
  %88 = phi i64 [ %90, %87 ], [ 1, %84 ]
  %89 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %88) #5
  store double 0.000000e+00, double* %89, align 1
  %90 = add nuw nsw i64 %88, 1
  %91 = icmp eq i64 %90, 2501
  br i1 %91, label %92, label %87

92:                                               ; preds = %104, %87
  %93 = phi i64 [ %105, %104 ], [ 1, %87 ]
  %94 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 20008, double* elementtype(double) getelementptr inbounds ([2500 x [2501 x double]], [2500 x [2501 x double]]* @"linpk_$A", i64 0, i64 0, i64 0), i64 %93) #5
  br label %95

95:                                               ; preds = %95, %92
  %96 = phi i64 [ 1, %92 ], [ %102, %95 ]
  %97 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %96) #5
  %98 = load double, double* %97, align 1
  %99 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %94, i64 %96) #5
  %100 = load double, double* %99, align 1
  %101 = fadd fast double %100, %98
  store double %101, double* %97, align 1
  %102 = add nuw nsw i64 %96, 1
  %103 = icmp eq i64 %102, 2501
  br i1 %103, label %104, label %95

104:                                              ; preds = %95
  %105 = add nuw nsw i64 %93, 1
  %106 = icmp eq i64 %105, 2501
  br i1 %106, label %107, label %92

107:                                              ; preds = %107, %104
  %108 = phi i64 [ %112, %107 ], [ 1, %104 ]
  %109 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %108)
  %110 = load double, double* %109, align 1
  %111 = fneg fast double %110
  store double %111, double* %109, align 1
  %112 = add nuw nsw i64 %108, 1
  %113 = icmp eq i64 %112, 2501
  br i1 %113, label %114, label %107

114:                                              ; preds = %107
  tail call fastcc void @dmxpy_() #5
  br label %115

115:                                              ; preds = %115, %114
  %116 = phi i64 [ %127, %115 ], [ 1, %114 ]
  %117 = phi double [ %122, %115 ], [ 0.000000e+00, %114 ]
  %118 = phi double [ %126, %115 ], [ 0.000000e+00, %114 ]
  %119 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$B", i64 0, i64 0), i64 %116)
  %120 = load double, double* %119, align 1
  %121 = tail call fast double @llvm.fabs.f64(double %120)
  %122 = tail call fast double @llvm.maxnum.f64(double %117, double %121)
  %123 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 %116)
  %124 = load double, double* %123, align 1
  %125 = tail call fast double @llvm.fabs.f64(double %124)
  %126 = tail call fast double @llvm.maxnum.f64(double %118, double %125)
  %127 = add nuw nsw i64 %116, 1
  %128 = icmp eq i64 %127, 2501
  br i1 %128, label %129, label %115

129:                                              ; preds = %115
  %130 = fmul fast double %81, 0x3D63880000000000
  %131 = fmul fast double %130, %126
  %132 = fdiv fast double %122, %131
  %133 = getelementptr inbounds [2 x i8], [2 x i8]* %3, i64 0, i64 0
  store i8 1, i8* %133, align 1
  %134 = getelementptr inbounds [2 x i8], [2 x i8]* %3, i64 0, i64 1
  store i8 0, i8* %134, align 1
  %135 = bitcast [8 x i64]* %1 to i8*
  %136 = call i32 (i8*, i32, i64, i8*, i8*, i8*, ...) @for_write_seq_fmt(i8* nonnull %135, i32 -1, i64 1239157112576, i8* nonnull %133, i8* null, i8* getelementptr inbounds ([120 x i8], [120 x i8]* @"linpk_$format_pack", i64 0, i64 32)) #5
  %137 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 0
  store i8 48, i8* %137, align 1
  %138 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 1
  store i8 1, i8* %138, align 1
  %139 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 2
  store i8 2, i8* %139, align 1
  %140 = getelementptr inbounds [4 x i8], [4 x i8]* %4, i64 0, i64 3
  store i8 0, i8* %140, align 1
  %141 = getelementptr inbounds { double }, { double }* %5, i64 0, i32 0
  store double %132, double* %141, align 8
  %142 = bitcast { double }* %5 to i8*
  %143 = call i32 (i8*, i32, i64, i8*, i8*, i8*, ...) @for_write_seq_fmt(i8* nonnull %135, i32 -1, i64 1239157112576, i8* nonnull %137, i8* nonnull %142, i8* getelementptr inbounds ([120 x i8], [120 x i8]* @"linpk_$format_pack", i64 0, i64 0)) #5
  %144 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 0
  store i8 48, i8* %144, align 1
  %145 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 1
  store i8 1, i8* %145, align 1
  %146 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 2
  store i8 2, i8* %146, align 1
  %147 = getelementptr inbounds [4 x i8], [4 x i8]* %6, i64 0, i64 3
  store i8 0, i8* %147, align 1
  %148 = getelementptr inbounds { double }, { double }* %7, i64 0, i32 0
  store double %122, double* %148, align 8
  %149 = bitcast { double }* %7 to i8*
  %150 = call i32 @for_write_seq_fmt_xmit(i8* nonnull %135, i8* nonnull %144, i8* nonnull %149) #5
  %151 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 0
  store i8 48, i8* %151, align 1
  %152 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 1
  store i8 1, i8* %152, align 1
  %153 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 2
  store i8 2, i8* %153, align 1
  %154 = getelementptr inbounds [4 x i8], [4 x i8]* %8, i64 0, i64 3
  store i8 0, i8* %154, align 1
  %155 = getelementptr inbounds { double }, { double }* %9, i64 0, i32 0
  store double 0x3CB0000000000000, double* %155, align 8
  %156 = bitcast { double }* %9 to i8*
  %157 = call i32 @for_write_seq_fmt_xmit(i8* nonnull %135, i8* nonnull %151, i8* nonnull %156) #5
  %158 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 1)
  %159 = bitcast double* %158 to i64*
  %160 = load i64, i64* %159, align 1
  %161 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 0
  store i8 48, i8* %161, align 1
  %162 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 1
  store i8 1, i8* %162, align 1
  %163 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 2
  store i8 2, i8* %163, align 1
  %164 = getelementptr inbounds [4 x i8], [4 x i8]* %10, i64 0, i64 3
  store i8 0, i8* %164, align 1
  %165 = bitcast { double }* %11 to i64*
  store i64 %160, i64* %165, align 8
  %166 = bitcast { double }* %11 to i8*
  %167 = call i32 @for_write_seq_fmt_xmit(i8* nonnull %135, i8* nonnull %161, i8* nonnull %166) #5
  %168 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([2500 x double], [2500 x double]* @"linpk_$X", i64 0, i64 0), i64 2500)
  %169 = bitcast double* %168 to i64*
  %170 = load i64, i64* %169, align 1
  %171 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 0
  store i8 48, i8* %171, align 1
  %172 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 1
  store i8 1, i8* %172, align 1
  %173 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 2
  store i8 1, i8* %173, align 1
  %174 = getelementptr inbounds [4 x i8], [4 x i8]* %12, i64 0, i64 3
  store i8 0, i8* %174, align 1
  %175 = bitcast { double }* %13 to i64*
  store i64 %170, i64* %175, align 8
  %176 = bitcast { double }* %13 to i8*
  %177 = call i32 @for_write_seq_fmt_xmit(i8* nonnull %135, i8* nonnull %171, i8* nonnull %176) #5
  ret void
}

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 %0, i64 %1, i64 %2, double* %3, i64 %4)

declare double @llvm.fabs.f64(double %0)

declare double @llvm.maxnum.f64(double %0, double %1)

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 %0, i64 %1, i64 %2, i32* %3, i64 %4)

declare dso_local i32 @for_set_reentrancy(i32* %0) local_unnamed_addr

declare dso_local i32 @for_write_seq_fmt(i8* %0, i32 %1, i64 %2, i8* %3, i8* %4, i8* %5, ...) local_unnamed_addr

declare dso_local i32 @for_write_seq_fmt_xmit(i8* %0, i8* %1, i8* %2) local_unnamed_addr

@anon.179c04c108271c6ee1aba768bef6092b.0 = internal unnamed_addr constant i32 2
@"linpk_$A" = internal unnamed_addr global [2500 x [2501 x double]] zeroinitializer, align 16
@"linpk_$B" = internal unnamed_addr global [2500 x double] zeroinitializer, align 16
@"linpk_$IPVT" = internal unnamed_addr global [2500 x i32] zeroinitializer, align 16
@"linpk_$X" = internal unnamed_addr global [2500 x double] zeroinitializer, align 16
@"linpk_$format_pack" = internal unnamed_addr global [120 x i8] c"6\00\00\00\0A\00\00\00\01\00\00\00\01\00\00\00\1E\00\00\08\05\00\00\00\10\00\00\007\00\00\006\00\00\00\1C\00,\00     norm. resid      resid           machep\1C\00\1B\00         x(1)          x(n)\007\00\00\00", align 4

attributes #0 = { "intel-lang"="fortran" }

; end INTEL_FEATURE_SW_ADVANCED
