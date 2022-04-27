; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


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


; ModuleID = 'task_final_expr.c'
source_filename = "task_final_expr.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"passed\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"failed\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %error = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  store i32 0, i32* %error, align 4
  %0 = load i32, i32* %n.addr, align 4
  %cmp = icmp sle i32 %0, 10
  %conv = zext i1 %cmp to i32
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FINAL"(i32 %conv), "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %error) ]

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

  %2 = load i32, i32* %n.addr, align 4
  %cmp1 = icmp sle i32 %2, 10
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = call i32 @omp_in_final() #1
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
  %call4 = call i32 @omp_in_final() #1
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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind
declare dso_local i32 @omp_in_final() #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %error = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %error, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %error), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  fence acquire
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %2, 20
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, i32* %i, align 4
  %call = call i32 @foo(i32 %3) #1
  %4 = load i32, i32* %error, align 4
  %add = add nsw i32 %4, %call
  store i32 %add, i32* %error, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %5 = load i32, i32* %i, align 4
  %inc = add nsw i32 %5, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %6 = load i32, i32* %error, align 4
  %cmp1 = icmp eq i32 %6, 0
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.end
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0))
  br label %if.end

if.else:                                          ; preds = %for.end
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0))
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %7 = load i32, i32* %error, align 4
  ret i32 %7
}

declare dso_local i32 @printf(i8*, ...) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
