; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <omp.h>
; #include <stdio.h>
;
; int *yptr;
; #define NT 4
; #define INIT 2
;
; __attribute__((noinline)) void foo(int n, int size) {
;   int y[10];
;   y[n] = INIT;
;   yptr = &y[n];
;
; #pragma omp parallel reduction(+ : y [n:size]) num_threads(NT)
;   {
;     y[n]++;
;     printf("tid = %d, y = %d, yptr = %p, &y = %p\n", omp_get_thread_num(), y[n],
;            yptr, &y[n]);
;   }
;   printf("y = %d (expected = %d)\n", y[n], INIT + NT);
; }
;
; int main() { foo(2, 3); }

; Check that the LB, UB exprs for the array section are captured
; CHECK: store i64 %sec.number_of_elements, ptr [[LB_ADDR:%[a-zA-Z._0-9]+]], align 8
; CHECK: store i64 %sec.offset_in_elements, ptr [[SIZE_ADDR:%[a-zA-Z._0-9]+]], align 8

; Check that the captured pointers are sent in to the outlined function
; CHECK: call void {{.+}} @__kmpc_fork_call(ptr {{.+}}, i32 4, ptr @foo.{{.+}}, ptr [[SIZE_ADDR]], ptr [[LB_ADDR]], ptr %n.addr, ptr %y)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@yptr = dso_local global ptr null, align 8
@.str = private unnamed_addr constant [38 x i8] c"tid = %d, y = %d, yptr = %p, &y = %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [24 x i8] c"y = %d (expected = %d)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 noundef %n, i32 noundef %size) #0 {
entry:
  %n.addr = alloca i32, align 4
  %size.addr = alloca i32, align 4
  %y = alloca [10 x i32], align 16
  store i32 %n, ptr %n.addr, align 4
  store i32 %size, ptr %size.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %idxprom
  store i32 2, ptr %arrayidx, align 4
  %1 = load i32, ptr %n.addr, align 4
  %idxprom1 = sext i32 %1 to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %idxprom1
  store ptr %arrayidx2, ptr @yptr, align 8
  %2 = load i32, ptr %n.addr, align 4
  %conv = sext i32 %2 to i64
  %3 = load i32, ptr %size.addr, align 4
  %conv3 = sext i32 %3 to i64
  %sec.base.cast = ptrtoint ptr %y to i64
  %4 = load i32, ptr %n.addr, align 4
  %5 = sext i32 %4 to i64
  %arrayidx4 = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %5
  %sec.lower.cast = ptrtoint ptr %arrayidx4 to i64
  %6 = load i32, ptr %n.addr, align 4
  %7 = sext i32 %6 to i64
  %8 = load i32, ptr %size.addr, align 4
  %9 = sext i32 %8 to i64
  %lb_add_len = add nsw i64 %7, %9
  %idx_sub_1 = sub nsw i64 %lb_add_len, 1
  %arrayidx5 = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %idx_sub_1
  %sec.upper.cast = ptrtoint ptr %arrayidx5 to i64
  %10 = sub i64 %sec.upper.cast, %sec.lower.cast
  %11 = sdiv exact i64 %10, 4
  %sec.number_of_elements = add i64 %11, 1
  %12 = sub i64 %sec.lower.cast, %sec.base.cast
  %sec.offset_in_elements = sdiv exact i64 %12, 4
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr %y, i32 0, i64 %sec.number_of_elements, i64 %sec.offset_in_elements),
    "QUAL.OMP.NUM_THREADS"(i32 4),
    "QUAL.OMP.SHARED:TYPED"(ptr @yptr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %n.addr, i32 0, i32 1) ]
  %14 = load i32, ptr %n.addr, align 4
  %idxprom6 = sext i32 %14 to i64
  %arrayidx7 = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %idxprom6
  %15 = load i32, ptr %arrayidx7, align 4
  %inc = add nsw i32 %15, 1
  store i32 %inc, ptr %arrayidx7, align 4
  %call = call i32 @omp_get_thread_num() #1
  %16 = load i32, ptr %n.addr, align 4
  %idxprom8 = sext i32 %16 to i64
  %arrayidx9 = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %idxprom8
  %17 = load i32, ptr %arrayidx9, align 4
  %18 = load ptr, ptr @yptr, align 8
  %19 = load i32, ptr %n.addr, align 4
  %idxprom10 = sext i32 %19 to i64
  %arrayidx11 = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %idxprom10
  %call12 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %call, i32 noundef %17, ptr noundef %18, ptr noundef %arrayidx11) #1
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.PARALLEL"() ]
  %20 = load i32, ptr %n.addr, align 4
  %idxprom13 = sext i32 %20 to i64
  %arrayidx14 = getelementptr inbounds [10 x i32], ptr %y, i64 0, i64 %idxprom13
  %21 = load i32, ptr %arrayidx14, align 4
  %call15 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %21, i32 noundef 6)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

; Function Attrs: nounwind
declare dso_local i32 @omp_get_thread_num() #3

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #4 {
entry:
  call void @foo(i32 noundef 2, i32 noundef 3)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
