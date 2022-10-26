; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; This test verifies that foo is inlined even though it has dynamic alloca
; instructions.

; RUN: opt -opaque-pointers < %s -S -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain  2>&1 | FileCheck %s
; Same as above except using metadata
; RUN: opt -opaque-pointers < %s -S -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain 2>&1 | FileCheck %s

; CHECK: COMPILE FUNC: foo
; CHECK: COMPILE FUNC: bar
; CHECK: {{.*}}INLINE: foo{{.*}}
; CHECK-NOT: foo {{.*}}Callee has dynamic alloca

; CHECK-NOT: call void @foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %i, ptr %dp) {
entry:
  %a = add i32 %i, 1
  br label %bb

bb:                                               ; preds = %entry
  %p1 = alloca i32, align 4
  store i32 20, ptr %p1, align 4
  %p2 = alloca ptr, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %p2)
  store ptr %dp, ptr %p2, align 8
  call void @baz(ptr %p1, ptr %p2)
  call void @llvm.lifetime.end.p0(i64 8, ptr %p2)
  %p3 = alloca i32, align 4
  store i32 2, ptr %p3, align 4
  %l3 = load i32, ptr %p3, align 4
  br label %return

return:                                           ; preds = %bb
  ret void
}

define void @bar(i32 %i, ptr %ptr) {
entry:
  call void @foo(i32 %i, ptr %ptr)
  br label %return

return:                                           ; preds = %entry
  ret void
}

declare void @baz(ptr, ptr)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { argmemonly nocallback nofree nosync nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
