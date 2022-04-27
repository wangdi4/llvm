; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int n) {
;   short a = 1;
;
; //#pragma omp parallel
; //#pragma omp single
; #pragma omp taskloop num_tasks(n)
;   for (int i = 0; i < n; i++)
;   {
;     printf("%d\n", a);
;   }
; }
;
; //int main() {
; //  foo(10);
; //}

; Check that the task's outlined function doesn't contain any extra argument for the num_tasks operand '%4'.
; CHECK: define internal void @{{[^ ]+}}DIR.OMP.TASK{{[^ ]+}}(i32 {{[^ ]+}}, %__struct.kmp_task_t_with_privates* {{[^ ]+}})

; ModuleID = 'taskloop_numtasks_var.c'
source_filename = "taskloop_numtasks_var.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %a = alloca i16, align 2
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  store i16 1, i16* %a, align 2
  %0 = load i32, i32* %n.addr, align 4
  store i32 %0, i32* %.capture_expr., align 4
  %1 = load i32, i32* %.capture_expr., align 4
  %sub = sub nsw i32 %1, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, i32* %.capture_expr.1, align 4
  %2 = load i32, i32* %.capture_expr., align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i64 0, i64* %.omp.lb, align 8
  %3 = load i32, i32* %.capture_expr.1, align 4
  %conv = sext i32 %3 to i64
  store i64 %conv, i64* %.omp.ub, align 8
  %4 = load i32, i32* %n.addr, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.NUM_TASKS"(i32 %4), "QUAL.OMP.SHARED"(i32* %n.addr), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i16* %a) ]
  %6 = load i64, i64* %.omp.lb, align 8
  %conv4 = trunc i64 %6 to i32
  store i32 %conv4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, i32* %.omp.iv, align 4
  %conv5 = sext i32 %7 to i64
  %8 = load i64, i64* %.omp.ub, align 8
  %cmp6 = icmp ule i64 %conv5, %8
  br i1 %cmp6, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add8 = add nsw i32 0, %mul
  store i32 %add8, i32* %i, align 4
  %10 = load i16, i16* %a, align 2
  %conv9 = sext i16 %10 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %conv9)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4
  %add10 = add nsw i32 %11, 1
  store i32 %add10, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
