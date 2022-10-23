; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -passes='require<wholeprogram>,function(instcombine),cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe807 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-BEFORE
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='function(instcombine),inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='require<wholeprogram>,cgscc(inline)' -whole-program-assume-read -lto-inline-cost -inline-report=0xe886 -forced-inline-opt-level=3  -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-AFTER

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

define internal void @bi_cgstab_block_(ptr noalias %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3, ptr noalias nocapture readonly %arg4, ptr noalias nocapture readonly %arg5, ptr noalias nocapture readonly %arg6, ptr noalias nocapture readonly %arg7, ptr noalias nocapture readonly %arg8, ptr noalias nocapture readonly %arg9, ptr noalias %arg10, ptr noalias %arg11, ptr noalias %arg12, ptr noalias %arg13) #0 {
bb:
  ret void
}

define dso_local void @MAIN__() #0 {
bb:
  %i = alloca [8 x i64], align 32
  %i1 = alloca double, align 8
  %i2 = alloca double, align 8
  %i3 = alloca double, align 8
  %i4 = alloca double, align 8
  %i5 = alloca double, align 8
  %i6 = alloca double, align 8
  %i7 = alloca i32, align 8
  %i8 = alloca i32, align 8
  %i9 = alloca i32, align 8
  %i10 = alloca i32, align 8
  %i11 = alloca i32, align 8
  %i12 = alloca i32, align 8
  %i13 = alloca i32, align 8
  %i14 = alloca i32, align 8
  %i15 = alloca i32, align 8
  call void (...) @shell_(ptr nonnull %i2, ptr nonnull %i1, ptr nonnull %i15, ptr nonnull %i14, ptr nonnull %i13, ptr nonnull %i9, ptr nonnull %i8, ptr nonnull %i7, ptr nonnull %i6, ptr nonnull %i5, ptr nonnull %i4, ptr nonnull %i3, ptr nonnull %i12, ptr nonnull %i11, ptr nonnull %i10)
  ret void
}

define internal void @flux_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture %arg2, ptr noalias nocapture %arg3, ptr noalias nocapture %arg4, ptr noalias nocapture %arg5, ptr noalias nocapture %arg6, ptr noalias nocapture readnone %arg7, ptr noalias nocapture readonly %arg8, ptr noalias nocapture readonly %arg9, ptr noalias nocapture readonly %arg10, ptr noalias nocapture readonly %arg11, ptr noalias nocapture readonly %arg12, ptr noalias nocapture readonly %arg13, ptr noalias nocapture readonly %arg14, ptr noalias nocapture readonly %arg15) #0 {
bb:
  ret void
}

declare void @jacobian_(ptr noalias nocapture readonly, ptr noalias nocapture, ptr noalias nocapture, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readnone, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readnone, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly)

define internal void @shell_(ptr noalias %arg, ptr noalias %arg1, ptr noalias %arg2, ptr noalias %arg3, ptr noalias nocapture readonly %arg4, ptr noalias nocapture readonly %arg5, ptr noalias nocapture readnone %arg6, ptr noalias %arg7, ptr noalias nocapture readonly %arg8, ptr noalias nocapture readonly %arg9, ptr noalias nocapture readonly %arg10, ptr noalias nocapture readonly %arg11, ptr noalias nocapture readonly %arg12, ptr noalias nocapture readonly %arg13, ptr noalias nocapture readonly %arg14) #0 {
bb:
  %t17 = alloca double, align 8
  %t18 = alloca double, align 8
  %t19 = alloca double, align 8
  %t20 = alloca double, align 8
  %t21 = alloca double, align 8
  %t36 = load i32, ptr %arg2, align 1
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
  call void (...) @flux_(ptr nonnull %t90, ptr nonnull %t81, ptr nonnull %t80, ptr nonnull %t79, ptr nonnull %t78, ptr nonnull %t77, ptr nonnull %t74, ptr %arg, ptr %arg1, ptr nonnull %t17, ptr nonnull %arg2, ptr nonnull %arg3, ptr nonnull %arg7, ptr nonnull %t20, ptr nonnull %t19, ptr nonnull %t18)
  call void (...) @bi_cgstab_block_(ptr nonnull %t83, ptr nonnull %t82, ptr nonnull %t58, ptr nonnull %t57, ptr nonnull %t56, ptr nonnull %t55, ptr nonnull %t54, ptr nonnull %t53, ptr nonnull %t52, ptr nonnull %t21, ptr nonnull @anon, ptr nonnull %arg2, ptr nonnull %arg3, ptr nonnull %arg7)
  ret void
}

declare void @fill1_(ptr noalias nocapture, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly)

declare void @fill2_(ptr noalias nocapture, ptr noalias nocapture readonly, ptr noalias nocapture readonly, ptr noalias nocapture readonly)

attributes #0 = { "intel-lang"="fortran" }
; end INTEL_FEATURE_SW_ADVANCED
