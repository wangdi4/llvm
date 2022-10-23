; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that @perdida_m_mp_perdida_ is inlined due to the single callsite local
; linkage heuristic, but @perdida_m_mp_generalized_hookes_law_ is not inlined
; due to the inline budget heuristic because the calls to it are not contained
; within loops.

; CHECK-BEFORE-NOT: call{{.*}}@perdida_m_mp_perdida_
; CHECK-BEFORE: call{{.*}}@perdida_m_mp_generalized_hookes_law_
; CHECK-BEFORE: call{{.*}}@perdida_m_mp_generalized_hookes_law_
; CHECK: INLINE: perdida_m_mp_perdida_{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: INLINE: perdida_m_mp_generalized_hookes_law_{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: INLINE: perdida_m_mp_generalized_hookes_law_{{.*}}Has inline budget for small application
; CHECK-AFTER-NOT: call{{.*}}@perdida_m_mp_perdida_
; CHECK-AFTER: call{{.*}}@perdida_m_mp_generalized_hookes_law_
; CHECK-AFTER: call{{.*}}@perdida_m_mp_generalized_hookes_law_

%"QNCA_a0$double*$rank3$" = type { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank1$" = type { double*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$i32*$rank1$" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@anon.58475115b99d6a1547144f0b5d7ba7df.120 = internal unnamed_addr constant [0 x i8] zeroinitializer
@anon.58475115b99d6a1547144f0b5d7ba7df.78 = internal unnamed_addr constant [43 x i8] c"discriminant is negative in perdida, abort."
@anon.58475115b99d6a1547144f0b5d7ba7df.77 = internal unnamed_addr constant [46 x i8] c"bad plastic strain fraction in perdida, abort."

@"iztaccihuatl_$DAMAGE" = internal global %"QNCA_a0$double*$rank1$" { double* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"iztaccihuatl_$ACCUMULATED_PLASTIC_STRAIN" = internal global %"QNCA_a0$double*$rank1$" { double* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"iztaccihuatl_$ISOTROPIC_HARDENING_STRESS" = internal global %"QNCA_a0$double*$rank1$" { double* null, i64 0, i64 0, i64 1073741952, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }

declare dso_local i32 @for_stop_core_quiet(i8* %0, i32 %1, i32 %2, i64 %3, i32 %4, i32 %5, ...)

declare dso_local i32 @for_write_seq_lis(i8* %0, i32 %1, i64 %2, i8* %3, i8* %4, ...)

declare dso_local i32 @for_write_seq_lis_xmit(i8* %0, i8* %1, i8* %2)

declare void @llvm.stackrestore(i8* %0)

declare i8* @llvm.stacksave()

@"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR" = internal unnamed_addr global [6 x [6 x double]] zeroinitializer, align 16
@"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR" = internal unnamed_addr global [6 x double] zeroinitializer, align 16
@"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR" = internal unnamed_addr global [6 x double] zeroinitializer, align 16

@"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$OLD_PLASTIC_STRAIN_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16
@"perdida_m_mp_perdida_$BACK_STRESS_RATE_TENSOR" = internal unnamed_addr global [3 x [3 x double]] zeroinitializer, align 16

define internal fastcc void @perdida_m_mp_generalized_hookes_law_(%"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) %0, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %1, double* noalias nocapture readonly %2, double* noalias nocapture readonly %3) unnamed_addr #0 {
  %5 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %1, i64 0, i32 0
  br label %14

6:                                                ; preds = %14, %6
  %7 = phi i64 [ 1, %14 ], [ %9, %6 ]
  %8 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %16, i64 %7)
  store double 0.000000e+00, double* %8, align 1
  %9 = add nuw nsw i64 %7, 1
  %10 = icmp eq i64 %9, 7
  br i1 %10, label %11, label %6

11:                                               ; preds = %6
  %12 = add nuw nsw i64 %15, 1
  %13 = icmp eq i64 %12, 7
  br i1 %13, label %17, label %14

14:                                               ; preds = %11, %4
  %15 = phi i64 [ 1, %4 ], [ %12, %11 ]
  %16 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 %15)
  br label %6

17:                                               ; preds = %11
  %18 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %1, i64 0, i32 6, i64 0, i32 1
  %19 = load double, double* %2, align 1
  %20 = load double, double* %3, align 1
  %21 = fmul fast double %20, 2.000000e+00
  %22 = fadd fast double %21, %19
  %23 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 1)
  %24 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %23, i64 1)
  store double %22, double* %24, align 1
  %25 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 2)
  %26 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %25, i64 1)
  store double %19, double* %26, align 1
  %27 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 3)
  %28 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %27, i64 1)
  store double %19, double* %28, align 1
  %29 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %23, i64 2)
  store double %19, double* %29, align 1
  %30 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %25, i64 2)
  store double %22, double* %30, align 1
  %31 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %27, i64 2)
  store double %19, double* %31, align 1
  %32 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %23, i64 3)
  store double %19, double* %32, align 1
  %33 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %25, i64 3)
  store double %19, double* %33, align 1
  %34 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %27, i64 3)
  store double %22, double* %34, align 1
  %35 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 4)
  %36 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %35, i64 4)
  store double %20, double* %36, align 1
  %37 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 5)
  %38 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %37, i64 5)
  store double %20, double* %38, align 1
  %39 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 6)
  %40 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %39, i64 6)
  store double %20, double* %40, align 1
  %41 = load double*, double** %5, align 1
  %42 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %18, i32 0)
  %43 = load i64, i64* %42, align 1
  %44 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %18, i32 1)
  %45 = load i64, i64* %44, align 1
  %46 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %45, double* elementtype(double) %41, i64 1)
  %47 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %43, double* elementtype(double) %46, i64 1)
  %48 = bitcast double* %47 to i64*
  %49 = load i64, i64* %48, align 1
  %50 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 0, i64 0), i64 1)
  %51 = bitcast double* %50 to i64*
  store i64 %49, i64* %51, align 1
  %52 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %45, double* elementtype(double) %41, i64 2)
  %53 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %43, double* elementtype(double) %52, i64 2)
  %54 = bitcast double* %53 to i64*
  %55 = load i64, i64* %54, align 1
  %56 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 0, i64 0), i64 2)
  %57 = bitcast double* %56 to i64*
  store i64 %55, i64* %57, align 1
  %58 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %45, double* elementtype(double) %41, i64 3)
  %59 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %43, double* elementtype(double) %58, i64 3)
  %60 = bitcast double* %59 to i64*
  %61 = load i64, i64* %60, align 1
  %62 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 0, i64 0), i64 3)
  %63 = bitcast double* %62 to i64*
  store i64 %61, i64* %63, align 1
  %64 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %43, double* elementtype(double) %58, i64 2)
  %65 = bitcast double* %64 to i64*
  %66 = load i64, i64* %65, align 1
  %67 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 0, i64 0), i64 4)
  %68 = bitcast double* %67 to i64*
  store i64 %66, i64* %68, align 1
  %69 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %43, double* elementtype(double) %58, i64 1)
  %70 = bitcast double* %69 to i64*
  %71 = load i64, i64* %70, align 1
  %72 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 0, i64 0), i64 5)
  %73 = bitcast double* %72 to i64*
  store i64 %71, i64* %73, align 1
  %74 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %43, double* elementtype(double) %52, i64 1)
  %75 = bitcast double* %74 to i64*
  %76 = load i64, i64* %75, align 1
  %77 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 0, i64 0), i64 6)
  %78 = bitcast double* %77 to i64*
  store i64 %76, i64* %78, align 1
  br label %79

79:                                               ; preds = %93, %17
  %80 = phi i64 [ %95, %93 ], [ 1, %17 ]
  br label %81

