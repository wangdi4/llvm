; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

;Test src:
;
; float AAA[100] __attribute__((aligned(64)));
; void foo() {
;   float BBB[100] __attribute__((aligned(32)));
; #pragma omp parallel private(AAA, BBB)
;   {
;     float CCC[100] __attribute__((aligned(64)));
;     AAA[7] = BBB[8] + CCC[9];
;   }
; }

; CHECK: %CCC.priv = alloca [100 x float], align 64
; CHECK-NEXT: %BBB.priv = alloca [100 x float], align 32
; CHECK-NEXT: %AAA.priv = alloca [100 x float], align 64

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@AAA = dso_local global [100 x float] zeroinitializer, align 64

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %BBB = alloca [100 x float], align 32
  %CCC = alloca [100 x float], align 64
  %array.begin = getelementptr inbounds [100 x float], ptr %BBB, i32 0, i32 0
  %array.begin2 = getelementptr inbounds [100 x float], ptr %CCC, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @AAA, float 0.000000e+00, i64 100),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %BBB, float 0.000000e+00, i64 100),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %CCC, float 0.000000e+00, i64 100) ]
  %arrayidx = getelementptr inbounds [100 x float], ptr %BBB, i64 0, i64 8
  %1 = load float, ptr %arrayidx, align 32
  %arrayidx1 = getelementptr inbounds [100 x float], ptr %CCC, i64 0, i64 9
  %2 = load float, ptr %arrayidx1, align 4
  %add = fadd fast float %1, %2
  store float %add, ptr getelementptr inbounds ([100 x float], ptr @AAA, i64 0, i64 7), align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
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
