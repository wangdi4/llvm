; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; SRC:
; #include <omp.h>
; int main() {
; #pragma omp masked filter(3)
;  { }
; #pragma omp master
;  { }
; }
;
;Front end IR was hand modified because MASKED and FILTER are not yet supported
;CHECK: %{{.*}} = call i32 @__kmpc_masked(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 3)
;CHECK: call void @__kmpc_end_masked(%struct.ident_t* @{{.*}}, i32 %{{.*}})
;check that in case of master construct we generate kmpc_masked with filter = 0.
;CHECK: %{{.*}} = call i32 @__kmpc_masked(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 0)
;CHECK: call void @__kmpc_end_masked(%struct.ident_t* @{{.*}}, i32 %{{.*}})

; ModuleID = '/tmp/icxsjWoeu.bc'
source_filename = "test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASKED"(), "QUAL.OMP.FILTER"(i32 3) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.MASKED"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"clang version 9.0.0"}
