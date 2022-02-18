; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -ip-cloning -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning -mtriple=i686-- -mattr=+avx2  -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='module(ip-cloning)' -ip-gen-cloning-force-enable-dtrans -debug-only=ipcloning -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@count = available_externally dso_local local_unnamed_addr global i32 0, align 8

; Check that foo is not a recursive progression clone candidate
; because it is not compiled for Intel AVX2.
; CHECK-LABEL: Cloning Analysis for:  foo
; CHECK-NOT: Selected RecProgression cloning

define dso_local void @MAIN__() #0 {
  %1 = alloca i32, align 4
  store i32 1, i32* %1, align 4
  call void @foo(i32* nonnull %1)
  ret void
}

; Function Attrs: nounwind
define internal void @foo(i32* noalias nocapture readonly) {
  %2 = alloca i32, align 4
  %3 = load i32, i32* @count, align 8
  %4 = add nsw i32 %3, 1
  store i32 %4, i32* @count, align 8
  %5 = load i32, i32* %0, align 4
  %6 = icmp eq i32 %5, 8
  br i1 %6, label %7, label %9

; <label>:7:                                      ; preds = %1
  %8 = add nsw i32 %3, 2
  store i32 %8, i32* @count, align 8
  br label %13

; <label>:9:                                      ; preds = %1
  %10 = icmp slt i32 %4, 500000
  br i1 %10, label %11, label %13

; <label>:11:                                     ; preds = %9
  %12 = add nsw i32 %5, 1
  store i32 %12, i32* %2, align 4
  call void @foo(i32* nonnull %2)
  br label %13

; <label>:13:                                     ; preds = %11, %9, %7
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
