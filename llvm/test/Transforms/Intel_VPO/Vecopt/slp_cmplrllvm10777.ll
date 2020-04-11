; RUN: opt -enable-intel-advanced-opts -mcpu=skylake-avx512 -tti -basicaa --slp-vectorizer -disable-output -debug-only=SLP -slp-look-ahead-users-budget=100 < %s 2>&1 | FileCheck %s -check-prefix=CHECK_HI_LIMIT
; RUN: not --crash opt -enable-intel-advanced-opts -mcpu=skylake-avx512 -tti -basicaa --slp-vectorizer -disable-output -debug-only=SLP < %s > %t.out 2>&1
; RUN: FileCheck %s -check-prefix=CHECK_DEFAULT_LIMIT < %t.out

; REQUIRES: asserts

; SLP here makes 3 attempts to build vectorizable tree. On the second try
; it touches IR and looks for cost. If cost is no better it then reverts
; IR changes and makes another tree build. On the third attempt even though
; IR is same as during initial build SLP behavior turned out to be sensitive
; to ordering of instruction users lists and that affected its behavior.

; CHECK_HI_LIMIT:  SLP: Total Cost = 0.
; CHECK_HI_LIMIT:  SLP: Total Cost = 7.
; CHECK_HI_LIMIT:  SLP: Total Cost = 0.

; FIXME: Fix intended to add determinism to SLP behavior so cost can change a bit with it.
; Important thing is that for first and third attempts cost must be same
; and second time cost is bigger.

; CHECK_DEFAULT_LIMIT:  SLP: Total Cost = 0.
; CHECK_DEFAULT_LIMIT:  SLP: Total Cost = 7.
; CHECK_DEFAULT_LIMIT:  SLP: Total Cost = 6.

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