81:                                               ; preds = %81, %79
  %82 = phi double [ 0.000000e+00, %79 ], [ %90, %81 ]
  %83 = phi i64 [ 1, %79 ], [ %91, %81 ]
  %84 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 48, double* elementtype(double) getelementptr inbounds ([6 x [6 x double]], [6 x [6 x double]]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_CONSTITUTIVE_TENSOR", i64 0, i64 0, i64 0), i64 %83)
  %85 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %84, i64 %80)
  %86 = load double, double* %85, align 1
  %87 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRAIN_VECTOR", i64 0, i64 0), i64 %83)
  %88 = load double, double* %87, align 1
  %89 = fmul fast double %88, %86
  %90 = fadd fast double %89, %82
  %91 = add nuw nsw i64 %83, 1
  %92 = icmp eq i64 %91, 7
  br i1 %92, label %93, label %81

93:                                               ; preds = %81
  %94 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 0, i64 0), i64 %80)
  store double %90, double* %94, align 1
  %95 = add nuw nsw i64 %80, 1
  %96 = icmp eq i64 %95, 7
  br i1 %96, label %97, label %79

97:                                               ; preds = %93
  %98 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %0, i64 0, i32 0
  %99 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 0, i64 0), i64 1)
  %100 = bitcast double* %99 to i64*
  %101 = load i64, i64* %100, align 1
  %102 = load double*, double** %98, align 1
  %103 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) %102, i64 1)
  %104 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %103, i64 1)
  %105 = bitcast double* %104 to i64*
  store i64 %101, i64* %105, align 1
  %106 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 0, i64 0), i64 2)
  %107 = bitcast double* %106 to i64*
  %108 = load i64, i64* %107, align 1
  %109 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) %102, i64 2)
  %110 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %109, i64 2)
  %111 = bitcast double* %110 to i64*
  store i64 %108, i64* %111, align 1
  %112 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 0, i64 0), i64 3)
  %113 = bitcast double* %112 to i64*
  %114 = load i64, i64* %113, align 1
  %115 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) %102, i64 3)
  %116 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %115, i64 3)
  %117 = bitcast double* %116 to i64*
  store i64 %114, i64* %117, align 1
  %118 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 0, i64 0), i64 4)
  %119 = bitcast double* %118 to i64*
  %120 = load i64, i64* %119, align 1
  %121 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %115, i64 2)
  %122 = bitcast double* %121 to i64*
  store i64 %120, i64* %122, align 1
  %123 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 0, i64 0), i64 5)
  %124 = bitcast double* %123 to i64*
  %125 = load i64, i64* %124, align 1
  %126 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %115, i64 1)
  %127 = bitcast double* %126 to i64*
  store i64 %125, i64* %127, align 1
  %128 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) getelementptr inbounds ([6 x double], [6 x double]* @"perdida_m_mp_generalized_hookes_law_$GENERALIZED_STRESS_VECTOR", i64 0, i64 0), i64 6)
  %129 = bitcast double* %128 to i64*
  %130 = load i64, i64* %129, align 1
  %131 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %109, i64 1)
  %132 = bitcast double* %131 to i64*
  store i64 %130, i64* %132, align 1
  %133 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %109, i64 3)
  %134 = bitcast double* %133 to i64*
  store i64 %120, i64* %134, align 1
  %135 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %103, i64 3)
  %136 = bitcast double* %135 to i64*
  store i64 %125, i64* %136, align 1
  %137 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %103, i64 2)
  %138 = bitcast double* %137 to i64*
  store i64 %130, i64* %138, align 1
  ret void
}

define internal fastcc void @perdida_m_mp_perdida_(double* noalias nocapture readonly %0, double* noalias nocapture readonly %1, double* noalias nocapture readonly %2, double* noalias nocapture readonly %3, double* noalias nocapture readonly %4, double* noalias nocapture readonly %5, double* noalias nocapture readonly %6, double* noalias nocapture readonly %7, double* noalias nocapture readonly %8, double* noalias nocapture readonly %9, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %10, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %11, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %12, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %13, double* noalias nocapture %14, %"QNCA_a0$double*$rank2$"* noalias nocapture readonly dereferenceable(96) "ptrnoalias" %15, double* noalias nocapture %16, double* noalias nocapture %17, double* noalias nocapture readonly %18, double* noalias nocapture readonly %19) unnamed_addr #0 {
  %21 = alloca [8 x i64], align 16
  %22 = alloca %"QNCA_a0$double*$rank2$", align 8
  %23 = alloca [9 x double], align 16
  %24 = alloca %"QNCA_a0$double*$rank2$", align 8
  %25 = alloca %"QNCA_a0$double*$rank2$", align 8
  %26 = alloca [9 x double], align 16
  %27 = alloca %"QNCA_a0$double*$rank2$", align 8
  %28 = alloca [4 x i8], align 1
  %29 = alloca { i64, i8* }, align 8
  %30 = alloca [4 x i8], align 1
  %31 = alloca { i64, i8* }, align 8
  %32 = alloca [4 x i8], align 1
  %33 = alloca { double }, align 8
  %34 = getelementptr inbounds [9 x double], [9 x double]* %26, i64 0, i64 0
  %35 = getelementptr inbounds [9 x double], [9 x double]* %23, i64 0, i64 0
  %36 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %10, i64 0, i32 0
  %37 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %10, i64 0, i32 6, i64 0, i32 0
  %38 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %10, i64 0, i32 6, i64 0, i32 1
  %39 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %11, i64 0, i32 0
  %40 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %11, i64 0, i32 6, i64 0, i32 0
  %41 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %11, i64 0, i32 6, i64 0, i32 1
  %42 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %12, i64 0, i32 0
  %43 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %12, i64 0, i32 6, i64 0, i32 1
  %44 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 0
  %45 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 1
  %46 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 2
  %47 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 3
  %48 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 4
  %49 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 6, i64 0, i32 0
  %50 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 6, i64 0, i32 1
  %51 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %22, i64 0, i32 6, i64 0, i32 2
  %52 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 0
  %53 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 1
  %54 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 2
  %55 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 3
  %56 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 4
  %57 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0, i32 0
  %58 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0, i32 1
  %59 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %24, i64 0, i32 6, i64 0, i32 2
  %60 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %15, i64 0, i32 0
  %61 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %15, i64 0, i32 6, i64 0, i32 0
  %62 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %15, i64 0, i32 6, i64 0, i32 1
  %63 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %13, i64 0, i32 0
  %64 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %13, i64 0, i32 6, i64 0, i32 0
  %65 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %13, i64 0, i32 6, i64 0, i32 1
  %66 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 1
  %67 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 2
  %68 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 3
  %69 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 4
  %70 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 6, i64 0, i32 0
  %71 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 6, i64 0, i32 1
  %72 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %25, i64 0, i32 6, i64 0, i32 2
  %73 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 0
  %74 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 1
  %75 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 2
  %76 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 3
  %77 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 4
  %78 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 6, i64 0, i32 0
  %79 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 6, i64 0, i32 1
  %80 = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %27, i64 0, i32 6, i64 0, i32 2
  %81 = load double, double* %1, align 1
  %82 = fmul fast double %81, 5.000000e-01
  %83 = load double, double* %2, align 1
  %84 = fadd fast double %83, %81
  %85 = fdiv fast double %82, %84
  %86 = fmul fast double %85, 2.000000e+00
  %87 = fsub fast double 1.000000e+00, %86
  %88 = tail call i8* @llvm.stacksave()
  %89 = load double*, double** %36, align 1
  %90 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %38, i32 0)
  %91 = load i64, i64* %90, align 1
  %92 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %37, i32 0)
  %93 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %38, i32 1)
  %94 = load i64, i64* %93, align 1
  %95 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %37, i32 1)
  %96 = load double, double* %17, align 1
  %97 = load double*, double** %39, align 1
  %98 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %41, i32 0)
  %99 = load i64, i64* %98, align 1
  %100 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %40, i32 0)
  %101 = load i64, i64* %100, align 1
  %102 = icmp sgt i64 %101, 0
  %103 = select i1 %102, i64 %101, i64 0
  %104 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %41, i32 1)
  %105 = load i64, i64* %104, align 1
  %106 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %40, i32 1)
  %107 = load i64, i64* %106, align 1
  %108 = icmp sgt i64 %107, 0
  %109 = select i1 %108, i64 %107, i64 0
  %110 = load double*, double** %42, align 1
  %111 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %43, i32 0)
  %112 = load i64, i64* %111, align 1
  %113 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %43, i32 1)
  %114 = load i64, i64* %113, align 1
  %115 = shl nsw i64 %103, 3
  %116 = mul nsw i64 %109, %115
  %117 = lshr exact i64 %116, 3
  %118 = alloca double, i64 %117, align 1
  %119 = icmp slt i64 %107, 1
  %120 = icmp slt i64 %101, 1
  %121 = or i1 %119, %120
  br i1 %121, label %143, label %122

