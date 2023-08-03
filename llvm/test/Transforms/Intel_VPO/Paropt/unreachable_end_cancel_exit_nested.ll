; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -S %s | FileCheck %s -check-prefix=SIMPL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplify)' -S %s | FileCheck %s -check-prefix=SIMPL

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM

; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; void foo() {
;
; #pragma omp parallel num_threads(2)
;   {
; #pragma omp parallel num_threads(4)
;     {
;
;       int i = omp_get_thread_num();
; //      printf("%d: before\n", i);
;
;       while (1) {
; #pragma omp cancel parallel
;         exit(1);
; //      printf("%d: after\n", 10);
;       }
; //      printf("%d: after\n", i);
;     }
;
; //    printf("%d: outer parallel\n", omp_get_thread_num());
;   }
; }
;
; /*
; int main() {
;   foo();
;   return 0;
; }*/

; Check for a branch emitted from begin to end directive after vpo-paropt-prepare phase.
; PREPR: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; PREPR-SAME: "QUAL.OMP.JUMP.TO.END.IF"(ptr [[BRANCH_TEMP:%[^ ,]+]])
; PREPR-NEXT: [[TEMP_LOAD:%[^ ]+]] = load volatile i1, ptr [[BRANCH_TEMP]]
; PREPR-NEXT: [[CMP:%[^ ]+]] = icmp ne i1 [[TEMP_LOAD]], false
; PREPR-NEXT: br i1 [[CMP]], label %[[END_LABEL:[^ ,]+]], label %{{.*}}
; PREPR: [[END_LABEL]]:
; PREPR-NEXT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL"() ]


; After simplifycfg, there should be no printf left in the IR as it's unreachable,
; but the two end region directives should still be present.
; SIMPL-NOT: call i32 (ptr, ...) @printf
; SIMPL: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL"() ]
; SIMPL: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL"() ]

; After transform, there should be no directives left, and if we don't comp-fail,
; then it means we're fine.
; TFORM-NOT:  call token @llvm.directive.region.entry()
; TFORM: call void {{.+}} @__kmpc_fork_call
; TFORM: call i32 @__kmpc_cancel

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"%d: after\0A\00", align 1

define dso_local void @foo() {
entry:
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 4),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %call = call i32 @omp_get_thread_num()
  store i32 %call, ptr %i, align 4
  br label %while.cond

while.cond:                                       ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.cond
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(),
    "QUAL.OMP.CANCEL.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.CANCEL"() ]
  call void @exit(i32 1) #0
  %call1 = call i32 (ptr, ...) @printf(ptr @.str, i32 10)
  unreachable

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @omp_get_thread_num()
declare dso_local i32 @printf(ptr, ...)
declare dso_local void @exit(i32) #0

attributes #0 = { noreturn nounwind }
