; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; C test for the allocate clause
; #include <omp.h>
; int main() {
;    float array[100];
;    float scalar = 15.0;
;    #pragma omp parallel private(array) firstprivate(scalar) \
;                allocate(omp_large_cap_mem_alloc:array) allocate(scalar)
;    {
;       array[50] = scalar;
;    }
;    return 0;
; }

; Without the allocate clauses, the IR to privatize "private(array) firstprivate(scalar)"
; would have been:
;   %scalar.fpriv = alloca float
;   %array.priv = alloca [100 x float]

; The IR to privatize "firstprivate(scalar) allocate(scalar)" is
;   %my.tid1 = load i32, i32* %tid, align 4
;   %default_allocator = call i64 @omp_get_default_allocator()
;   %0 = call i8* @__kmpc_aligned_alloc(i32 %my.tid1, i64 0, i64 4, i64 %default_allocator)
;
; CHECK: [[TID:%[^ ]+]] = load i32, i32* %tid, align 4
; CHECK: [[DEFALLOC:%[^ ]+]] = call i64 @omp_get_default_allocator()
; CHECK-NEXT:  call i8* @__kmpc_aligned_alloc(i32 [[TID]], i64 0, i64 4, i64 [[DEFALLOC]])

; The IR to privatize "private(array) allocate(omp_large_cap_mem_alloc:array)" is
; (where omp_large_cap_mem_alloc is an enum == 2)
;
;   %my.tid = load i32, i32* %tid, align 4
;   %1 = call i8* @__kmpc_aligned_alloc(i32 %my.tid, i64 0, i64 400, i64 2)
;   %array.priv = bitcast i8* %1 to [100 x float]*
;
; CHECK: [[TID2:%[^ ]+]] = load i32, i32* %tid, align 4
; CHECK-NEXT:  call i8* @__kmpc_aligned_alloc(i32 [[TID2]], i64 0, i64 400, i64 2)
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %array = alloca [100 x float], align 16
  %scalar = alloca float, align 4
  store i32 0, i32* %retval, align 4
  store float 1.500000e+01, float* %scalar, align 4
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"([100 x float]* %array), "QUAL.OMP.FIRSTPRIVATE"(float* %scalar), "QUAL.OMP.ALLOCATE"(i64 0, [100 x float]* %array, i32 2), "QUAL.OMP.ALLOCATE"(i64 0, float* %scalar) ]
  br label %DIR.OMP.PARALLEL.34

DIR.OMP.PARALLEL.34:                              ; preds = %DIR.OMP.PARALLEL.2
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.34
  %1 = load float, float* %scalar, align 4
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* %array, i64 0, i64 50
  store float %1, float* %arrayidx, align 8
  br label %DIR.OMP.END.PARALLEL.4.split

DIR.OMP.END.PARALLEL.4.split:                     ; preds = %DIR.OMP.PARALLEL.3
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.4.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.4
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
