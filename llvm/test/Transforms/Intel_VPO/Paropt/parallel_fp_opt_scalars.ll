; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s

; Test src:
;
; void foo() {
;   float x;
;   double y;
;   int i;
;   long long int j;
;   char c;
;   long double ld;
; #pragma omp parallel firstprivate(x, y, i, j, c, ld)
;   {
;     (void)x;(void)y;(void)i;(void)j;(void)c;(void)ld;
;   }
; }

; Check that float, double, i32, i64 and i8 firsrprivates are passed by value:
; CHECK-DAG: %x = alloca float
; CHECK-DAG: %y = alloca double
; CHECK-DAG: %i = alloca i32
; CHECK-DAG: %j = alloca i64
; CHECK-DAG: %c = alloca i8
; CHECK-DAG: %ld = alloca x86_fp80
; CHECK-DAG: [[x_val:%[a-zA-Z._0-9]+]] = load float, ptr %x
; CHECK-DAG: [[x_val_bcast:%[a-zA-Z._0-9]+]] = bitcast float [[x_val]] to i32
; CHECK-DAG: [[x_val_bcast_zext:%[a-zA-Z._0-9]+]] = zext i32 [[x_val_bcast]] to i64
; CHECK-DAG: [[y_val:%[a-zA-Z._0-9]+]] = load double, ptr %y
; CHECK-DAG: [[y_val_bcast:%[a-zA-Z._0-9]+]] = bitcast double [[y_val]] to i64
; CHECK-DAG: [[i_val:%[a-zA-Z._0-9]+]] = load i32, ptr %i
; CHECK-DAG: [[i_val_zext:%[a-zA-Z._0-9]+]] = zext i32 [[i_val]] to i64
; CHECK-DAG: [[j_val:%[a-zA-Z._0-9]+]] = load i64, ptr %j
; CHECK-DAG: [[c_val:%[a-zA-Z._0-9]+]] = load i8, ptr %c
; CHECK-DAG: [[c_val_zext:%[a-zA-Z._0-9]+]] = zext i8 [[c_val]] to i64
; CHECK: call void{{.*}}@__kmpc_fork_call({{[^,]*}}, i32 6, ptr @foo{{[a-zA-Z._0-9]*}}, ptr %ld, i64 [[c_val_zext]], i64 [[j_val]], i64 [[i_val_zext]], i64 [[y_val_bcast]], i64 [[x_val_bcast_zext]])
; CHECK-DAG: define internal void @foo{{[a-zA-Z._0-9]*}}(ptr {{[^,]*}}, ptr {{[^,]*}}, ptr {{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}}, i64{{[^,]*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %x = alloca float, align 4
  %y = alloca double, align 8
  %i = alloca i32, align 4
  %j = alloca i64, align 8
  %c = alloca i8, align 1
  %ld = alloca x86_fp80, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %x, float 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, double 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %j, i64 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %c, i8 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %ld, x86_fp80 0xK00000000000000000000, i32 1) ]

  %1 = load float, ptr %x, align 4
  %2 = load double, ptr %y, align 8
  %3 = load i32, ptr %i, align 4
  %4 = load i64, ptr %j, align 8
  %5 = load i8, ptr %c, align 1
  %6 = load x86_fp80, ptr %ld, align 16
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