122:                                              ; preds = %20
  %123 = add nuw nsw i64 %101, 1
  %124 = add nuw nsw i64 %107, 1
  br label %138

125:                                              ; preds = %138, %125
  %126 = phi i64 [ 1, %138 ], [ %133, %125 ]
  %127 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %99, double* elementtype(double) %140, i64 %126)
  %128 = load double, double* %127, align 1
  %129 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %112, double* elementtype(double) %141, i64 %126)
  %130 = load double, double* %129, align 1
  %131 = fsub fast double %128, %130
  %132 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %142, i64 %126)
  store double %131, double* %132, align 1
  %133 = add nuw nsw i64 %126, 1
  %134 = icmp eq i64 %133, %123
  br i1 %134, label %135, label %125

135:                                              ; preds = %125
  %136 = add nuw nsw i64 %139, 1
  %137 = icmp eq i64 %136, %124
  br i1 %137, label %143, label %138

138:                                              ; preds = %135, %122
  %139 = phi i64 [ %136, %135 ], [ 1, %122 ]
  %140 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %105, double* elementtype(double) %97, i64 %139)
  %141 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %114, double* elementtype(double) %110, i64 %139)
  %142 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %115, double* elementtype(double) nonnull %118, i64 %139)
  br label %125

143:                                              ; preds = %135, %20
  store i64 8, i64* %45, align 8
  store i64 2, i64* %48, align 8
  store i64 0, i64* %46, align 8
  %144 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %50, i32 0)
  store i64 8, i64* %144, align 1
  %145 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %51, i32 0)
  store i64 1, i64* %145, align 1
  %146 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %49, i32 0)
  store i64 %103, i64* %146, align 1
  %147 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %50, i32 1)
  store i64 %115, i64* %147, align 1
  %148 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %51, i32 1)
  store i64 1, i64* %148, align 1
  %149 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %49, i32 1)
  store i64 %109, i64* %149, align 1
  store double* %118, double** %44, align 8
  store i64 1, i64* %47, align 8
  store i64 8, i64* %53, align 8
  store i64 2, i64* %56, align 8
  store i64 0, i64* %54, align 8
  %150 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %58, i32 0)
  store i64 8, i64* %150, align 1
  %151 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %59, i32 0)
  store i64 1, i64* %151, align 1
  %152 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %57, i32 0)
  store i64 3, i64* %152, align 1
  %153 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %58, i32 1)
  store i64 24, i64* %153, align 1
  %154 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %59, i32 1)
  store i64 1, i64* %154, align 1
  %155 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %57, i32 1)
  store i64 3, i64* %155, align 1
  store double* %35, double** %52, align 8
  store i64 1, i64* %55, align 8
  call fastcc void @perdida_m_mp_generalized_hookes_law_(%"QNCA_a0$double*$rank2$"* nonnull %24, %"QNCA_a0$double*$rank2$"* nonnull %22, double* nonnull %1, double* nonnull %2)
  %156 = fsub fast double 1.000000e+00, %96
  br label %168

157:                                              ; preds = %168, %157
  %158 = phi i64 [ 1, %168 ], [ %163, %157 ]
  %159 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %170, i64 %158)
  %160 = load double, double* %159, align 1
  %161 = fmul fast double %160, %156
  %162 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %91, double* elementtype(double) %171, i64 %158)
  store double %161, double* %162, align 1
  %163 = add nuw nsw i64 %158, 1
  %164 = icmp eq i64 %163, 4
  br i1 %164, label %165, label %157

165:                                              ; preds = %157
  %166 = add nuw nsw i64 %169, 1
  %167 = icmp eq i64 %166, 4
  br i1 %167, label %172, label %168

168:                                              ; preds = %165, %143
  %169 = phi i64 [ 1, %143 ], [ %166, %165 ]
  %170 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) nonnull %35, i64 %169)
  %171 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %94, double* elementtype(double) %89, i64 %169)
  br label %157

172:                                              ; preds = %165
  call void @llvm.stackrestore(i8* %88)
  %173 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %94, double* elementtype(double) nonnull %89, i64 1)
  %174 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %91, double* elementtype(double) %173, i64 1)
  %175 = load double, double* %174, align 1
  %176 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %94, double* elementtype(double) nonnull %89, i64 2)
  %177 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %91, double* elementtype(double) %176, i64 2)
  %178 = load double, double* %177, align 1
  %179 = fadd fast double %178, %175
  %180 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %94, double* elementtype(double) nonnull %89, i64 3)
  %181 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %91, double* elementtype(double) %180, i64 3)
  %182 = load double, double* %181, align 1
  %183 = fadd fast double %179, %182
  %184 = fmul fast double %183, 0x3FD5555555555555
  %185 = fcmp fast olt double %184, -1.000000e-10
  %186 = fcmp fast ogt double %96, 0.000000e+00
  %187 = and i1 %186, %185
  br i1 %187, label %209, label %237

188:                                              ; preds = %199, %188
  %189 = phi i64 [ 1, %199 ], [ %194, %188 ]
  %190 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %91, double* elementtype(double) %201, i64 %189)
  %191 = load double, double* %190, align 1
  %192 = fmul fast double %191, %217
  %193 = fdiv fast double %192, %156
  store double %193, double* %190, align 1
  %194 = add nuw nsw i64 %189, 1
  %195 = icmp eq i64 %194, %219
  br i1 %195, label %196, label %188

196:                                              ; preds = %188
  %197 = add nuw nsw i64 %200, 1
  %198 = icmp eq i64 %197, %220
  br i1 %198, label %202, label %199

199:                                              ; preds = %218, %196
  %200 = phi i64 [ %197, %196 ], [ 1, %218 ]
  %201 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %94, double* elementtype(double) nonnull %89, i64 %200)
  br label %188

202:                                              ; preds = %214, %209, %196
  %203 = load double, double* %174, align 1
  %204 = load double, double* %177, align 1
  %205 = fadd fast double %204, %203
  %206 = load double, double* %181, align 1
  %207 = fadd fast double %205, %206
  %208 = fmul fast double %207, 0x3FD5555555555555
  br label %237

209:                                              ; preds = %172
  %210 = load double, double* %19, align 1
  %211 = load i64, i64* %92, align 1
  %212 = load i64, i64* %95, align 1
  %213 = icmp slt i64 %212, 1
  br i1 %213, label %202, label %214

214:                                              ; preds = %209
  %215 = icmp slt i64 %211, 1
  %216 = fmul fast double %210, %96
  %217 = fsub fast double 1.000000e+00, %216
  br i1 %215, label %202, label %218

