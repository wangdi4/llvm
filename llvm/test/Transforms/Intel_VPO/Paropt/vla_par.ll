; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
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

@yptr = common dso_local global ptr null, align 8
@.str = private unnamed_addr constant [30 x i8] c"y[2] = %d, yptr = %p, y = %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [27 x i8] c"y[2] = %d (expected = %d)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  store ptr %vla, ptr @yptr, align 8
  %arrayidx = getelementptr inbounds i32, ptr %vla, i64 2
  store i32 2, ptr %arrayidx, align 8
  store i64 %1, ptr %omp.vla.tmp, align 8


; CHECK: %vla = alloca i32, i64 [[VLA_SIZE_VAL:%[a-zA-Z._0-9]+]]
; CHECK-DAG: call void {{.+}} @__kmpc_fork_call(ptr {{.+}}, ptr @foo.{{.*}}, ptr [[VLA_SIZE_CAPTURED:%.+]], ptr %omp.vla.tmp, ptr %vla)
; Check that VLA_SIZE_VAL was stored to VLA_SIZE_CAPTURED
; CHECK-DAG: store i64 [[VLA_SIZE_VAL]], ptr [[VLA_SIZE_CAPTURED]]
; CHECK that a load from VLA_SIZE_CAPTURED was used in another alloca (which is for the private copy of %vla)
; CHECK: [[VLA_SIZE_VAL2:%.+]] = load i64, ptr [[VLA_SIZE_CAPTURED]]
; CHECK: [[VLA_PRIVATE:%.+]] = alloca i32, i64 [[VLA_SIZE_VAL2]]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %vla, i32 0, i64 %1),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.SHARED"(ptr @yptr),
    "QUAL.OMP.SHARED"(ptr %omp.vla.tmp) ]

  %4 = load i64, ptr %omp.vla.tmp, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %vla, i64 2
  %5 = load i32, ptr %arrayidx1, align 8
  %6 = load ptr, ptr @yptr, align 8
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %5, ptr %6, ptr %vla)
  %arrayidx2 = getelementptr inbounds i32, ptr %vla, i64 2
  store i32 0, ptr %arrayidx2, align 8
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]

  %arrayidx3 = getelementptr inbounds i32, ptr %vla, i64 2
  %7 = load i32, ptr %arrayidx3, align 8
  %call4 = call i32 (ptr, ...) @printf(ptr @.str.1, i32 %7, i32 2)
  %8 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %8)
  ret void
}

; Function Attrs: nounwind
declare ptr @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr, ...) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore(ptr) #1

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
