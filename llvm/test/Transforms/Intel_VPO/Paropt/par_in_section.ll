; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Pass condition: successful compilation. The test would fail if
; vpo-cfg-restructuring doesn't add an extra empty BBlock before the entry
; directive for the parallel construct, as loop recognition for the
; outer sections construct fails.
;
; CHECK: call void {{.*}} @__kmpc_fork_call

; Test src:
;
; #include <stdio.h>
; void foo() {
; #pragma omp sections
;   {
; #pragma omp section
;     {
; #pragma omp parallel
;       {
;         printf("a\n");
;       }
;     }
;
; #pragma omp section
;     {
;       printf("b\n");
;     }
;   }
; }

; ModuleID = 'par_in_section.c'
source_filename = "par_in_section.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"a\0A\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"b\0A\00", align 1

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]

  %call = call i32 (ptr, ...) @printf(ptr noundef @.str)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SECTION"() ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]

  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SECTION"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTIONS"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