218:                                              ; preds = %214
  %219 = add nuw nsw i64 %211, 1
  %220 = add nuw nsw i64 %212, 1
  br label %199

221:                                              ; preds = %233, %221
  %222 = phi i64 [ 1, %233 ], [ %228, %221 ]
  %223 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %91, double* elementtype(double) %235, i64 %222)
  %224 = bitcast double* %223 to i64*
  %225 = load i64, i64* %224, align 1
  %226 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %236, i64 %222)
  %227 = bitcast double* %226 to i64*
  store i64 %225, i64* %227, align 1
  %228 = add nuw nsw i64 %222, 1
  %229 = icmp eq i64 %228, 4
  br i1 %229, label %230, label %221

230:                                              ; preds = %221
  %231 = add nuw nsw i64 %234, 1
  %232 = icmp eq i64 %231, 4
  br i1 %232, label %243, label %233

233:                                              ; preds = %237, %230
  %234 = phi i64 [ 1, %237 ], [ %231, %230 ]
  %235 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %94, double* elementtype(double) nonnull %89, i64 %234)
  %236 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %234)
  br label %221

237:                                              ; preds = %202, %172
  %238 = phi double [ %206, %202 ], [ %182, %172 ]
  %239 = phi double [ %204, %202 ], [ %178, %172 ]
  %240 = phi double [ %203, %202 ], [ %175, %172 ]
  %241 = phi double [ %208, %202 ], [ %184, %172 ]
  %242 = phi double [ %210, %202 ], [ 1.000000e+00, %172 ]
  br label %233

243:                                              ; preds = %230
  %244 = fsub fast double %240, %241
  %245 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 1)
  %246 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %245, i64 1)
  store double %244, double* %246, align 1
  %247 = fsub fast double %239, %241
  %248 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 2)
  %249 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %248, i64 2)
  store double %247, double* %249, align 1
  %250 = fsub fast double %238, %241
  %251 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 3)
  %252 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %251, i64 3)
  store double %250, double* %252, align 1
  %253 = fmul fast double %242, %96
  %254 = fsub fast double 1.000000e+00, %253
  br label %266

255:                                              ; preds = %266, %255
  %256 = phi i64 [ 1, %266 ], [ %261, %255 ]
  %257 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %268, i64 %256)
  %258 = load double, double* %257, align 1
  %259 = fdiv fast double %258, %254
  %260 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %269, i64 %256)
  store double %259, double* %260, align 1
  %261 = add nuw nsw i64 %256, 1
  %262 = icmp eq i64 %261, 4
  br i1 %262, label %263, label %255

263:                                              ; preds = %255
  %264 = add nuw nsw i64 %267, 1
  %265 = icmp eq i64 %264, 4
  br i1 %265, label %302, label %266

266:                                              ; preds = %263, %243
  %267 = phi i64 [ 1, %243 ], [ %264, %263 ]
  %268 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %267)
  %269 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %267)
  br label %255

270:                                              ; preds = %285, %270
  %271 = phi double [ %286, %285 ], [ %279, %270 ]
  %272 = phi i64 [ 1, %285 ], [ %280, %270 ]
  %273 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %288, i64 %272)
  %274 = load double, double* %273, align 1
  %275 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %305, double* elementtype(double) %289, i64 %272)
  %276 = load double, double* %275, align 1
  %277 = fsub fast double %274, %276
  %278 = fmul fast double %277, %277
  %279 = fadd fast double %278, %271
  %280 = add nuw nsw i64 %272, 1
  %281 = icmp eq i64 %280, 4
  br i1 %281, label %282, label %270

282:                                              ; preds = %270
  %283 = add nuw nsw i64 %287, 1
  %284 = icmp eq i64 %283, 4
  br i1 %284, label %290, label %285

285:                                              ; preds = %302, %282
  %286 = phi double [ 0.000000e+00, %302 ], [ %279, %282 ]
  %287 = phi i64 [ 1, %302 ], [ %283, %282 ]
  %288 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %287)
  %289 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %307, double* elementtype(double) %303, i64 %287)
  br label %270

290:                                              ; preds = %282
  %291 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %61, i32 0)
  %292 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %61, i32 1)
  %293 = fmul fast double %279, 1.500000e+00
  %294 = call fast double @llvm.sqrt.f64(double %293)
  %295 = load double, double* %16, align 1
  %296 = load double, double* %3, align 1
  %297 = fadd fast double %296, %295
  %298 = fsub fast double %294, %297
  %299 = fcmp fast olt double %298, 0.000000e+00
  %300 = fcmp fast olt double %294, 1.000000e-10
  %301 = or i1 %300, %299
  br i1 %301, label %778, label %381

302:                                              ; preds = %263
  %303 = load double*, double** %60, align 1
  %304 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %62, i32 0)
  %305 = load i64, i64* %304, align 1
  %306 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %62, i32 1)
  %307 = load i64, i64* %306, align 1
  br label %285

308:                                              ; preds = %320, %308
  %309 = phi i64 [ 1, %320 ], [ %315, %308 ]
  %310 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %384, double* elementtype(double) %322, i64 %309)
  %311 = bitcast double* %310 to i64*
  %312 = load i64, i64* %311, align 1
  %313 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %323, i64 %309)
  %314 = bitcast double* %313 to i64*
  store i64 %312, i64* %314, align 1
  %315 = add nuw nsw i64 %309, 1
  %316 = icmp eq i64 %315, 4
  br i1 %316, label %317, label %308

317:                                              ; preds = %308
  %318 = add nuw nsw i64 %321, 1
  %319 = icmp eq i64 %318, 4
  br i1 %319, label %324, label %320

320:                                              ; preds = %381, %317
  %321 = phi i64 [ 1, %381 ], [ %318, %317 ]
  %322 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %386, double* elementtype(double) %382, i64 %321)
  %323 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 %321)
  br label %308

324:                                              ; preds = %317
  %325 = fmul fast double %397, 0x3FD5555555555555
  %326 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %64, i32 1)
  %327 = fsub fast double %389, %325
  %328 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 1)
  %329 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %328, i64 1)
  store double %327, double* %329, align 1
  %330 = fsub fast double %392, %325
  %331 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 2)
  %332 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %331, i64 2)
  store double %330, double* %332, align 1
  %333 = fsub fast double %396, %325
  %334 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 3)
  %335 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %334, i64 3)
  store double %333, double* %335, align 1
  %336 = load i64, i64* %398, align 1
  %337 = icmp sgt i64 %336, 0
  %338 = select i1 %337, i64 %336, i64 0
  %339 = load i64, i64* %326, align 1
  %340 = icmp sgt i64 %339, 0
  %341 = select i1 %340, i64 %339, i64 0
  store i64 8, i64* %66, align 8
  store i64 2, i64* %69, align 8
  store i64 0, i64* %67, align 8
  %342 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %71, i32 0)
  store i64 %384, i64* %342, align 1
  %343 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %72, i32 0)
  store i64 1, i64* %343, align 1
  %344 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %70, i32 0)
  store i64 %338, i64* %344, align 1
  %345 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %71, i32 1)
  store i64 %386, i64* %345, align 1
  %346 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %72, i32 1)
  store i64 1, i64* %346, align 1
  %347 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %70, i32 1)
  store i64 %341, i64* %347, align 1
  %348 = bitcast %"QNCA_a0$double*$rank2$"* %25 to i64*
  store i64 %399, i64* %348, align 8
  store i64 1, i64* %68, align 8
  store i64 8, i64* %74, align 8
  store i64 2, i64* %77, align 8
  store i64 0, i64* %75, align 8
  %349 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %79, i32 0)
  store i64 8, i64* %349, align 1
  %350 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %80, i32 0)
  store i64 1, i64* %350, align 1
  %351 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %78, i32 0)
  store i64 3, i64* %351, align 1
  %352 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %79, i32 1)
  store i64 24, i64* %352, align 1
  %353 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %80, i32 1)
  store i64 1, i64* %353, align 1
  %354 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %78, i32 1)
  store i64 3, i64* %354, align 1
  store double* %34, double** %73, align 8
  store i64 1, i64* %76, align 8
  call fastcc void @perdida_m_mp_generalized_hookes_law_(%"QNCA_a0$double*$rank2$"* nonnull %27, %"QNCA_a0$double*$rank2$"* nonnull %25, double* nonnull %1, double* nonnull %2)
  br label %372

