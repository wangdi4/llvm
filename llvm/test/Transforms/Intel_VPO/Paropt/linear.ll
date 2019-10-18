; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; Test src (only foo function is used in this lit test):
;
; #include <stdio.h>
;
; short y = 0;
;
; void foo() {
; #pragma omp parallel for linear(y : 2)
;   for (long i = 0; i < 10; i++) {
;     printf("y[%ld] = %d\n", i, y);
;     y += 2;
;   }
; }
;
; int main() {
;
;   foo();
;
;   printf("Final: y = %d\n", y);
;   if (y == 20) {
;       printf("PASSED\n");
;     return 0;
;   }
;
;   printf("FAILED\n");
;   return 1;
; }

; ModuleID = 'linear.c'
source_filename = "linear.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = dso_local global i16 0, align 2
@.str = private unnamed_addr constant [13 x i8] c"y[%ld] = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %.omp.iv = alloca i64, align 8
  %tmp = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i64, align 8
  store i64 0, i64* %.omp.lb, align 8
  store i64 9, i64* %.omp.ub, align 8
  store i64 1, i64* %.omp.stride, align 8
  store i32 0, i32* %.omp.is_last, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.LINEAR"(i16* @y, i32 2), "QUAL.OMP.SHARED"(i16* @y), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %i) ]

; Initial copy of linear var
; CHECK: [[LOAD1:%[a-zA-Z._0-9]+]] = load i16, i16* @y
; CHECK: store i16 [[LOAD1]], i16* [[LINEAR_INIT:%[a-zA-Z._0-9]+]]
; CHECK: call void @__kmpc_barrier{{.*}}
; CHECK: call void @__kmpc_for_static_init_8(%__struct.ident_t* @{{[^, ]+}}, i32 %{{[^, ]+}}, i32 34, i32* %{{[^, ]+}}, i64* [[CHUNK_LB_PTR:%[^, ]+]]{{.*}}

; Initialization of linear var per chunk
; CHECK: [[CHUNK_LB:%[^ ]+]] = load i64, i64* [[CHUNK_LB_PTR]]
; CHECK: [[LOAD2:%[a-zA-Z._0-9]+]] = load i16, i16* [[LINEAR_INIT]]
; CHECK: [[MUL:%[a-zA-Z._0-9]+]] = mul i64 [[CHUNK_LB]], 2
; CHECK: [[CAST1:%[a-zA-Z._0-9]+]] = sext i16 [[LOAD2]] to i64
; CHECK: [[ADD:%[a-zA-Z._0-9]+]] = add i64 [[CAST1]], [[MUL]]
; CHECK: [[CAST2:%[a-zA-Z._0-9]+]] = trunc i64 [[ADD]] to i16
; CHECK: store i16 [[CAST2]], i16* [[LINEAR_LOCAL:%[a-zA-Z._0-9]+]]

; Final copyout for linear var
; CHECK-DAG: store i16 [[LOAD3:%[a-zA-Z._0-9]+]], i16* @y
; CHECK-DAG: [[LOAD3]] = load i16, i16* [[LINEAR_LOCAL]]

  %1 = load i64, i64* %.omp.lb, align 8
  store i64 %1, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i64, i64* %.omp.iv, align 8
  %3 = load i64, i64* %.omp.ub, align 8
  %cmp = icmp sle i64 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i64, i64* %.omp.iv, align 8
  %mul = mul nsw i64 %4, 1
  %add = add nsw i64 0, %mul
  store i64 %add, i64* %i, align 8
  %5 = load i64, i64* %i, align 8
  %6 = load i16, i16* @y, align 2
  %conv = sext i16 %6 to i32
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str, i32 0, i32 0), i64 %5, i32 %conv)
  %7 = load i16, i16* @y, align 2
  %conv1 = sext i16 %7 to i32
  %add2 = add nsw i32 %conv1, 2
  %conv3 = trunc i32 %add2 to i16
  store i16 %conv3, i16* @y, align 2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i64, i64* %.omp.iv, align 8
  %add4 = add nsw i64 %8, 1
  store i64 %add4, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
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
