; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; Test src (only foo function is used in this lit test):
;
; Note: The test has a different step in the body of the loop than
; the one specified on the clause.
;
; #include <stdio.h>
;
; int step = -1;
; short y = 0;
; int x = 0;
;
; void foo() {
; #pragma omp parallel for linear(y : step) firstprivate(x) lastprivate(x)
;   for (unsigned int i = 0; i < 10; i++) {
;     printf("y[%d] = %d\n", i, y);
;     y += 100;
;     x = i;
;   }
; }
;
; int main() {
;
;   foo();
;
;   printf("Final: x = %d, y = %d\n", x, y);
;   if (x == 9 && y == 91) {
;       printf("PASSED\n");
;     return 0;
;   }
;
;   printf("FAILED\n");
;   return 1;
; }

; ModuleID = 'linear_fp_lp.c'
source_filename = "linear_fp_lp.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@step = dso_local global i32 -1, align 4
@y = dso_local global i16 0, align 2
@x = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [12 x i8] c"y[%d] = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4

  %0 = load i32, i32* @step, align 4
; CHECK: [[STEP_VAL:%[a-zA-Z._0-9]+]] = load i32, i32* @step, align 4
; Check that the value of step is stored to a pointer, and then sent in
; though the entry.
; CHECK: store i32 [[STEP_VAL]], i32* [[STEP_VAL_CAPTURED:%[a-zA-Z._0-9]+]]
; CHECK: void @foo{{[a-zA-Z._0-9]*}}(i32* %{{.*}}, i32* %{{.*}}, i32* [[STEP_VAL_CAPTURED]], i32* %.omp.lb, i32* %.omp.ub)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.LINEAR"(i16* @y, i32 %0), "QUAL.OMP.FIRSTPRIVATE"(i32* @x), "QUAL.OMP.LASTPRIVATE"(i32* @x), "QUAL.OMP.SHARED"(i16* @y), "QUAL.OMP.SHARED"(i32* @step), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; Initial copy of linear var
; CHECK: [[STEP_VAL_INREGION:%[a-zA-Z._0-9]+]] = load i32, i32* [[STEP_VAL_CAPTURED]]
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load i16, i16* @y
; CHECK: store i16 [[LOAD1]], i16* [[LINEAR_INIT:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier{{.*}}
; CHECK: call void @__kmpc_for_static_init_4u(%__struct.ident_t* @{{[^, ]+}}, i32 %{{[^, ]+}}, i32 34, i32* %{{[^, ]+}}, i32* [[CHUNK_LB_PTR:%[^, ]+]]{{.*}}

; Initialization of linear var per chunk
; CHECK: [[CHUNK_LB:%[^ ]+]] = load i32, i32* [[CHUNK_LB_PTR]]
; CHECK: [[LOAD2:%[a-zA-Z._0-9]+]] = load i16, i16* [[LINEAR_INIT]]
; CHECK: [[MUL:%[a-zA-Z._0-9]+]] = mul i32 [[CHUNK_LB]], [[STEP_VAL_INREGION]]
; CHECK: [[CAST1:%[a-zA-Z._0-9]+]] = sext i16 [[LOAD2]] to i32
; CHECK: [[ADD:%[a-zA-Z._0-9]+]] = add i32 [[CAST1]], [[MUL]]
; CHECK: [[CAST2:%[a-zA-Z._0-9]+]] = trunc i32 [[ADD]] to i16
; CHECK: store i16 [[CAST2]], i16* [[LINEAR_LOCAL:%[a-zA-Z._0-9]+]]

; Make sure that only one barrier is emitted even though the test has
; fp+lp as well as linear vars.
; CHECK-NOT: call void @__kmpc_barrier{{.*}}

; Final copyout for linear var
; CHECK-DAG: store i16 [[LOAD3:%[a-zA-Z._0-9]+]], i16* @y
; CHECK-DAG: [[LOAD3]] = load i16, i16* [[LINEAR_LOCAL]]

  %2 = load i32, i32* %.omp.lb, align 4
  store i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp ule i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul i32 %5, 1
  %add = add i32 0, %mul
  store i32 %add, i32* %i, align 4
  %6 = load i32, i32* %i, align 4
  %7 = load i16, i16* @y, align 2
  %conv = sext i16 %7 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i32 0, i32 0), i32 %6, i32 %conv)
  %8 = load i16, i16* @y, align 2
  %conv1 = sext i16 %8 to i32
  %add2 = add nsw i32 %conv1, 100
  %conv3 = trunc i32 %add2 to i16
  store i16 %conv3, i16* @y, align 2
  %9 = load i32, i32* %i, align 4
  store i32 %9, i32* @x, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32* %.omp.iv, align 4
  %add4 = add i32 %10, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
