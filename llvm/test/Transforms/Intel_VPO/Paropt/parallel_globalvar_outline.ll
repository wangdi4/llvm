; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; unsigned int NY = 100;
;
; void lbm_3() {
;   NY = 100;
; #pragma omp parallel firstprivate(NY)
;   {
;     NY = 102;
; #pragma omp parallel shared(NY)
;     {
;       printf("NY= %d \n", NY); // should print 102
;     }
;   }
;   return;
; }
;
; int main() {
;   lbm_3();
;   return 0;
; }

; parallel shared
; CHECK: define internal void @lbm_3.DIR.OMP.PARALLEL.{{.*}}(ptr %tid, ptr %bid, ptr %NY)
; CHECK: {{%[0-9]+}} = load i32, ptr %NY, align 4

; parallel firstprivate
; CHECK: define internal void @lbm_3.DIR.OMP.PARALLEL.{{.*}}(ptr %tid, ptr %bid, i64 %NY.val.zext)
; CHECK: %NY.fpriv = alloca i32, align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@NY = dso_local global i32 100, align 4
@.str = private unnamed_addr constant [9 x i8] c"NY= %d \0A\00", align 1

define dso_local void @lbm_3() {
entry:
  store i32 100, ptr @NY, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @NY, i32 0, i32 1) ]

  store i32 102, ptr @NY, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @NY, i32 0, i32 1) ]

  %2 = load i32, ptr @NY, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %2)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @lbm_3()
  ret i32 0
}
