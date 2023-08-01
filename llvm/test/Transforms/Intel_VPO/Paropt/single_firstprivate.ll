; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; int main() {
;   int x = 1;
; #pragma omp parallel num_threads(1)
;   {
;     printf("parallel (before): x = %d (expected 1)\n", x);
; #pragma omp single firstprivate(x)
;     {
;       printf("single (before): x = %d (expected 1)\n", x);
;       x = 2;
;       printf("single (after): x = %d (expected 2)\n", x);
;     }
;     printf("parallel (after): x = %d (expected 1)\n", x);
;   }
;   return 0;
; }

; Check that %x was privatized for the single region
; CHECK: "DIR.OMP.PARALLEL"
; CHECK: [[X_FPRIV:%[a-zA-Z._0-9]+]] = alloca i32
; CHECK: kmpc_single
; CHECK: store i32 2, ptr [[X_FPRIV]]
; CHECK: kmpc_end_single

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [40 x i8] c"parallel (before): x = %d (expected 1)\0A\00", align 1
@.str.1 = private unnamed_addr constant [38 x i8] c"single (before): x = %d (expected 1)\0A\00", align 1
@.str.2 = private unnamed_addr constant [37 x i8] c"single (after): x = %d (expected 2)\0A\00", align 1
@.str.3 = private unnamed_addr constant [39 x i8] c"parallel (after): x = %d (expected 1)\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 1, ptr %x, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1) ]
  %1 = load i32, ptr %x, align 4
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %1)

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %x, i32 0, i32 1) ]
  fence acquire
  %3 = load i32, ptr %x, align 4
  %call1 = call i32 (ptr, ...) @printf(ptr @.str.1, i32 %3)
  store i32 2, ptr %x, align 4
  %4 = load i32, ptr %x, align 4
  %call2 = call i32 (ptr, ...) @printf(ptr @.str.2, i32 %4)
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]


  %5 = load i32, ptr %x, align 4
  %call3 = call i32 (ptr, ...) @printf(ptr @.str.3, i32 %5)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)