355:                                              ; preds = %372, %355
  %356 = phi double [ %373, %372 ], [ %366, %355 ]
  %357 = phi i64 [ 1, %372 ], [ %367, %355 ]
  %358 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %375, i64 %357)
  %359 = load double, double* %358, align 1
  %360 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %305, double* elementtype(double) %376, i64 %357)
  %361 = load double, double* %360, align 1
  %362 = fsub fast double %359, %361
  %363 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %377, i64 %357)
  %364 = load double, double* %363, align 1
  %365 = fmul fast double %362, %364
  %366 = fadd fast double %365, %356
  %367 = add nuw nsw i64 %357, 1
  %368 = icmp eq i64 %367, 4
  br i1 %368, label %369, label %355

369:                                              ; preds = %355
  %370 = add nuw nsw i64 %374, 1
  %371 = icmp eq i64 %370, 4
  br i1 %371, label %378, label %372

372:                                              ; preds = %369, %324
  %373 = phi double [ 0.000000e+00, %324 ], [ %366, %369 ]
  %374 = phi i64 [ 1, %324 ], [ %370, %369 ]
  %375 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %374)
  %376 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %307, double* elementtype(double) %303, i64 %374)
  %377 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) nonnull %34, i64 %374)
  br label %355

378:                                              ; preds = %369
  %379 = fmul fast double %366, 1.500000e+00
  %380 = fdiv fast double %379, %294
  br label %778

381:                                              ; preds = %290
  %382 = load double*, double** %63, align 1
  %383 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %65, i32 0)
  %384 = load i64, i64* %383, align 1
  %385 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %65, i32 1)
  %386 = load i64, i64* %385, align 1
  %387 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %386, double* elementtype(double) %382, i64 1)
  %388 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %384, double* elementtype(double) %387, i64 1)
  %389 = load double, double* %388, align 1
  %390 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %386, double* elementtype(double) %382, i64 2)
  %391 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %384, double* elementtype(double) %390, i64 2)
  %392 = load double, double* %391, align 1
  %393 = fadd fast double %392, %389
  %394 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %386, double* elementtype(double) %382, i64 3)
  %395 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %384, double* elementtype(double) %394, i64 3)
  %396 = load double, double* %395, align 1
  %397 = fadd fast double %393, %396
  %398 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %64, i32 0)
  %399 = ptrtoint double* %382 to i64
  br label %320

400:                                              ; preds = %412, %400
  %401 = phi i64 [ 1, %412 ], [ %407, %400 ]
  %402 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %112, double* elementtype(double) %414, i64 %401)
  %403 = bitcast double* %402 to i64*
  %404 = load i64, i64* %403, align 1
  %405 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %415, i64 %401)
  %406 = bitcast double* %405 to i64*
  store i64 %404, i64* %406, align 1
  %407 = add nuw nsw i64 %401, 1
  %408 = icmp eq i64 %407, 4
  br i1 %408, label %409, label %400

409:                                              ; preds = %400
  %410 = add nuw nsw i64 %413, 1
  %411 = icmp eq i64 %410, 4
  br i1 %411, label %428, label %412

412:                                              ; preds = %778, %409
  %413 = phi i64 [ %410, %409 ], [ 1, %778 ]
  %414 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %114, double* elementtype(double) %110, i64 %413)
  %415 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$OLD_PLASTIC_STRAIN_TENSOR", i64 0, i64 0, i64 0), i64 %413)
  br label %400

416:                                              ; preds = %428, %416
  %417 = phi double [ %429, %428 ], [ %422, %416 ]
  %418 = phi i64 [ 1, %428 ], [ %423, %416 ]
  %419 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %431, i64 %418)
  %420 = load double, double* %419, align 1
  %421 = fmul fast double %420, %420
  %422 = fadd fast double %421, %417
  %423 = add nuw nsw i64 %418, 1
  %424 = icmp eq i64 %423, 4
  br i1 %424, label %425, label %416

425:                                              ; preds = %416
  %426 = add nuw nsw i64 %430, 1
  %427 = icmp eq i64 %426, 4
  br i1 %427, label %446, label %428

428:                                              ; preds = %425, %409
  %429 = phi double [ %422, %425 ], [ 0.000000e+00, %409 ]
  %430 = phi i64 [ %426, %425 ], [ 1, %409 ]
  %431 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %430)
  br label %416

432:                                              ; preds = %446, %432
  %433 = phi double [ %447, %446 ], [ %440, %432 ]
  %434 = phi i64 [ 1, %446 ], [ %441, %432 ]
  %435 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %449, i64 %434)
  %436 = load double, double* %435, align 1
  %437 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %305, double* elementtype(double) %450, i64 %434)
  %438 = load double, double* %437, align 1
  %439 = fmul fast double %438, %436
  %440 = fadd fast double %439, %433
  %441 = add nuw nsw i64 %434, 1
  %442 = icmp eq i64 %441, 4
  br i1 %442, label %443, label %432

443:                                              ; preds = %432
  %444 = add nuw nsw i64 %448, 1
  %445 = icmp eq i64 %444, 4
  br i1 %445, label %451, label %446

446:                                              ; preds = %443, %425
  %447 = phi double [ %440, %443 ], [ 0.000000e+00, %425 ]
  %448 = phi i64 [ %444, %443 ], [ 1, %425 ]
  %449 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %448)
  %450 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %307, double* elementtype(double) %303, i64 %448)
  br label %432

451:                                              ; preds = %443
  %452 = fmul fast double %440, 2.000000e+00
  %453 = load i64, i64* %291, align 1
  %454 = load i64, i64* %292, align 1
  %455 = icmp slt i64 %454, 1
  %456 = icmp slt i64 %453, 1
  %457 = or i1 %455, %456
  br i1 %457, label %477, label %458

458:                                              ; preds = %451
  %459 = add nuw nsw i64 %453, 1
  %460 = add nuw nsw i64 %454, 1
  br label %473

461:                                              ; preds = %473, %461
  %462 = phi double [ %474, %473 ], [ %467, %461 ]
  %463 = phi i64 [ 1, %473 ], [ %468, %461 ]
  %464 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %305, double* elementtype(double) %476, i64 %463)
  %465 = load double, double* %464, align 1
  %466 = fmul fast double %465, %465
  %467 = fadd fast double %466, %462
  %468 = add nuw nsw i64 %463, 1
  %469 = icmp eq i64 %468, %459
  br i1 %469, label %470, label %461

470:                                              ; preds = %461
  %471 = add nuw nsw i64 %475, 1
  %472 = icmp eq i64 %471, %460
  br i1 %472, label %477, label %473

473:                                              ; preds = %470, %458
  %474 = phi double [ %467, %470 ], [ 0.000000e+00, %458 ]
  %475 = phi i64 [ %471, %470 ], [ 1, %458 ]
  %476 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %307, double* elementtype(double) nonnull %303, i64 %475)
  br label %461

