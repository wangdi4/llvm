; Checks that %c0 is not captured by the OMP.PARALLEL directive.

; RUN: opt -S -passes="function-attrs" %s | FileCheck %s

; CHECK: define{{.*}}@const_load(ptr nocapture %c0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define dso_local void @const_load(ptr %c0) {
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(ptr %c0) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}
