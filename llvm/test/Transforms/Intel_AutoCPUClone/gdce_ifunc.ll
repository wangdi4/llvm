; RUN: opt -auto-cpu-clone -inline -globaldce < %s -S | FileCheck %s
; RUN: opt -passes=auto-cpu-clone,inline,globaldce < %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:      @__intel_cpu_feature_indicator = external global [2 x i64]

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
; CHECK-NEXT: declare dso_local intel_features_init_cc void @__intel_cpu_features_init()
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar.V(i32 %a) #1 !llvm.acd.clone !0 {
; CHECK-NEXT:   %ret.i = call i32 @baz.V(i32 33)
; CHECK-NEXT:   %add.i = add i32 42, %ret.i
; CHECK-NEXT:   ret i32 %add.i
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 (i32)* @bar.resolver() #0 {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   br label %cpu_feature_init_cmp
; CHECK-EMPTY:
; CHECK-NEXT: cpu_feature_init_cmp:                             ; preds = %cpu_feature_init_body, %resolver_entry
; CHECK-NEXT:   %cpu_feature_indicator = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator, i64 0, i64 0), align 8
; CHECK-NEXT:   %0 = icmp eq i64 %cpu_feature_indicator, 0
; CHECK-NEXT:   br i1 %0, label %cpu_feature_init_body, label %cpu_feature_init_rest
; CHECK-EMPTY:
; CHECK-NEXT: cpu_feature_init_body:                            ; preds = %cpu_feature_init_cmp
; CHECK-NEXT:   call void @__intel_cpu_features_init()
; CHECK-NEXT:   br label %cpu_feature_init_cmp
; CHECK-EMPTY:
; CHECK-NEXT: cpu_feature_init_rest:                            ; preds = %cpu_feature_init_cmp
; CHECK-NEXT:   %cpu_feature_indicator1 = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator, i64 0, i64 0), align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator1, 10330092
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 10330092
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %cpu_feature_init_rest
; CHECK-NEXT:   ret i32 (i32)* @bar.V
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %cpu_feature_init_rest
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
