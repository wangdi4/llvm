; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int &a) {
; #pragma omp task private(a)
;   {
;     a = 2;
;     printf("%d\n", a);
;   }
; }
;
; // int main() {
; //   int a;
; //   a = 0;
; //   foo(a);
; //   return 0;
; // }

; Check for the space allocated for the private copy.
; CHECK: %__struct.kmp_privates.t = type { i32 }

; For the outlined function for the task, check that the address of the local
; copy for '%a.addr.gep' is stored to ptr %a.addr.gep.ref, and then that is used in the region.
; CHECK: store ptr %a.addr.gep, ptr %a.addr.gep.ref, align 8
; CHECK: [[A_PRIVATE_ADDR_LOAD:%[^ ]+]] = load ptr, ptr %a.addr.gep.ref, align 8
; CHECK: store i32 2, ptr [[A_PRIVATE_ADDR_LOAD]], align 4


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local void @_Z3fooRi(ptr noundef nonnull align 4 dereferenceable(4) %a) {
entry:
  %a.addr = alloca ptr, align 8
  store ptr %a, ptr %a.addr, align 8
  %0 = load ptr, ptr %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.PRIVATE:BYREF.TYPED"(ptr %a.addr, i32 0, i32 1) ]

  %2 = load ptr, ptr %a.addr, align 8
  store i32 2, ptr %2, align 4
  %3 = load ptr, ptr %a.addr, align 8
  %4 = load i32, ptr %3, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %4)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)
