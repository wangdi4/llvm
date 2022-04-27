; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; int *yptr;
;
; __attribute__((noinline)) void foo(int n) {
;   int y[n];
;   yptr = (int*) y;
;   y[2] = 2;
;
; #pragma omp parallel firstprivate(y) num_threads(1)
;   {
;     printf("y[2] = %d, yptr = %p, y = %p\n", y[2], (int*) yptr, (int*) y);
;     y[2] = 0;
;   }
;   printf("y[2] = %d (expected = %d)\n", y[2], 2);
; }
;
; int main() {
;   foo(10);
; }
;
; ModuleID = 'vla_par.c'
source_filename = "vla_par.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@yptr = common dso_local global i32* null, align 8
@.str = private unnamed_addr constant [30 x i8] c"y[2] = %d, yptr = %p, y = %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [27 x i8] c"y[2] = %d (expected = %d)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  store i32* %vla, i32** @yptr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %vla, i64 2
  store i32 2, i32* %arrayidx, align 8
  store i64 %1, i64* %omp.vla.tmp, align 8


; CHECK: %vla = alloca i32, i64 [[VLA_SIZE_VAL:%[a-zA-Z._0-9]+]]
; CHECK-DAG: call void {{.+}} @__kmpc_fork_call(%struct.ident_t* {{.+}}, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64*, i64*, i32*)* @foo.{{.*}} to void (i32*, i32*, ...)*), i64* [[VLA_SIZE_CAPTURED:%.+]], i64* %omp.vla.tmp, i32* %vla)
; Check that VLA_SIZE_VAL was stored to VLA_SIZE_CAPTURED
; CHECK-DAG: store i64 [[VLA_SIZE_VAL]], i64* [[VLA_SIZE_CAPTURED]]
; CHECK that a load from VLA_SIZE_CAPTURED was used in another alloca (which is for the private copy of %vla)
; CHECK: [[VLA_SIZE_VAL2:%.+]] = load i64, i64* [[VLA_SIZE_CAPTURED]]
; CHECK: [[VLA_PRIVATE:%.+]] = alloca i32, i64 [[VLA_SIZE_VAL2]]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %vla), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.SHARED"(i32** @yptr), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp) ]
  %4 = load i64, i64* %omp.vla.tmp, align 8
  %arrayidx1 = getelementptr inbounds i32, i32* %vla, i64 2
  %5 = load i32, i32* %arrayidx1, align 8
  %6 = load i32*, i32** @yptr, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str, i64 0, i64 0), i32 %5, i32* %6, i32* %vla)
  %arrayidx2 = getelementptr inbounds i32, i32* %vla, i64 2
  store i32 0, i32* %arrayidx2, align 8
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  %arrayidx3 = getelementptr inbounds i32, i32* %vla, i64 2
  %7 = load i32, i32* %arrayidx3, align 8
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([27 x i8], [27 x i8]* @.str.1, i64 0, i64 0), i32 %7, i32 2)
  %8 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %8)
  ret void
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  call void @foo(i32 10)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
