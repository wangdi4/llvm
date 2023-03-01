; This test verifies that "%0 = load i32, ptr @a" is not removed by
; InstCombine by using incorrect points-to info.
; @a and @b need to be treated as single variable.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK-LABEL: main
; CHECK: load{{.*}}ptr @a

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = internal global i32 0, align 4

@b = alias i32, ptr @a

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) uwtable
define i32 @main() local_unnamed_addr {
entry:
  store i32 0, ptr @a, align 4
  tail call fastcc void @inc()
  %0 = load i32, ptr @a, align 4
  ret i32 %0
}

define internal fastcc void @inc() unnamed_addr {
entry:
  %0 = load i32, ptr @b, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @b, align 4
  ret void
}
