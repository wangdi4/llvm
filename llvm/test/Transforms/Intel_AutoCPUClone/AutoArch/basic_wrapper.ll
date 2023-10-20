; RUN: opt -passes=auto-cpu-clone -acd-enable-all < %s -S | FileCheck %s


; CHECK:      @baz.ptr = internal global ptr @baz.A, !llvm.acd.dispatcher !0
; CHECK-NEXT: @llvm.global_ctors = appending global [2 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 500, ptr @__intel.acd.resolver, ptr null }, { i32, ptr, ptr } { i32 0, ptr @__intel_cpu_features_init_x, ptr null }]
; CHECK-NEXT: @__intel_cpu_feature_indicator_x = external global [2 x i64]
; CHECK-NEXT: @foo.ptr = internal global ptr @foo.A, !llvm.acd.dispatcher !0
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @baz.A(i32 %a) #0 comdat !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @foo.A(i32 %a) #0 comdat !llvm.acd.clone !0 {
; CHECK-NEXT:   %ret = call i32 @baz.A(i32 33)
; CHECK-NEXT:   %add = add i32 %a, %ret
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @bar(i32 %a) #0 comdat {
; CHECK-NEXT:   %ret = call i32 @foo(i32 42)
; CHECK-NEXT:   ret i32 %ret
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @baz.V(i32 %a) #1 !llvm.acd.clone !0 {
; CHECK-NEXT:   %add = add i32 %a, 42
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define internal void @__intel.acd.resolver() #0 {
; CHECK-NEXT: resolver_entry:
; CHECK-NEXT:   %cpu_feature_indicator = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
; CHECK-NEXT:   %cpu_feature_join = and i64 %cpu_feature_indicator, 10330094
; CHECK-NEXT:   %cpu_feature_check = icmp eq i64 %cpu_feature_join, 10330094
; CHECK-NEXT:   br i1 %cpu_feature_check, label %resolver_return, label %resolver_else
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return:                                  ; preds = %resolver_entry
; CHECK-NEXT:   store ptr @baz.V, ptr @baz.ptr, align 8
; CHECK-NEXT:   br label %resolver_exit
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else:                                    ; preds = %resolver_entry
; CHECK-NEXT:   store ptr @baz.A, ptr @baz.ptr, align 8
; CHECK-NEXT:   br label %resolver_exit
; CHECK-EMPTY:
; CHECK-NEXT: resolver_exit:                                    ; preds = %resolver_else, %resolver_return
; CHECK-NEXT:   %cpu_feature_indicator2 = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
; CHECK-NEXT:   %cpu_feature_join3 = and i64 %cpu_feature_indicator2, 10330094
; CHECK-NEXT:   %cpu_feature_check4 = icmp eq i64 %cpu_feature_join3, 10330094
; CHECK-NEXT:   br i1 %cpu_feature_check4, label %resolver_return5, label %resolver_else6
; CHECK-EMPTY:
; CHECK-NEXT: resolver_return5:                                 ; preds = %resolver_exit
; CHECK-NEXT:   store ptr @foo.V, ptr @foo.ptr, align 8
; CHECK-NEXT:   br label %resolver_exit1
; CHECK-EMPTY:
; CHECK-NEXT: resolver_else6:                                   ; preds = %resolver_exit
; CHECK-NEXT:   store ptr @foo.A, ptr @foo.ptr, align 8
; CHECK-NEXT:   br label %resolver_exit1
; CHECK-EMPTY:
; CHECK-NEXT: resolver_exit1:                                   ; preds = %resolver_else6, %resolver_return5
; CHECK-NEXT:   ret void
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: declare intel_features_init_cc void @__intel_cpu_features_init_x()
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @baz(i32 %a) #0 !llvm.acd.dispatcher !0 {
; CHECK-NEXT:   %1 = load ptr, ptr @baz.ptr, align 8
; CHECK-NEXT:   %2 = call i32 %1(i32 %a)
; CHECK-NEXT:   ret i32 %2
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @foo.V(i32 %a) #1 !llvm.acd.clone !0 {
; CHECK-NEXT:   %ret = call i32 @baz.V(i32 33)
; CHECK-NEXT:   %add = add i32 %a, %ret
; CHECK-NEXT:   ret i32 %add
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: define i32 @foo(i32 %a) #0 !llvm.acd.dispatcher !0 {
; CHECK-NEXT:   %1 = load ptr, ptr @foo.ptr, align 8
; CHECK-NEXT:   %2 = call i32 %1(i32 %a)
; CHECK-NEXT:   ret i32 %2
; CHECK-NEXT: }
; CHECK-EMPTY:
; CHECK-NEXT: attributes #0 = { "advanced-optim"="false" }
; CHECK-NEXT: attributes #1 = { "advanced-optim"="false" "target-cpu"="haswell" "target-features"="+avx,+avx2,+bmi,+bmi2,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "tune-cpu"="haswell" }


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

$baz = comdat any
$foo = comdat any
$bar = comdat any

define i32 @baz(i32 %a) comdat !llvm.auto.arch !0 {
  %add = add i32 %a, 42
  ret i32 %add
}

define i32 @foo(i32 %a) comdat !llvm.auto.arch !0 {
  %ret = call i32 @baz(i32 33)
  %add = add i32 %a, %ret
  ret i32 %add
}

define i32 @bar(i32 %a) comdat {
  %ret = call i32 @foo(i32 42)
  ret i32 %ret
}

!0 = !{!"haswell"}
