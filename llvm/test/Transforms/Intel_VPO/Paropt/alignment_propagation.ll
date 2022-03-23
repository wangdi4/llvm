; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
;Test src:
;float AAA[100] __attribute__((aligned(64))) ;
;void foo() {
;  float BBB[100] __attribute__((aligned(32))) ;
;  #pragma omp parallel private(AAA, BBB)
;  {
;    float CCC[100] __attribute__((aligned(64))) ;
;    AAA[7] = BBB[8] + CCC[9];
;  }
;}
;
; CHECK: %CCC.priv = alloca [100 x float], align 64
; CHECK-NEXT: %BBB.priv = alloca [100 x float], align 32
; CHECK-NEXT: %AAA.priv = alloca [100 x float], align 64

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@AAA = common dso_local global [100 x float] zeroinitializer, align 64

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %BBB = alloca [100 x float], align 32
  %CCC = alloca [100 x float], align 64
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"([100 x float]* @AAA), "QUAL.OMP.PRIVATE"([100 x float]* %BBB), "QUAL.OMP.PRIVATE"([100 x float]* %CCC) ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* %BBB, i64 0, i64 8
  %1 = load float, float* %arrayidx, align 32
  %arrayidx1 = getelementptr inbounds [100 x float], [100 x float]* %CCC, i64 0, i64 9
  %2 = load float, float* %arrayidx1, align 4
  %add = fadd float %1, %2
  store float %add, float* getelementptr inbounds ([100 x float], [100 x float]* @AAA, i64 0, i64 7), align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
