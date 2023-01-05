; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='intel-math-libraries-decl' -enable-intel-advanced-opts -S < %s 2>&1 | FileCheck %s

; Check that a declaration of __intel_new_feature_proc_init is generated
; when auto CPU dispatch is employed and -intel-libirc-allowed=true and
; that metadata is added appropriately.

; CHECK: @llvm.compiler.used = {{.*}}@__intel_new_feature_proc_init{{.*}}section "llvm.metadata", !intel_dtrans_type !0
; CHECK: declare void @__intel_new_feature_proc_init(i32, i64)
; CHECK: !0 = !{!"A", i32 1, !1}
; CHECK: !1 = !{i8 0, i32 1}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

!intel.dtrans.types = !{}

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
