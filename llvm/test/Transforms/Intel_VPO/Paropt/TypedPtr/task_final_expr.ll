; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check task construct with final clause expression, which
; is scalar conditional expression.

; #include <stdio.h>
; #include <omp.h>
;
; int foo (int n)
; {
;   int error = 0;
; #pragma omp task final(n <= 10)
;   {
;     if(n <= 10) {
;       if (!omp_in_final()) {
;         error +=1;
;       }
;     } else {
;       if (omp_in_final()) {
;         error += 1;
;       }
;     }
;   }
;   return error;
; }
;
; int main()
; {
;   int error = 0;
;
; #pragma omp parallel
;   {
; #pragma omp single
;     {
;       for (int i=0; i<20; i++)
;         error += foo(i);
;     }
;   }
;
;   if (error == 0)
;     printf("passed\n");
;   else
;     printf("failed\n");
;   return error;
; }

; CHECK-LABEL: codeRepl:
; CHECK: %[[TASK_FLAG_ALLOC:[^,]+]] = alloca i32, align 4
; CHECK-NEXT: store i32 1, i32* %[[TASK_FLAG_ALLOC]], align 4
; CHECK-NEXT: %[[CMP:[^,]+]] = icmp ne i32 %{{.*}}, 0
; CHECK-NEXT: br i1 %[[CMP]], label %[[IF_THEN:[^,]+]], label %[[IF_ELSE:[^,]+]]
; CHECK: [[IF_THEN]]:
; CHECK-NEXT: store i32 3, i32* %[[TASK_FLAG_ALLOC]], align 4
; CHECK-NEXT: br label %[[IF_END:[^,]+]]
; CHECK: [[IF_ELSE]]:
; CHECK-NEXT: br label %[[IF_END]]
; CHECK: [[IF_END]]:
; CHECK-NEXT: %[[TASK_FLAG_LOAD:[^,]+]] = load i32, i32* %[[TASK_FLAG_ALLOC]], align 4
; CHECK-NEXT: %{{.*}} = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* {{.*}}, i32 {{.*}}, i32 %[[TASK_FLAG_LOAD]], {{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"passed\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"failed\0A\00", align 1

define dso_local i32 @foo(i32 %n) {
entry:
  %n.addr = alloca i32, align 4
  %error = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  store i32 0, i32* %error, align 4
  %0 = load i32, i32* %n.addr, align 4
  %cmp = icmp sle i32 %0, 10
  %conv = zext i1 %cmp to i32
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FINAL"(i32 %conv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %error) ]

  %2 = load i32, i32* %n.addr, align 4
  %cmp1 = icmp sle i32 %2, 10
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = call i32 @omp_in_final()
  %tobool = icmp ne i32 %call, 0
  br i1 %tobool, label %if.end, label %if.then3

if.then3:                                         ; preds = %if.then
  %3 = load i32, i32* %error, align 4
  %add = add nsw i32 %3, 1
  store i32 %add, i32* %error, align 4
  br label %if.end

if.end:                                           ; preds = %if.then3, %if.then
  br label %if.end9

if.else:                                          ; preds = %entry
  %call4 = call i32 @omp_in_final()
  %tobool5 = icmp ne i32 %call4, 0
  br i1 %tobool5, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.else
  %4 = load i32, i32* %error, align 4
  %add7 = add nsw i32 %4, 1
  store i32 %add7, i32* %error, align 4
  br label %if.end8

if.end8:                                          ; preds = %if.then6, %if.else
  br label %if.end9

if.end9:                                          ; preds = %if.end8, %if.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  %5 = load i32, i32* %error, align 4
  ret i32 %5
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @omp_in_final()
