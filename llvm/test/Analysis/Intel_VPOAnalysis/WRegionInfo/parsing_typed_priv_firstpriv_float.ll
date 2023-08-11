; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This code tests TYPED clause
; The test is passed if PRIVATE:TYPED and FIRSTPRIVATE:TYPED clauses are parsed correctly

; Test src:
;
; #include <omp.h>
; int main() {
;   float x = 5.0;
;   float y[10];
; #pragma omp parallel private(x) firstprivate(y)
;   { y[5] = x; }
; }

; CHECK: PRIVATE clause (size=1): TYPED({{.*}}, TYPE: float, NUM_ELEMENTS: i32 1)
; CHECK: FIRSTPRIVATE clause (size=1): TYPED({{.*}}, TYPE: float, NUM_ELEMENTS: i64 10)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %x = alloca float, align 4
  %y = alloca [10 x float], align 16
  store float 5.000000e+00, ptr %x, align 4
  %array.begin = getelementptr inbounds [10 x float], ptr %y, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x, float 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, float 0.000000e+00, i64 10) ]
  %1 = load float, ptr %x, align 4
  %arrayidx = getelementptr inbounds [10 x float], ptr %y, i64 0, i64 5
  store float %1, ptr %arrayidx, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
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
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