477:                                              ; preds = %470, %451
  %478 = phi double [ 0.000000e+00, %451 ], [ %467, %470 ]
  %479 = fmul fast double %297, %297
  %480 = fmul fast double %479, 0x3FE5555555555555
  %481 = fsub fast double %478, %480
  %482 = fmul fast double %452, %452
  %483 = fmul fast double %422, 4.000000e+00
  %484 = fmul fast double %483, %481
  %485 = fsub fast double %482, %484
  %486 = fcmp fast olt double %485, 0.000000e+00
  br i1 %486, label %487, label %498

487:                                              ; preds = %477
  %488 = getelementptr inbounds [4 x i8], [4 x i8]* %28, i64 0, i64 0
  store i8 56, i8* %488, align 1
  %489 = getelementptr inbounds [4 x i8], [4 x i8]* %28, i64 0, i64 1
  store i8 4, i8* %489, align 1
  %490 = getelementptr inbounds [4 x i8], [4 x i8]* %28, i64 0, i64 2
  store i8 1, i8* %490, align 1
  %491 = getelementptr inbounds [4 x i8], [4 x i8]* %28, i64 0, i64 3
  store i8 0, i8* %491, align 1
  %492 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %29, i64 0, i32 0
  store i64 43, i64* %492, align 8
  %493 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %29, i64 0, i32 1
  store i8* getelementptr inbounds ([43 x i8], [43 x i8]* @anon.58475115b99d6a1547144f0b5d7ba7df.78, i64 0, i64 0), i8** %493, align 8
  %494 = bitcast [8 x i64]* %21 to i8*
  %495 = bitcast { i64, i8* }* %29 to i8*
  %496 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %494, i32 -1, i64 1239157112576, i8* nonnull %488, i8* nonnull %495) #6
  %497 = call i32 (i8*, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(i8* getelementptr inbounds ([0 x i8], [0 x i8]* @anon.58475115b99d6a1547144f0b5d7ba7df.120, i64 0, i64 0), i32 0, i32 0, i64 1239157112576, i32 0, i32 0) #6
  br label %498

498:                                              ; preds = %487, %477
  %499 = call fast double @llvm.sqrt.f64(double %485)
  %500 = fadd fast double %499, %452
  %501 = fmul fast double %422, 2.000000e+00
  %502 = fdiv fast double %500, %501
  %503 = fcmp fast olt double %502, 0.000000e+00
  %504 = fcmp fast ogt double %502, 1.000000e+00
  %505 = or i1 %503, %504
  br i1 %505, label %506, label %537

506:                                              ; preds = %498
  %507 = getelementptr inbounds [4 x i8], [4 x i8]* %30, i64 0, i64 0
  store i8 56, i8* %507, align 1
  %508 = getelementptr inbounds [4 x i8], [4 x i8]* %30, i64 0, i64 1
  store i8 4, i8* %508, align 1
  %509 = getelementptr inbounds [4 x i8], [4 x i8]* %30, i64 0, i64 2
  store i8 2, i8* %509, align 1
  %510 = getelementptr inbounds [4 x i8], [4 x i8]* %30, i64 0, i64 3
  store i8 0, i8* %510, align 1
  %511 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %31, i64 0, i32 0
  store i64 46, i64* %511, align 8
  %512 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %31, i64 0, i32 1
  store i8* getelementptr inbounds ([46 x i8], [46 x i8]* @anon.58475115b99d6a1547144f0b5d7ba7df.77, i64 0, i64 0), i8** %512, align 8
  %513 = bitcast [8 x i64]* %21 to i8*
  %514 = bitcast { i64, i8* }* %31 to i8*
  %515 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %513, i32 -1, i64 1239157112576, i8* nonnull %507, i8* nonnull %514) #6
  %516 = getelementptr inbounds [4 x i8], [4 x i8]* %32, i64 0, i64 0
  store i8 48, i8* %516, align 1
  %517 = getelementptr inbounds [4 x i8], [4 x i8]* %32, i64 0, i64 1
  store i8 1, i8* %517, align 1
  %518 = getelementptr inbounds [4 x i8], [4 x i8]* %32, i64 0, i64 2
  store i8 1, i8* %518, align 1
  %519 = getelementptr inbounds [4 x i8], [4 x i8]* %32, i64 0, i64 3
  store i8 0, i8* %519, align 1
  %520 = getelementptr inbounds { double }, { double }* %33, i64 0, i32 0
  store double %502, double* %520, align 8
  %521 = bitcast { double }* %33 to i8*
  %522 = call i32 @for_write_seq_lis_xmit(i8* nonnull %513, i8* nonnull %516, i8* nonnull %521) #6
  %523 = call i32 (i8*, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(i8* getelementptr inbounds ([0 x i8], [0 x i8]* @anon.58475115b99d6a1547144f0b5d7ba7df.120, i64 0, i64 0), i32 0, i32 0, i64 1239157112576, i32 0, i32 0) #6
  br label %537

524:                                              ; preds = %534, %524
  %525 = phi i64 [ 1, %534 ], [ %529, %524 ]
  %526 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %536, i64 %525)
  %527 = load double, double* %526, align 1
  %528 = fmul fast double %527, %502
  store double %528, double* %526, align 1
  %529 = add nuw nsw i64 %525, 1
  %530 = icmp eq i64 %529, 4
  br i1 %530, label %531, label %524

531:                                              ; preds = %524
  %532 = add nuw nsw i64 %535, 1
  %533 = icmp eq i64 %532, 4
  br i1 %533, label %550, label %534

534:                                              ; preds = %537, %531
  %535 = phi i64 [ 1, %537 ], [ %532, %531 ]
  %536 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %535)
  br label %524

537:                                              ; preds = %506, %498
  br label %534

538:                                              ; preds = %550, %538
  %539 = phi i64 [ 1, %550 ], [ %545, %538 ]
  %540 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %552, i64 %539)
  %541 = bitcast double* %540 to i64*
  %542 = load i64, i64* %541, align 1
  %543 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %91, double* elementtype(double) %553, i64 %539)
  %544 = bitcast double* %543 to i64*
  store i64 %542, i64* %544, align 1
  %545 = add nuw nsw i64 %539, 1
  %546 = icmp eq i64 %545, 4
  br i1 %546, label %547, label %538

547:                                              ; preds = %538
  %548 = add nuw nsw i64 %551, 1
  %549 = icmp eq i64 %548, 4
  br i1 %549, label %554, label %550

550:                                              ; preds = %547, %531
  %551 = phi i64 [ %548, %547 ], [ 1, %531 ]
  %552 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %551)
  %553 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %94, double* elementtype(double) %89, i64 %551)
  br label %538

554:                                              ; preds = %547
  %555 = load double, double* %174, align 1
  %556 = fadd fast double %555, %241
  store double %556, double* %174, align 1
  %557 = load double, double* %177, align 1
  %558 = fadd fast double %557, %241
  store double %558, double* %177, align 1
  %559 = load double, double* %181, align 1
  %560 = fadd fast double %559, %241
  store double %560, double* %181, align 1
  %561 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %105, double* elementtype(double) %97, i64 1)
  %562 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %99, double* elementtype(double) %561, i64 1)
  %563 = load double, double* %562, align 1
  %564 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %105, double* elementtype(double) %97, i64 2)
  %565 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %99, double* elementtype(double) %564, i64 2)
  %566 = load double, double* %565, align 1
  %567 = fadd fast double %566, %563
  %568 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %105, double* elementtype(double) %97, i64 3)
  %569 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %99, double* elementtype(double) %568, i64 3)
  %570 = load double, double* %569, align 1
  %571 = fadd fast double %567, %570
  br label %584

