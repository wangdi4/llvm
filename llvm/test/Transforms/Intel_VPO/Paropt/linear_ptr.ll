; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; Test src (only foo function is used in this lit test):
;
; #include <stdio.h>
;
; int x[10];
; int *xptr = &x[0];
; long step = 1;
;
; void foo() {
; #pragma omp for linear(xptr : step)
;   for (short i = 0; i < 10; i++) {
;     printf("xptr[%d] = %p\n", i, xptr);
;     xptr++;
;   }
; }
;
; int main() {
;
;   printf("Initial = %p\n", xptr);
;   foo();
;   printf("Final = %p\n", xptr);
;   if (xptr == &x[10]) {
;     printf("PASSED\n");
;     return 0;
;   }
;
;   printf("FAILED\n");
;   return 1;
; }

; ModuleID = 'linear_ptr.c'
source_filename = "linear_ptr.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common dso_local global [10 x i32] zeroinitializer, align 16
@xptr = dso_local global i32* getelementptr inbounds ([10 x i32], [10 x i32]* @x, i32 0, i32 0), align 8
@step = dso_local global i64 1, align 8
@.str = private unnamed_addr constant [15 x i8] c"xptr[%d] = %p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i16, align 2
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i16, align 2
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4

  %0 = load i64, i64* @step, align 8
; CHECK: [[LOAD_STEP:%[a-zA-Z._0-9]+]] = load i64, i64* @step, align 8

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LINEAR"(i32** @xptr, i64 %0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i16* %i) ]

; Initial copy of linear var
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load i32*, i32** @xptr
; CHECK: store i32* [[LOAD1]], i32** [[LINEAR_INIT:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier{{.*}}
; CHECK: call void @__kmpc_for_static_init_4(%__struct.ident_t* @{{[^, ]+}}, i32 %{{[^, ]+}}, i32 34, i32* %{{[^, ]+}}, i32* [[CHUNK_LB_PTR:%[^, ]+]]{{.*}}

; Initialization of linear var per chunk
; CHECK: [[CHUNK_LB:%[^ ]+]] = load i32, i32* [[CHUNK_LB_PTR]]
; CHECK: [[LOAD2:%[a-zA-Z._0-9]+]] = load i32*, i32** [[LINEAR_INIT]]
; CHECK: [[CAST1:%[a-zA-Z._0-9]+]] = sext i32 [[CHUNK_LB]] to i64
; CHECK: [[MUL:%[a-zA-Z._0-9]+]] = mul i64 [[CAST1]], [[LOAD_STEP]]
; CHECK: [[GEP:%[a-zA-Z._0-9]+]] = getelementptr inbounds i32, i32* [[LOAD2]], i64 [[MUL]]
; CHECK: store i32* [[GEP]], i32** [[LINEAR_LOCAL:%[a-zA-Z._0-9]+]]

; Final copyout for linear var
; CHECK-DAG: store i32* [[LOAD3:%[a-zA-Z._0-9]+]], i32** @xptr
; CHECK-DAG: [[LOAD3]] = load i32*, i32** [[LINEAR_LOCAL]]

  %2 = load i32, i32* %.omp.lb, align 4
  store i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  %conv = trunc i32 %add to i16
  store i16 %conv, i16* %i, align 2
  %6 = load i16, i16* %i, align 2
  %conv1 = sext i16 %6 to i32
  %7 = load i32*, i32** @xptr, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i32 0, i32 0), i32 %conv1, i32* %7)
  %8 = load i32*, i32** @xptr, align 8
  %incdec.ptr = getelementptr inbounds i32, i32* %8, i32 1
  store i32* %incdec.ptr, i32** @xptr, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
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
