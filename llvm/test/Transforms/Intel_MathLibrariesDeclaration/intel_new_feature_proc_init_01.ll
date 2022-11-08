; RUN: opt -passes=intel-math-libraries-decl -enable-intel-advanced-opts -intel-libirc-allowed=true -S < %s 2>&1 | FileCheck %s

; Check that a declaration of __intel_new_feature_proc_init is generated
; when -enable-intel-advanced-opts and -intel-libirc-allowed=true.

; CHECK: @llvm.compiler.used = {{.*}}@__intel_new_feature_proc_init
; CHECK: declare void @__intel_new_feature_proc_init(i32, i64)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo() {
  ret i32 0
}

