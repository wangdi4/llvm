; RUN: opt -opaque-pointers -passes=auto-cpu-clone,inline,globaldce -enable-selective-mv=0 < %s -S | FileCheck %s


; CHECK:      @__intel_cpu_feature_indicator = external global [2 x i64]
; CHECK:      @llvm.global_ctors = appending global [3 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 500, ptr @baz.resolver, ptr null }, { i32, ptr, ptr } { i32 0, ptr @__intel_cpu_features_init, ptr null }, { i32, ptr, ptr } { i32 500, ptr @bar.resolver, ptr null }]
; CHECK:      @bar.ptr = internal global ptr null
; CHECK-EMPTY:
; CHECK-NEXT: define internal i32 @baz.A(i32 %a) #0 !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar.A(i32 %a) #0 !llvm.acd.clone !0 {
; CHECK-NEXT:   %1 = call i32 @baz.A(i32 33)
; CHECK-NEXT:   %add.i = add i32 42, %1
; CHECK-NEXT:   ret i32 %add.i
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define internal i32 @baz.V(i32 %a) #1 !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define internal void @baz.resolver() #0 {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   %cpu_feature_indicator = load i64, ptr @__intel_cpu_feature_indicator, align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator, 10330092
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 10330092
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %resolver_entry
; CHECK-NEXT:   ret void
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %resolver_entry
; CHECK-NEXT:   ret void
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: declare intel_features_init_cc void @__intel_cpu_features_init()
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar.V(i32 %a) #1 !llvm.acd.clone !0 {
; CHECK-NEXT:   %1 = call i32 @baz.V(i32 33)
; CHECK-NEXT:   %add.i = add i32 42, %1
; CHECK-NEXT:   ret i32 %add.i
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define internal void @bar.resolver() #0 {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   %cpu_feature_indicator = load i64, ptr @__intel_cpu_feature_indicator, align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator, 10330092
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 10330092
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %resolver_entry
; CHECK-NEXT:   store ptr @bar.V, ptr @bar.ptr, align 8
; CHECK-NEXT:   ret void
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %resolver_entry
; CHECK-NEXT:   store ptr @bar.A, ptr @bar.ptr, align 8
; CHECK-NEXT:   ret void
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar(i32 %a) #0 !llvm.acd.dispatcher !0 {
; CHECK-NEXT:   %1 = load ptr, ptr @bar.ptr, align 8
; CHECK-NEXT:   %2 = call i32 %1(i32 %a)
; CHECK-NEXT:   ret i32 %2
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: attributes #0 = { "advanced-optim"="false" }
; CHECK-NEXT: attributes #1 = { "advanced-optim"="true" "loopopt-pipeline"="full" "target-cpu"="haswell" "target-features"="+cmov,+mmx,+sse,+sse2,+sse3,+ssse3,+sse4.1,+sse4.2,+movbe,+popcnt,+f16c,+avx,+fma,+bmi,+lzcnt,+avx2" "tune-cpu"="haswell" }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

define internal i32 @baz(i32 %a) !llvm.auto.cpu.dispatch !0 {
  %add = add i32 %a, 42
  ret i32 %add
}

define internal i32 @foo(i32 %a) {
  %ret = call i32 @baz(i32 33)
  %add = add i32 %a, %ret
  ret i32 %add
}

define i32 @bar(i32 %a) !llvm.auto.cpu.dispatch !0 {
  %ret = call i32 @foo(i32 42)
  ret i32 %ret
}

!0 = !{!1}
!1 = !{!"auto-cpu-dispatch-target", !"haswell"}
