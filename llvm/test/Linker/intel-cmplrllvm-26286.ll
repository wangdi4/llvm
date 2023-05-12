; RUN: opt -module-summary %s -o - | llvm-dis | FileCheck %s

; Check that MCD.resolver is marked as live in module-summary.

; CHECK: (name: "MCD.resolver",
; CHECK-SAME: live: 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$MCD.resolver = comdat any

@__intel_cpu_feature_indicator_x = external dso_local local_unnamed_addr global [2 x i64]
@ok = dso_local local_unnamed_addr global i32 0, align 4

@MCD = weak_odr dso_local alias void (), ptr @MCD.ifunc

@MCD.ifunc = weak_odr dso_local ifunc void (), ptr @MCD.resolver

declare dso_local void @__intel_cpu_features_init_x() local_unnamed_addr

define weak_odr ptr @MCD.resolver() comdat {
resolver_entry:
  tail call void @__intel_cpu_features_init_x()
  %cpu_feature_indicator = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  %cpu_feature_join = and i64 %cpu_feature_indicator, 108
  %cpu_feature_check = icmp eq i64 %cpu_feature_join, 108
  %spec.select = select i1 %cpu_feature_check, ptr @MCD.J, ptr @MCD.A
  ret ptr %spec.select
}

define internal void @MCD.A() {
entry:
  store i32 1, ptr @ok, align 4
  ret void
}

define internal void @MCD.J() {
entry:
  store i32 2, ptr @ok, align 4
  ret void
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  store i32 0, ptr @ok, align 4
  tail call void @MCD.ifunc()
  %i = load i32, ptr @ok, align 4
  ret i32 %i
}
