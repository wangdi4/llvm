; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(intel-ir-optreport-emitter)' -intel-opt-report=high -intel-opt-report-file=stdout -disable-output < %s | FileCheck %s --strict-whitespace

; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; void foo(int *a, int *b, int *c) {
;   int i;
; #pragma omp parallel
;   {
;     a[i] = b[i] + c[i];
;
; #pragma omp master
;     printf("tid = %d\n", omp_get_thread_num());
;   }
; }

; Test to check that "construct transformed" opt-report messages are printed for
; VPO OpenMP. This test is based on optrpt_yaml_par.ll.

; CHECK: Global optimization report for : foo.DIR.OMP.PARALLEL.[[#]]

; CHECK: OMP PARALLEL BEGIN
; CHECK:     remark #30008: parallel construct transformed

; CHECK:     OMP MASKED BEGIN
; CHECK:         remark #30008: masked construct transformed
; CHECK:     OMP MASKED END
; CHECK: OMP PARALLEL END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1

define dso_local void @foo(ptr noundef %a, ptr noundef %b, ptr noundef %c) {
entry:
  %a.addr = alloca ptr, align 8
  %b.addr = alloca ptr, align 8
  %c.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  store ptr %b, ptr %b.addr, align 8
  store ptr %c, ptr %c.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %b.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %c.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1) ]
  %1 = load ptr, ptr %b.addr, align 8
  %2 = load i32, ptr %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 %idxprom
  %3 = load i32, ptr %arrayidx, align 4
  %4 = load ptr, ptr %c.addr, align 8
  %5 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %4, i64 %idxprom1
  %6 = load i32, ptr %arrayidx2, align 4
  %add = add nsw i32 %3, %6
  %7 = load ptr, ptr %a.addr, align 8
  %8 = load i32, ptr %i, align 4
  %idxprom3 = sext i32 %8 to i64
  %arrayidx4 = getelementptr inbounds i32, ptr %7, i64 %idxprom3
  store i32 %add, ptr %arrayidx4, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %call = call i32 @omp_get_thread_num() #1
  %call5 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %call) #1
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)
declare dso_local i32 @omp_get_thread_num()
