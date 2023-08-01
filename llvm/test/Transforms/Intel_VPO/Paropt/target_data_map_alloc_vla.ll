; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check that the map-type for %1 is not changed from 0 to something else
; (like 0x23) by Paropt.
; CHECK: [[MAPTYPES:@.offload_maptypes]] = {{.*}} [1 x i64] zeroinitializer
; CHECK: call void @__tgt_target_data_begin_mapper({{.*}}ptr [[MAPTYPES]]{{.*}})
; CHECK: call void @__tgt_target_data_end_mapper({{.*}}ptr [[MAPTYPES]]{{.*}})

; Test src:
;
; #include <stdio.h>
;
; void foo(int *x, int n) {
;
;   printf("%p\n", &x[0]);
; #pragma omp target data map(alloc : x [0:n])
;   { printf("%p\n", &x[0]); }
; }
;
; int main() {
;   int y[2];
;   foo(&y[0], 2);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

define dso_local void @foo(ptr %x, i32 %n) {
entry:
  %x.addr = alloca ptr, align 8
  %n.addr = alloca i32, align 4
  store ptr %x, ptr %x.addr, align 8
  store i32 %n, ptr %n.addr, align 4
  %0 = load ptr, ptr %x.addr, align 8
  %call = call i32 (ptr, ...) @printf(ptr @.str, ptr %0)
  %1 = load ptr, ptr %x.addr, align 8
  %2 = load ptr, ptr %x.addr, align 8
  %3 = load i32, ptr %n.addr, align 4
  %conv = sext i32 %3 to i64
  %4 = mul nuw i64 %conv, 4

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.MAP.TOFROM"(ptr %1, ptr %2, i64 %4, i64 0, ptr null, ptr null) ]

  %6 = load ptr, ptr %x.addr, align 8
  %call2 = call i32 (ptr, ...) @printf(ptr @.str, ptr %6)

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET.DATA"() ]

  ret void
}

declare dso_local i32 @printf(ptr, ...)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
