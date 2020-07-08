; RUN: opt -enable-intel-advanced-opts -mcpu=skylake-avx512 -tti -basic-aa --slp-vectorizer -disable-output -debug-only=SLP -slp-look-ahead-users-budget=100 < %s 2>&1 | FileCheck %s -check-prefix=CHECK_HI
; RUN: opt -enable-intel-advanced-opts -mcpu=skylake-avx512 -tti -basic-aa --slp-vectorizer -disable-output -debug-only=SLP < %s 2>&1 | FileCheck %s -check-prefix=CHECK_DEFAULT

; REQUIRES: asserts

; SLP here used to make 3 attempts to build vectorizable tree. After initial
; tree build it discovered a possibility for PSLP (padded SLP) and made the
; second try for PSLP. Then since PSLP attempt appeared no better it
; reverted IR changes made for PSLP attempt and ran another tree build.
; Expectation was the cost from 1st try would match 3d one.
; Problem is that SLP behavior is sensitive to ordering of instruction users
; lists and cleanup after PSLP changed their order so that vectorizable tree
; turned out different.
; First attempt to fix that was to sort users list before visiting but the
; remedy turned out way too expensive for compile time.
; Current fix is not to run third tree build if the first attempt was
; not profitable. Additionally the assertion was relaxed to check that the
; third build still remains profitable to vectorize.

; CHECK_HI: SLP: Total Cost = 0.
; CHECK_HI: SLP: Total Cost = 7.
; CHECK_HI-NOT: SLP: Total Cost =

; CHECK_DEFAULT: SLP: Total Cost = 0.
; CHECK_DEFAULT: SLP: Total Cost = 7.
; CHECK_DEFAULT-NOT: SLP: Total Cost =

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define hidden void @foo([16 x [8 x i32]]* %p1) {
  %1 = sub nsw i32 undef, undef
  %2 = add nsw i32 %1, undef
  %3 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %p1, i64 0, i64 1, i64 7
  %4 = sub nsw i32 undef, %2
  %5 = sub nsw i32 %4, undef
  %6 = add i32 %5, undef
  store i32 %6, i32* %3, align 4
  %7 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %p1, i64 0, i64 2, i64 0
  %8 = add nsw i32 undef, undef
  %9 = add nsw i32 undef, undef
  %10 = sub nsw i32 undef, undef
  %11 = add nsw i32 %10, undef
  %12 = add nsw i32 %11, %9
  %13 = add nsw i32 %12, undef
  %14 = add i32 %13, %8
  store i32 %14, i32* %7, align 4
  %15 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %p1, i64 0, i64 2, i64 2
  %16 = add nsw i32 %11, %9
  %17 = sub nsw i32 %16, undef
  %18 = sub i32 %17, %8
  store i32 %18, i32* %15, align 4
  %19 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %p1, i64 0, i64 2, i64 1
  %20 = sub nsw i32 %11, %9
  %21 = add nsw i32 %20, undef
  %22 = sub i32 %21, %8
  store i32 %22, i32* %19, align 4
  unreachable
}


