; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; void foo(void) {
;   double s = 0.0;
; #pragma omp parallel reduction(+:s)
;   ;
; }

; CHECK: define internal void @foo_tree_reduce{{.*}}(ptr [[DSTP:%[0-9A-Za-z._]+]], ptr [[SRCP:%[0-9A-Za-z._]+]]) {
; CHECK-DAG: [[DSTS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[DSTP]], i32 0, i32 0
; CHECK-DAG: [[SRCS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[SRCP]], i32 0, i32 0
; CHECK-DAG: [[DSTV:%[0-9A-Za-z._]+]] = load double, ptr [[DSTS]]
; CHECK-DAG: [[SRCV:%[0-9A-Za-z._]+]] = load double, ptr [[SRCS]]
; CHECK: [[ADD:%[0-9A-Za-z._]+]] = fadd double [[DSTV]], [[SRCV]]
; CHECK: store double [[ADD]], ptr [[DSTS]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %s = alloca double, align 8
  store double 0.000000e+00, ptr %s, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %s, double 0.000000e+00, i32 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
