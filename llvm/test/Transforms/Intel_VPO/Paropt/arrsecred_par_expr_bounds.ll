; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
; #include <omp.h>
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
; #pragma omp parallel reduction(+: y[n:size]) num_threads(NT)
;   {
;     y[n]++;
;     printf("tid = %d, y = %d, yptr = %p, &y = %p\n", omp_get_thread_num(), y[n], yptr, &y[n]);
;   }
;   printf("y = %d (expected = %d)\n", y[n], INIT+NT);
; }
;
; int main() {
;   foo(2, 3);
; }
;
; ModuleID = 'arrsecred_par_expr_bounds.c'
source_filename = "arrsecred_par_expr_bounds.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@yptr = common dso_local global i32* null, align 8
@.str = private unnamed_addr constant [38 x i8] c"tid = %d, y = %d, yptr = %p, &y = %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [24 x i8] c"y = %d (expected = %d)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n, i32 %size) #0 {
entry:
  %n.addr = alloca i32, align 4
  %size.addr = alloca i32, align 4
  %y = alloca [10 x i32], align 16
  store i32 %n, i32* %n.addr, align 4
  store i32 %size, i32* %size.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %y, i64 0, i64 %idxprom
  store i32 2, i32* %arrayidx, align 4
  %1 = load i32, i32* %n.addr, align 4
  %idxprom1 = sext i32 %1 to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], [10 x i32]* %y, i64 0, i64 %idxprom1
  store i32* %arrayidx2, i32** @yptr, align 8
  %2 = load i32, i32* %n.addr, align 4
  %conv = sext i32 %2 to i64
  %3 = load i32, i32* %size.addr, align 4
  %conv3 = sext i32 %3 to i64

; Check that the LB, UB exprs for the array section are captured
; CHECK: store i64 %conv, i64* [[LB_ADDR:%[a-zA-Z._0-9]+]]
; CHECK: store i64 %conv3, i64* [[SIZE_ADDR:%[a-zA-Z._0-9]+]]
; Check that the captured pointers are sent in to the outlined function
; CHECK: call void {{.+}} @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 4, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64*, i64*, i32*, [10 x i32]*)* @foo.{{.*}} to void (i32*, i32*, ...)*), i64* [[SIZE_ADDR]], i64* [[LB_ADDR]], i32* %n.addr, [10 x i32]* %y)

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([10 x i32]* %y, i64 1, i64 %conv, i64 %conv3, i64 1), "QUAL.OMP.NUM_THREADS"(i32 4), "QUAL.OMP.SHARED"(i32* %n.addr), "QUAL.OMP.SHARED"(i32** @yptr) ]
  %5 = load i32, i32* %n.addr, align 4
  %idxprom4 = sext i32 %5 to i64
  %arrayidx5 = getelementptr inbounds [10 x i32], [10 x i32]* %y, i64 0, i64 %idxprom4
  %6 = load i32, i32* %arrayidx5, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %arrayidx5, align 4
  %call = call i32 @omp_get_thread_num()
  %7 = load i32, i32* %n.addr, align 4
  %idxprom6 = sext i32 %7 to i64
  %arrayidx7 = getelementptr inbounds [10 x i32], [10 x i32]* %y, i64 0, i64 %idxprom6
  %8 = load i32, i32* %arrayidx7, align 4
  %9 = load i32*, i32** @yptr, align 8
  %10 = load i32, i32* %n.addr, align 4
  %idxprom8 = sext i32 %10 to i64
  %arrayidx9 = getelementptr inbounds [10 x i32], [10 x i32]* %y, i64 0, i64 %idxprom8
  %call10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([38 x i8], [38 x i8]* @.str, i64 0, i64 0), i32 %call, i32 %8, i32* %9, i32* %arrayidx9)
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL"() ]
  %11 = load i32, i32* %n.addr, align 4
  %idxprom11 = sext i32 %11 to i64
  %arrayidx12 = getelementptr inbounds [10 x i32], [10 x i32]* %y, i64 0, i64 %idxprom11
  %12 = load i32, i32* %arrayidx12, align 4
  %call13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str.1, i64 0, i64 0), i32 %12, i32 6)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

declare dso_local i32 @omp_get_thread_num() #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  call void @foo(i32 2, i32 3)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
