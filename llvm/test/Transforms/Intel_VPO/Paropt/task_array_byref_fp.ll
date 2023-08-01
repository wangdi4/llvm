; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int (&a)[10]) {
; #pragma omp task firstprivate(a)
;   {
;     a[1] += 1;
;     printf("%d\n", a[1]);
;   }
; }
;
;  //int main() {
;  //  int a[10];
;  //  a[1] = 0;
;  //  foo(a);
;  //  return 0;
;  //}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; Check for the space allocated for the private copy.
; CHECK: %__struct.kmp_privates.t = type { [10 x i32] }

define dso_local void @_Z3fooRA10_i(ptr dereferenceable(40) %a) {
entry:
  %a.addr = alloca ptr, align 8
  store ptr %a, ptr %a.addr, align 8
  %0 = load ptr, ptr %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:BYREF.TYPED"(ptr %a.addr, i32 0, i32 10) ]

; For the outlined function for the task, check that the address of the local
; copy for '%a.addr' is stored to a ptr, and then that is used instead
; of '%a.addr' in the region.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}
; CHECK: store ptr %{{[^ ]+}}, ptr [[A_PRIVATE_ADDR:%[^ ]+]]

; CHECK: [[A_PRIVATE_ADDR_LOAD:%[^ ]+]] = load ptr, ptr [[A_PRIVATE_ADDR]]
; CHECK: [[A_PRIVATE_ADDR_LOAD_GEP:%[^ ]+]] = getelementptr inbounds [10 x i32], ptr [[A_PRIVATE_ADDR_LOAD]], i64 0, i64 1
; CHECK: [[A_PRIVATE_ADDR_LOAD_GEP_LOAD:%[^ ]+]] = load i32, ptr [[A_PRIVATE_ADDR_LOAD_GEP]]
; CHECK: %{{[^ ]+}} = add nsw i32 [[A_PRIVATE_ADDR_LOAD_GEP_LOAD]], 1

  %2 = load ptr, ptr %a.addr, align 8
  %arrayidx = getelementptr inbounds [10 x i32], ptr %2, i64 0, i64 1
  %3 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %3, 1

  store i32 %add, ptr %arrayidx, align 4
  %4 = load ptr, ptr %a.addr, align 8
  %arrayidx1 = getelementptr inbounds [10 x i32], ptr %4, i64 0, i64 1
  %5 = load i32, ptr %arrayidx1, align 4
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %5)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)
