; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <omp.h>
; int main() {
;   float array[100];
;   float scalar = 15.0;
; #pragma omp parallel private(array) firstprivate(scalar)                       \
;     allocate(align(16), allocator(omp_large_cap_mem_alloc)                     \
;              : array) allocate(align(8)                                        \
;                                : scalar)
;   { array[50] = scalar; }
;   return 0;
; }

; Without the allocate clauses, the IR to privatize "private(array) firstprivate(scalar)"
; would have been:
;   %scalar.fpriv = alloca float
;   %array.priv = alloca [100 x float]

; The IR to privatize "firstprivate(scalar) allocate(align(8):scalar)" is
;   %my.tid1 = load i32, ptr %tid, align 4
;   %default_allocator = call i64 @omp_get_default_allocator()
;   %scalar.fpriv = call ptr @__kmpc_aligned_alloc(i32 %my.tid1, i64 8, i64 4, i64 %default_allocator)
;
; CHECK: [[TID:%[^ ]+]] = load i32, ptr %tid, align 4
; CHECK: [[DEFALLOC:%[^ ]+]] = call i64 @omp_get_default_allocator()
; CHECK-NEXT:  call ptr @__kmpc_aligned_alloc(i32 [[TID]], i64 8, i64 4, i64 [[DEFALLOC]])

; The IR to privatize "private(array) allocate(align(16),allocator(omp_large_cap_mem_alloc):array)" is
; (where omp_large_cap_mem_alloc is an enum == 2)
;   %my.tid = load i32, ptr %tid, align 4
;   %array.priv = call ptr @__kmpc_aligned_alloc(i32 %my.tid, i64 16, i64 400, i64 2)
;
; CHECK: [[TID2:%[^ ]+]] = load i32, ptr %tid, align 4
; CHECK-NEXT:  call ptr @__kmpc_aligned_alloc(i32 [[TID2]], i64 16, i64 400, i64 2)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %array = alloca [100 x float], align 16
  %scalar = alloca float, align 4
  store i32 0, ptr %retval, align 4
  store float 1.500000e+01, ptr %scalar, align 4
  %array.begin = getelementptr inbounds [100 x float], ptr %array, i32 0, i32 0
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %array, float 0.000000e+00, i64 100),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %scalar, float 0.000000e+00, i32 1),
    "QUAL.OMP.ALLOCATE"(i64 16, ptr %array, i64 2),
    "QUAL.OMP.ALLOCATE"(i64 8, ptr %scalar) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  %1 = load float, ptr %scalar, align 4
  %arrayidx = getelementptr inbounds [100 x float], ptr %array, i64 0, i64 50
  store float %1, ptr %arrayidx, align 8
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.4
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
