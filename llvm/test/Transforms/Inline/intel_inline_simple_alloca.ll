; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; This test verifies that foo is inlined even though it has dynamic alloca
; instructions.

; RUN: opt < %s -S -inline -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain 2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt < %s -S -inlinereportsetup -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -inlinereportemitter 2>&1 | FileCheck %s

; New PM command
; RUN: opt < %s -S -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain  2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt < %s -S -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain 2>&1 | FileCheck %s

; CHECK: COMPILE FUNC: foo
; CHECK: COMPILE FUNC: bar
; CHECK: {{.*}}INLINE: foo{{.*}}
; CHECK-NOT: foo {{.*}}Callee has dynamic alloca

; CHECK-NOT: call void @foo


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %i, double* %dp) {
entry:
  %a = add i32 %i, 1
  br label %bb
bb:
  %p1 = alloca i32
  store i32 20, i32* %p1
  %p2 = alloca double*
  %b1 = bitcast double** %p2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %b1)
  store double* %dp, double** %p2
  call void @baz(i32* %p1, double** %p2)
  %b2 = bitcast double** %p2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %b2)
  %p3 = alloca i32
  store i32 2, i32* %p3
  %l3 = load i32, i32* %p3
  br label %return

return:
  ret void
}

define void @bar(i32 %i, double* %ptr) {
entry:
  call void @foo(i32 %i, double* %ptr)
  br label %return

return:
  ret void
}

declare void @baz(i32*, double**)
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
; end INTEL_FEATURE_SW_ADVANCED
