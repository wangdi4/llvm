; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -passes='require<wholeprogram>,function(instcombine),cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -passes='function(instcombine),inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

; Check that @flux_ and @bi_cgstab_block_ are inlined as single callsite and
; local linkage functions, even though the calls to them contain a module
; level BitCastOperator, which will be deleted by instcombine.

; CHECK-BEFORE: call{{.*}}@shell_
; CHECK-BEFORE-NOT: call{{.*}}@flux_
; CHECK-BEFORE-NOT: call{{.*}}@bi_cgstab_block__
; CHECK-DAG: DEAD STATIC FUNC: flux_
; CHECK-DAG: DEAD STATIC FUNC: bi_cgstab_block_
; CHECK-DAG: COMPILE FUNC: shell_
; CHECK-DAG: INLINE: flux_{{.*}}Callee has single callsite and local linkage
; CHECK-DAG: INLINE: bi_cgstab_block_{{.*}}Callee has single callsite and local linkage
; CHECK-DAG: COMPILE FUNC: MAIN__
; CHECK-DAG: shell_
; CHECK-AFTER: call{{.*}}@shell_
; CHECK-AFTER-NOT: call{{.*}}@flux_
; CHECK-AFTER-NOT: call{{.*}}@bi_cgstab_block__

@anon = internal unnamed_addr constant i32 5

define internal void @bi_cgstab_block_(double* noalias %0, double* noalias nocapture readonly %1, double* noalias nocapture readonly %2, double* noalias nocapture readonly %3, double* noalias nocapture readonly %4, double* noalias nocapture readonly %5, double* noalias nocapture readonly %6, double* noalias nocapture readonly %7, double* noalias nocapture readonly %8, double* noalias nocapture readonly %9, i32* noalias %10, i32* noalias %11, i32* noalias %12, i32* noalias %13) #0 {
  ret void
}

define dso_local void @MAIN__() #0 {
  %1 = alloca [8 x i64], align 32
  %2 = alloca double, align 8
  %3 = alloca double, align 8
  %4 = alloca double, align 8
  %5 = alloca double, align 8
  %6 = alloca double, align 8
  %7 = alloca double, align 8
  %8 = alloca i32, align 8
  %9 = alloca i32, align 8
  %10 = alloca i32, align 8
  %11 = alloca i32, align 8
  %12 = alloca i32, align 8
  %13 = alloca i32, align 8
  %14 = alloca i32, align 8
  %15 = alloca i32, align 8
  %16 = alloca i32, align 8
  call void (...) bitcast (void (double*, double*, i32*, i32*, i32*, i32*, i32*, i32*, double*, double*, double*, double*, i32*, i32*, i32*)* @shell_ to void (...)*)(double* nonnull %3, double* nonnull %2, i32* nonnull %16, i32* nonnull %15, i32* nonnull %14, i32* nonnull %10, i32* nonnull %9, i32* nonnull %8, double* nonnull %7, double* nonnull %6, double* nonnull %5, double* nonnull %4, i32* nonnull %13, i32* nonnull %12, i32* nonnull %11)
  ret void
}

define internal void @flux_(double* noalias nocapture readonly %0, double* noalias nocapture %1, double* noalias nocapture %2, double* noalias nocapture %3, double* noalias nocapture %4, double* noalias nocapture %5, double* noalias nocapture %6, double* noalias nocapture readnone %7, double* noalias nocapture readonly %8, double* noalias nocapture readonly %9, i32* noalias nocapture readonly %10, i32* noalias nocapture readonly %11, i32* noalias nocapture readonly %12, double* noalias nocapture readonly %13, double* noalias nocapture readonly %14, double* noalias nocapture readonly %15) #0 {
  ret void
}

declare void @jacobian_(double* noalias nocapture readonly %0, double* noalias nocapture %1, double* noalias nocapture %2, double* noalias nocapture readonly %3, double* noalias nocapture readonly %4, double* noalias nocapture readonly %5, double* noalias nocapture readonly %6, double* noalias nocapture readnone %7, double* noalias nocapture readonly %8, double* noalias nocapture readonly %9, i32* noalias nocapture readonly %10, i32* noalias nocapture readonly %11, i32* noalias nocapture readnone %12, i32* noalias nocapture readonly %13, double* noalias nocapture readonly %14, i32* noalias nocapture readonly %15)

define internal void @shell_(double* noalias %0, double* noalias %1, i32* noalias %2, i32* noalias %3, i32* noalias nocapture readonly %4, i32* noalias nocapture readonly %5, i32* noalias nocapture readnone %6, i32* noalias %7, double* noalias nocapture readonly %8, double* noalias nocapture readonly %9, double* noalias nocapture readonly %10, double* noalias nocapture readonly %11, i32* noalias nocapture readonly %12, i32* noalias nocapture readonly %13, i32* noalias nocapture readonly %14) #0 {
  %t17 = alloca double, align 8
  %t18 = alloca double, align 8
  %t19 = alloca double, align 8
  %t20 = alloca double, align 8
  %t21 = alloca double, align 8
  %t36 = load i32, i32* %2, align 1
  %t51 = sext i32 %t36 to i64
  %t52 = alloca double, i64 %t51, align 1
  %t53 = alloca double, i64 %t51, align 1
  %t54 = alloca double, i64 %t51, align 1
  %t55 = alloca double, i64 %t51, align 1
  %t56 = alloca double, i64 %t51, align 1
  %t57 = alloca double, i64 %t51, align 1
  %t58 = alloca double, i64 %t51, align 1
  %t74 = alloca double, i64 %t51, align 1
  %t77 = alloca double, i64 %t51, align 1
  %t78 = alloca double, i64 %t51, align 1
  %t79 = alloca double, i64 %t51, align 1
  %t80 = alloca double, i64 %t51, align 1
  %t81 = alloca double, i64 %t51, align 1
  %t82 = alloca double, i64 %t51, align 1
  %t83 = alloca double, i64 %t51, align 1
  %t90 = alloca double, i64 %t51, align 1
  call void (...) bitcast (void (double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, i32*, i32*, i32*, double*, double*, double*)* @flux_ to void (...)*)(double* nonnull %t90, double* nonnull %t81, double* nonnull %t80, double* nonnull %t79, double* nonnull %t78, double* nonnull %t77, double* nonnull %t74, double* %0, double* %1, double* nonnull %t17, i32* nonnull %2, i32* nonnull %3, i32* nonnull %7, double* nonnull %t20, double* nonnull %t19, double* nonnull %t18)
  call void (...) bitcast (void (double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, i32*, i32*, i32*, i32*)* @bi_cgstab_block_ to void (...)*)(double* nonnull %t83, double* nonnull %t82, double* nonnull %t58, double* nonnull %t57, double* nonnull %t56, double* nonnull %t55, double* nonnull %t54, double* nonnull %t53, double* nonnull %t52, double* nonnull %t21, i32* nonnull @anon, i32* nonnull %2, i32* nonnull %3, i32* nonnull %7)
  ret void
}

declare void @fill1_(double* noalias nocapture %0, i32* noalias nocapture readonly %1, i32* noalias nocapture readonly %2, i32* noalias nocapture readonly %3, i32* noalias nocapture readonly %4)

declare void @fill2_(double* noalias nocapture %0, i32* noalias nocapture readonly %1, i32* noalias nocapture readonly %2, i32* noalias nocapture readonly %3)

attributes #0 = { "intel-lang"="fortran" }
; end INTEL_FEATURE_SW_ADVANCED
