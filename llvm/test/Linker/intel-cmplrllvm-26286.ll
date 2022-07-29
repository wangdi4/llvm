; RUN: opt -module-summary %s -o - | llvm-dis | FileCheck %s

; Check that MCD.resolver is marked as live in module-summary.

; CHECK: (name: "MCD.resolver",
; CHECK-SAME: live: 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$MCD.resolver = comdat any
@__intel_cpu_feature_indicator_x = external dso_local local_unnamed_addr global [2 x i64]

@ok = dso_local local_unnamed_addr global i32 0, align 4

@MCD = weak_odr dso_local alias void (), void ()* @MCD.ifunc

@MCD.ifunc = weak_odr dso_local ifunc void (), void ()* ()* @MCD.resolver

declare dso_local void @__intel_cpu_features_init_x() local_unnamed_addr

define weak_odr void ()* @MCD.resolver() comdat {
resolver_entry:
  tail call void @__intel_cpu_features_init_x()
  %cpu_feature_indicator = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  %cpu_feature_join = and i64 %cpu_feature_indicator, 108
  %cpu_feature_check = icmp eq i64 %cpu_feature_join, 108
  %spec.select = select i1 %cpu_feature_check, void ()* @MCD.J, void ()* @MCD.A
  ret void ()* %spec.select
}

; Function Attrs: nofree norecurse nounwind uwtable willreturn writeonly
define internal void @MCD.A() #0 {
entry:
  store i32 1, i32* @ok, align 4
  ret void
}

define internal void @MCD.J() #1 {
entry:
  store i32 2, i32* @ok, align 4
  ret void
}

define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  store i32 0, i32* @ok, align 4
  tail call void @MCD.ifunc() #3
  %0 = load i32, i32* @ok, align 4
  ret i32 %0
}

