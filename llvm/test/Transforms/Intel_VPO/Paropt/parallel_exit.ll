; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare),function-attrs,function(simplifycfg,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Pass conditon: There is no crash in WRegion construction, due to the
; `region.exit` directive being deleted for being unreachable by simplifycfg pass.

; Test Src:
;
; #include <stdio.h>
; #include <omp.h>
; #include <stdlib.h>
;
; void foo() {
; #pragma omp parallel
;   {
;     int tid = omp_get_thread_num();
;     printf("tid = %d\n", tid);
;       exit(1);
;   }
; }
;
; int main() {
;   foo();
;   return 0;
; }


; CHECK: call {{.+}} @__kmpc_fork_call

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1

define dso_local void @foo() {
entry:
  %tid = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tid, i32 0, i32 1) ]

  %call = call i32 @omp_get_thread_num()
  store i32 %call, ptr %tid, align 4
  %1 = load i32, ptr %tid, align 4
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %1)
  call void @exit(i32 noundef 1) #6
  unreachable

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @omp_get_thread_num()

declare dso_local i32 @printf(ptr noundef, ...)

declare dso_local void @exit(i32 noundef)

define dso_local i32 @main() #5 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @foo()
  ret i32 0
}
