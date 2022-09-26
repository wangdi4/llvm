; RUN: opt -auto-cpu-clone -inline -globaldce < %s -S | FileCheck %s
; RUN: opt -passes=auto-cpu-clone,inline,globaldce < %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:      @__intel_cpu_feature_indicator_x = external global [2 x i64]

; CHECK: @bar = ifunc i32 (i32), i32 (i32)* ()* @bar.resolver
; CHECK-EMPTY:
; CHECK-NEXT: define internal i32 @baz.A(i32 %a) #0 !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar.A(i32 %a) #0 !llvm.acd.clone !0 {
; CHECK-NEXT:   %ret.i = call i32 @baz.A(i32 33)
; CHECK-NEXT:   %add.i = add i32 42, %ret.i
; CHECK-NEXT:   ret i32 %add.i
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define internal i32 @baz.V(i32 %a) #1 !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: declare dso_local void @__intel_cpu_features_init_x()
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar.V(i32 %a) #1 !llvm.acd.clone !0 {
; CHECK-NEXT:   %ret.i = call i32 @baz.V(i32 33)
; CHECK-NEXT:   %add.i = add i32 42, %ret.i
; CHECK-NEXT:   ret i32 %add.i
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 (i32)* @bar.resolver() #0 {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   call void @__intel_cpu_features_init_x()
; CHECK-NEXT:   %cpu_feature_indicator = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator, 10330092
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 10330092
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %resolver_entry
; CHECK-NEXT:   ret i32 (i32)* @bar.V
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %resolver_entry
; CHECK-NEXT:   ret i32 (i32)* @bar.A
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: attributes #0 = { "advanced-optim"="false" }
; CHECK-NEXT: attributes #1 = { "advanced-optim"="true" "loopopt-pipeline"="full" "target-cpu"="haswell" "target-features"="+cmov,+mmx,+sse,+sse2,+sse3,+ssse3,+sse4.1,+sse4.2,+movbe,+popcnt,+f16c,+avx,+fma,+bmi,+lzcnt,+avx2" "tune-cpu"="haswell" }


define internal i32 @baz(i32 %a) !llvm.auto.cpu.dispatch !1 {
  %add = add i32 %a, 42
  ret i32 %add
}

define internal i32 @foo(i32 %a) {
  %ret = call i32 @baz(i32 33)
  %add = add i32 %a, %ret
  ret i32 %add
}

define i32 @bar(i32 %a) !llvm.auto.cpu.dispatch !1 {
  %ret = call i32 @foo(i32 42)
  ret i32 %ret
}

attributes #0 = { "target-features"="+sse4.2" }

!0 = !{!"auto-cpu-dispatch-target", !"haswell"}

!1 = !{!0}
