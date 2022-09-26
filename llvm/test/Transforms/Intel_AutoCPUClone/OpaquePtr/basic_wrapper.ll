; RUN: opt -opaque-pointers -auto-cpu-clone < %s -S | FileCheck %s
; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s
; XFAIL: *


; CHECK:      @__intel_cpu_feature_indicator_x = external global [2 x i64]
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @baz.A(i32 %a) !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @foo.A(i32 %a) !llvm.acd.clone !0 {
; CHECK-NEXT:   %ret = call i32 @baz.A(i32 33)
; CHECK-NEXT:   %add = add i32 %a, %ret
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar(i32 %a) {
; CHECK-NEXT:   %ret = call i32 @foo(i32 42)
; CHECK-NEXT:   ret i32 %ret
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @baz.V(i32 %a) #0 !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @baz(i32 %0) {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   call void @__intel_cpu_features_init_x()
; CHECK-NEXT:   %cpu_feature_indicator = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator, 10330092
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 10330092
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %resolver_entry
; CHECK-NEXT:   %1 = tail call i32 @baz.V(i32 %0)
; CHECK-NEXT:   ret i32 %1
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %resolver_entry
; CHECK-NEXT:   %2 = tail call i32 @baz.A(i32 %0)
; CHECK-NEXT:   ret i32 %2
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: declare dso_local void @__intel_cpu_features_init_x()
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @foo.V(i32 %a) #0 !llvm.acd.clone !0 {
; CHECK-NEXT:   %ret = call i32 @baz.V(i32 33)
; CHECK-NEXT:   %add = add i32 %a, %ret
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @foo(i32 %0) {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   call void @__intel_cpu_features_init_x()
; CHECK-NEXT:   %cpu_feature_indicator = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator, 10330092
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 10330092
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %resolver_entry
; CHECK-NEXT:   %1 = tail call i32 @foo.V(i32 %0)
; CHECK-NEXT:   ret i32 %1
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %resolver_entry
; CHECK-NEXT:   %2 = tail call i32 @foo.A(i32 %0)
; CHECK-NEXT:   ret i32 %2
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: attributes #0 = { "loopopt-pipeline"="full" "target-cpu"="haswell" "target-features"="+cmov,+mmx,+sse,+sse2,+sse3,+ssse3,+sse4.1,+sse4.2,+movbe,+popcnt,+f16c,+avx,+fma,+bmi,+lzcnt,+avx2" "tune-cpu"="haswell" }


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

define i32 @baz(i32 %a) !llvm.auto.cpu.dispatch !0 {
  %add = add i32 %a, 42
  ret i32 %add
}

define i32 @foo(i32 %a) !llvm.auto.cpu.dispatch !0 {
  %ret = call i32 @baz(i32 33)
  %add = add i32 %a, %ret
  ret i32 %add
}

define i32 @bar(i32 %a) {
  %ret = call i32 @foo(i32 42)
  ret i32 %ret
}

!0 = !{!1}
!1 = !{!"auto-cpu-dispatch-target", !"haswell"}