572:                                              ; preds = %584, %572
  %573 = phi i64 [ 1, %584 ], [ %579, %572 ]
  %574 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %99, double* elementtype(double) %586, i64 %573)
  %575 = bitcast double* %574 to i64*
  %576 = load i64, i64* %575, align 1
  %577 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %587, i64 %573)
  %578 = bitcast double* %577 to i64*
  store i64 %576, i64* %578, align 1
  %579 = add nuw nsw i64 %573, 1
  %580 = icmp eq i64 %579, 4
  br i1 %580, label %581, label %572

581:                                              ; preds = %572
  %582 = add nuw nsw i64 %585, 1
  %583 = icmp eq i64 %582, 4
  br i1 %583, label %588, label %584

584:                                              ; preds = %581, %554
  %585 = phi i64 [ 1, %554 ], [ %582, %581 ]
  %586 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %105, double* elementtype(double) nonnull %97, i64 %585)
  %587 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 0, i64 0, i64 0), i64 %585)
  br label %572

588:                                              ; preds = %581
  %589 = fmul fast double %571, 0x3FD5555555555555
  %590 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 0, i64 0, i64 0), i64 1)
  %591 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %590, i64 1)
  %592 = load double, double* %591, align 1
  %593 = fsub fast double %592, %589
  store double %593, double* %591, align 1
  %594 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 0, i64 0, i64 0), i64 2)
  %595 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %594, i64 2)
  %596 = load double, double* %595, align 1
  %597 = fsub fast double %596, %589
  store double %597, double* %595, align 1
  %598 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 0, i64 0, i64 0), i64 3)
  %599 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %598, i64 3)
  %600 = load double, double* %599, align 1
  %601 = fsub fast double %600, %589
  store double %601, double* %599, align 1
  br label %616

602:                                              ; preds = %616, %602
  %603 = phi i64 [ 1, %616 ], [ %611, %602 ]
  %604 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %112, double* elementtype(double) %618, i64 %603)
  %605 = load double, double* %604, align 1
  %606 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %619, i64 %603)
  %607 = load double, double* %606, align 1
  %608 = fsub fast double %605, %607
  %609 = fmul fast double %608, %502
  %610 = fadd fast double %609, %607
  store double %610, double* %604, align 1
  %611 = add nuw nsw i64 %603, 1
  %612 = icmp eq i64 %611, 4
  br i1 %612, label %613, label %602

613:                                              ; preds = %602
  %614 = add nuw nsw i64 %617, 1
  %615 = icmp eq i64 %614, 4
  br i1 %615, label %620, label %616

616:                                              ; preds = %613, %588
  %617 = phi i64 [ 1, %588 ], [ %614, %613 ]
  %618 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %114, double* elementtype(double) %110, i64 %617)
  %619 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DEVIATORIC_STRAIN_TENSOR", i64 0, i64 0, i64 0), i64 %617)
  br label %602

620:                                              ; preds = %613
  %621 = fsub fast double 1.000000e+00, %502
  %622 = load double, double* %0, align 1
  %623 = fmul fast double %622, %621
  %624 = fcmp fast ugt double %623, 0.000000e+00
  br i1 %624, label %650, label %633

625:                                              ; preds = %633, %625
  %626 = phi i64 [ 1, %633 ], [ %628, %625 ]
  %627 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %635, i64 %626)
  store double 0.000000e+00, double* %627, align 1
  %628 = add nuw nsw i64 %626, 1
  %629 = icmp eq i64 %628, 4
  br i1 %629, label %630, label %625

630:                                              ; preds = %625
  %631 = add nuw nsw i64 %634, 1
  %632 = icmp eq i64 %631, 4
  br i1 %632, label %682, label %633

633:                                              ; preds = %630, %620
  %634 = phi i64 [ %631, %630 ], [ 1, %620 ]
  %635 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 %634)
  br label %625

636:                                              ; preds = %650, %636
  %637 = phi i64 [ 1, %650 ], [ %645, %636 ]
  %638 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %112, double* elementtype(double) %652, i64 %637)
  %639 = load double, double* %638, align 1
  %640 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %653, i64 %637)
  %641 = load double, double* %640, align 1
  %642 = fsub fast double %639, %641
  %643 = fdiv fast double %642, %623
  %644 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %654, i64 %637)
  store double %643, double* %644, align 1
  %645 = add nuw nsw i64 %637, 1
  %646 = icmp eq i64 %645, 4
  br i1 %646, label %647, label %636

647:                                              ; preds = %636
  %648 = add nuw nsw i64 %651, 1
  %649 = icmp eq i64 %648, 4
  br i1 %649, label %682, label %650

650:                                              ; preds = %647, %620
  %651 = phi i64 [ %648, %647 ], [ 1, %620 ]
  %652 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %114, double* elementtype(double) nonnull %110, i64 %651)
  %653 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$OLD_PLASTIC_STRAIN_TENSOR", i64 0, i64 0, i64 0), i64 %651)
  %654 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 %651)
  br label %636

655:                                              ; preds = %667, %655
  %656 = phi double [ %668, %667 ], [ %661, %655 ]
  %657 = phi i64 [ 1, %667 ], [ %662, %655 ]
  %658 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %670, i64 %657)
  %659 = load double, double* %658, align 1
  %660 = fmul fast double %659, %659
  %661 = fadd fast double %660, %656
  %662 = add nuw nsw i64 %657, 1
  %663 = icmp eq i64 %662, 4
  br i1 %663, label %664, label %655

664:                                              ; preds = %655
  %665 = add nuw nsw i64 %669, 1
  %666 = icmp eq i64 %665, 4
  br i1 %666, label %671, label %667

667:                                              ; preds = %682, %664
  %668 = phi double [ 0.000000e+00, %682 ], [ %661, %664 ]
  %669 = phi i64 [ 1, %682 ], [ %665, %664 ]
  %670 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 %669)
  br label %655

671:                                              ; preds = %664
  %672 = fmul fast double %661, 0x3FE5555555555555
  %673 = call fast double @llvm.sqrt.f64(double %672)
  %674 = fmul fast double %673, %254
  %675 = load double, double* %14, align 1
  %676 = fmul fast double %673, %623
  %677 = fadd fast double %675, %676
  store double %677, double* %14, align 1
  %678 = load double, double* %7, align 1
  %679 = load double, double* %6, align 1
  %680 = fmul fast double %254, 0x3FE5555555555555
  %681 = fmul fast double %680, %679
  br label %699

682:                                              ; preds = %647, %630
  br label %667

683:                                              ; preds = %699, %683
  %684 = phi i64 [ 1, %699 ], [ %694, %683 ]
  %685 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %701, i64 %684)
  %686 = load double, double* %685, align 1
  %687 = fmul fast double %681, %686
  %688 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %305, double* elementtype(double) %702, i64 %684)
  %689 = load double, double* %688, align 1
  %690 = fmul fast double %689, %674
  %691 = fsub fast double %687, %690
  %692 = fmul fast double %691, %678
  %693 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %703, i64 %684)
  store double %692, double* %693, align 1
  %694 = add nuw nsw i64 %684, 1
  %695 = icmp eq i64 %694, 4
  br i1 %695, label %696, label %683

696:                                              ; preds = %683
  %697 = add nuw nsw i64 %700, 1
  %698 = icmp eq i64 %697, 4
  br i1 %698, label %704, label %699

699:                                              ; preds = %696, %671
  %700 = phi i64 [ 1, %671 ], [ %697, %696 ]
  %701 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$PLASTIC_STRAIN_RATE_TENSOR", i64 0, i64 0, i64 0), i64 %700)
  %702 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %307, double* elementtype(double) %303, i64 %700)
  %703 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$BACK_STRESS_RATE_TENSOR", i64 0, i64 0, i64 0), i64 %700)
  br label %683

