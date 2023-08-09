; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <omp.h>
; int main() {
;   int *a;
; #pragma omp masked filter(3)
;   {}
; #pragma omp master
;   {
;     *a = 10;
;   }
; }

; CHECK: %{{.*}} = call i32 @__kmpc_masked(ptr @{{.*}}, i32 %{{.*}}, i32 3)
; CHECK: call void @__kmpc_end_masked(ptr @{{.*}}, i32 %{{.*}})

; Check that in case of master construct we generate kmpc_masked with filter = 0.
; CHECK: %{{.*}} = call i32 @__kmpc_masked(ptr @{{.*}}, i32 %{{.*}}, i32 0)
; CHECK: call void @__kmpc_end_masked(ptr @{{.*}}, i32 %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca ptr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASKED"(),
    "QUAL.OMP.FILTER"(i32 3) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.MASKED"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %2 = load ptr, ptr %a, align 8
  store i32 10, ptr %2, align 4
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
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
