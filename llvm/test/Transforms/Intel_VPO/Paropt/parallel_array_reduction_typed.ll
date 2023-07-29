; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; void foo(void) {
;   double s[17][17] = {0.0};
; #pragma omp parallel reduction(+:s)
;   ;
; }

; CHECK: define internal void @foo_tree_reduce{{.*}}(ptr [[DSTP:%[0-9A-Za-z._]+]], ptr [[SRCP:%[0-9A-Za-z._]+]]) {
; CHECK-DAG: [[DSTS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[DSTP]], i32 0, i32 0
; CHECK-DAG: [[SRCS:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.fast_red_t, ptr [[SRCP]], i32 0, i32 0
; CHECK-DAG: [[DSTSGEP:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[DSTS]], i32 0, i32 0
; CHECK-DAG: [[SRCSGEP:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[SRCS]], i32 0, i32 0
; CHECK-DAG: [[DARRBEGIN:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[DSTSGEP]], i32 0
; CHECK-DAG: [[DARREND:%[0-9A-Za-z._]+]] = getelementptr double, ptr [[DARRBEGIN]], i64 289
; CHECK-DAG: [[SARRBEGIN:%[0-9A-Za-z._]+]] = getelementptr inbounds [289 x double], ptr [[SRCSGEP]], i32 0
; CHECK-DAG: [[ISEMPTY:%[0-9A-Za-z._]+]] = icmp eq ptr [[DARRBEGIN]], [[DARREND]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %s = alloca [17 x [17 x double]], align 16
  call void @llvm.memset.p0.i64(ptr align 16 %s, i8 0, i64 2312, i1 false)
  %array.begin = getelementptr inbounds [17 x [17 x double]], ptr %s, i32 0, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %s, double 0.000000e+00, i64 289) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
