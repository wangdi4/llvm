; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -S -passes=wholeprogramdevirt -whole-program-assume %s | FileCheck %s

; Check for successful devirtualization when vtable contains an alias.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-grtev4-linux-gnu"

%struct.D = type { ptr }

@_ZTV1D = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr undef, ptr @_ZN1D1mEiAlias] }, !type !3

define i32 @_ZN1D1mEi(ptr %this, i32 %a) {
   ret i32 0;
}

@_ZN1D1mEiAlias = unnamed_addr alias i32 (ptr, i32), ptr @_ZN1D1mEi

; CHECK-LABEL: define i32 @test
define i32 @test(ptr %obj2, i32 %a) {
entry:
  %vtable2 = load ptr, ptr %obj2
  %p2 = call i1 @llvm.type.test(ptr %vtable2, metadata !"_ZTS1D")
  call void @llvm.assume(i1 %p2)

  %fptr33 = load ptr, ptr %vtable2, align 8

; Check that the call was devirtualized.
; CHECK: %call4 = tail call i32 @_ZN1D1mEi
  %call4 = tail call i32 %fptr33(ptr nonnull %obj2, i32 %a)
  ret i32 %call4
}

declare i1 @llvm.type.test(ptr, metadata)
declare void @llvm.assume(i1)

!3 = !{i64 16, !"_ZTS1D"}

; end INTEL_FEATURE_SW_DTRANS