704:                                              ; preds = %696
  %705 = load double, double* %5, align 1
  %706 = load double, double* %4, align 1
  br label %720

707:                                              ; preds = %720, %707
  %708 = phi i64 [ 1, %720 ], [ %715, %707 ]
  %709 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 %305, double* elementtype(double) %722, i64 %708)
  %710 = load double, double* %709, align 1
  %711 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %723, i64 %708)
  %712 = load double, double* %711, align 1
  %713 = fmul fast double %712, %623
  %714 = fadd fast double %713, %710
  store double %714, double* %709, align 1
  %715 = add nuw nsw i64 %708, 1
  %716 = icmp eq i64 %715, 4
  br i1 %716, label %717, label %707

717:                                              ; preds = %707
  %718 = add nuw nsw i64 %721, 1
  %719 = icmp eq i64 %718, 4
  br i1 %719, label %724, label %720

720:                                              ; preds = %717, %704
  %721 = phi i64 [ 1, %704 ], [ %718, %717 ]
  %722 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %307, double* elementtype(double) nonnull %303, i64 %721)
  %723 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$BACK_STRESS_RATE_TENSOR", i64 0, i64 0, i64 0), i64 %721)
  br label %707

724:                                              ; preds = %717
  %725 = fsub fast double %706, %295
  %726 = fmul fast double %674, %623
  %727 = fmul fast double %726, %705
  %728 = fmul fast double %727, %725
  %729 = fadd fast double %728, %295
  store double %729, double* %16, align 1
  %730 = fcmp fast ogt double %241, -1.000000e-10
  br i1 %730, label %743, label %783

731:                                              ; preds = %743, %731
  %732 = phi double [ %744, %743 ], [ %737, %731 ]
  %733 = phi i64 [ 1, %743 ], [ %738, %731 ]
  %734 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %746, i64 %733)
  %735 = load double, double* %734, align 1
  %736 = fmul fast double %735, %735
  %737 = fadd fast double %736, %732
  %738 = add nuw nsw i64 %733, 1
  %739 = icmp eq i64 %738, 4
  br i1 %739, label %740, label %731

740:                                              ; preds = %731
  %741 = add nuw nsw i64 %745, 1
  %742 = icmp eq i64 %741, 4
  br i1 %742, label %747, label %743

743:                                              ; preds = %740, %724
  %744 = phi double [ %737, %740 ], [ 0.000000e+00, %724 ]
  %745 = phi i64 [ %741, %740 ], [ 1, %724 ]
  %746 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 24, double* elementtype(double) getelementptr inbounds ([3 x [3 x double]], [3 x [3 x double]]* @"perdida_m_mp_perdida_$DAMAGED_DEV_STRESS_TENSOR", i64 0, i64 0, i64 0), i64 %745)
  br label %731

747:                                              ; preds = %740
  %748 = load double, double* %9, align 1
  %749 = fcmp fast olt double %677, %748
  br i1 %749, label %769, label %750

750:                                              ; preds = %747
  %751 = fmul fast double %85, 0x3FE5555555555555
  %752 = fadd fast double %751, 0x3FE5555555555555
  %753 = fmul fast double %87, 3.000000e+00
  %754 = fmul fast double %241, %241
  %755 = fmul fast double %754, %753
  %756 = fmul fast double %737, 1.500000e+00
  %757 = fdiv fast double %755, %756
  %758 = fadd fast double %752, %757
  %759 = fmul fast double %81, 3.000000e+00
  %760 = fmul fast double %83, 2.000000e+00
  %761 = fadd fast double %760, %759
  %762 = fmul fast double %87, %761
  %763 = load double, double* %8, align 1
  %764 = fmul fast double %673, 7.500000e-01
  %765 = fmul fast double %764, %737
  %766 = fmul fast double %765, %758
  %767 = fmul fast double %762, %763
  %768 = fdiv fast double %766, %767
  br label %769

769:                                              ; preds = %750, %747
  %770 = phi double [ %768, %750 ], [ 0.000000e+00, %747 ]
  %771 = fmul fast double %770, %623
  %772 = fadd fast double %96, %771
  %773 = fcmp fast oge double %772, 1.000000e+00
  %774 = select fast i1 %773, double 1.000000e+00, double %772
  %775 = load double, double* %18, align 1
  %776 = fcmp fast oge double %774, %775
  %777 = select i1 %776, double 1.000000e+00, double %774
  store double %777, double* %17, align 1
  ret void

778:                                              ; preds = %378, %290
  %779 = phi double [ %380, %378 ], [ -1.000000e+00, %290 ]
  %780 = fcmp fast ole double %298, %779
  %781 = select i1 %780, double %298, double %779
  %782 = fcmp fast ult double %781, 0.000000e+00
  br i1 %782, label %783, label %412

783:                                              ; preds = %778, %724
  ret void
}

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* %3, i32 %4)

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 %0, i64 %1, i64 %2, double* %3, i64 %4)

declare double @llvm.sqrt.f64(double %0)

define dso_local void @MAIN__() #0 {
bb1:
  %t11 = alloca double, align 8
  %t12 = alloca double, align 8
  %t17 = alloca double, align 8
  %t18 = alloca double, align 8
  %t19 = alloca double, align 8
  %t20 = alloca double, align 8
  %t21 = alloca double, align 8
  %t22 = alloca double, align 8
  %t23 = alloca double, align 8
  %t24 = alloca double, align 8
  %t25 = alloca double, align 8
  %t26 = alloca double, align 8
  %t52 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t53 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t54 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t55 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t56 = alloca %"QNCA_a0$double*$rank2$", align 8
  %t661 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @"iztaccihuatl_$DAMAGE", i64 0, i32 6, i64 0, i32 2), i32 0)
  %t684 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @"iztaccihuatl_$ACCUMULATED_PLASTIC_STRAIN", i64 0, i32 6, i64 0, i32 2), i32 0)
  %t707 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @"iztaccihuatl_$ISOTROPIC_HARDENING_STRESS", i64 0, i32 6, i64 0, i32 2), i32 0)
  %t2092 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @"iztaccihuatl_$ACCUMULATED_PLASTIC_STRAIN", i64 0, i32 0), align 16
  %t2093 = load i64, i64* %t684, align 1
  %t2094 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %t2093, i64 8, double* elementtype(double) %t2092, i64 1)
  %t2105 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @"iztaccihuatl_$ISOTROPIC_HARDENING_STRESS", i64 0, i32 0), align 16
  %t2106 = load i64, i64* %t707, align 1
  %t2107 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %t2106, i64 8, double* elementtype(double) %t2105, i64 1)
  %t2108 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @"iztaccihuatl_$DAMAGE", i64 0, i32 0), align 16
  %t2109 = load i64, i64* %t661, align 1
  %t2110 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %t2109, i64 8, double* elementtype(double) %t2108, i64 1)
  call fastcc void @perdida_m_mp_perdida_(double* nonnull %t26, double* nonnull %t24, double* nonnull %t23, double* nonnull %t25, double* nonnull %t22, double* nonnull %t21, double* nonnull %t20, double* nonnull %t19, double* nonnull %t18, double* nonnull %t17, %"QNCA_a0$double*$rank2$"* nonnull %t52, %"QNCA_a0$double*$rank2$"* nonnull %t53, %"QNCA_a0$double*$rank2$"* nonnull %t54, %"QNCA_a0$double*$rank2$"* nonnull %t55, double* %t2094, %"QNCA_a0$double*$rank2$"* nonnull %t56, double* %t2107, double* %t2110, double* nonnull %t12, double* nonnull %t11)
  ret void
}

attributes #0 = { "intel-lang"="fortran" }
; end INTEL_FEATURE_SW_ADVANCED
