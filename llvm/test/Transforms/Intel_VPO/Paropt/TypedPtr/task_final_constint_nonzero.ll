; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check task construct with final clause expression, which
; is constant integer with non-zero value.
;
; #include <stdio.h>
; #include <omp.h>
; void foo ( )
; {
; #pragma omp task final(8)
;   {
;     if(omp_in_final()) {
;     printf("passed\n");
;     }
;     else {
;     printf("failed\n");
;     }
;   }
; }
;
; int main()
; {
; #pragma omp parallel
;   {
; #pragma omp single
;     foo();
;   }
; }

; The flag of 3rd argument in __kmpc_omp_task_alloc is expected to 3.
; CHECK: call i8* @__kmpc_omp_task_alloc(%struct.ident_t* {{.*}}, i32 {{.*}}, i32 3, {{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"passed\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"failed\0A\00", align 1

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FINAL"(i32 8) ]

  %call = call i32 @omp_in_final()
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0))
  br label %if.end

if.else:                                          ; preds = %entry
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0))
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @omp_in_final()
declare dso_local i32 @printf(i8*, ...)
