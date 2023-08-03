; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR

; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -S %s | FileCheck %s -check-prefix=SIMPL
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplify)' -S %s | FileCheck %s -check-prefix=SIMPL

; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM

; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; void foo() {
; #pragma omp parallel
;   {
;     int tid = omp_get_thread_num();
; PRINT:
;    printf("tid = %d\n", tid);
;    goto PRINT;
;   }
; }
;
; /*
; int main() {
;   foo();
;   return 0;
; }*/
;

; Check for a branch emitted from begin to end directive after vpo-paropt-prepare phase.
; PREPR: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; PREPR-SAME: "QUAL.OMP.JUMP.TO.END.IF"(i1* [[BRANCH_TEMP:%[^ ,]+]])
; PREPR-NEXT: [[TEMP_LOAD:%[^ ]+]] = load volatile i1, i1* [[BRANCH_TEMP]]
; PREPR-NEXT: [[CMP:%[^ ]+]] = icmp ne i1 [[TEMP_LOAD]], false
; PREPR-NEXT: br i1 [[CMP]], label %[[END_LABEL:[^ ,]+]], label %{{.*}}
; PREPR: [[END_LABEL]]:
; PREPR-NEXT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL"() ]

; After simplifycfg, there should still be an end-region directive in the IR even though
; it is after an infinite loop.
; SIMPL: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL"() ]

; After transform, there should be no directives left, and if we don't comp-fail,
; then it means we're fine.
; TFORM-NOT:  call token @llvm.directive.region.entry()
; TFORM: call void {{.+}} @__kmpc_fork_call

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1

define dso_local void @foo() {
entry:
  %tid = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(i32* %tid, i32 0, i32 1) ]

  %call = call i32 @omp_get_thread_num()
  store i32 %call, i32* %tid, align 4
  br label %PRINT

PRINT:                                            ; preds = %entry
  %1 = load i32, i32* %tid, align 4
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 %1)
  br label %PRINT

end:                                              ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @omp_get_thread_num()
declare dso_local i32 @printf(i8*, ...)
