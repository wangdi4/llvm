; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test Src:
;
;#include<omp.h>
; void test() {
; #pragma omp critical(xaxis) hint(omp_lock_hint_contended)
;     ;
; }
;
; check that we are generating kmpc_critical_with_hint
;CHECK: call void @__kmpc_critical_with_hint(%struct.ident_t* @{{[^ ]+}}, i32 %{{[^ ]+}}, [8 x i32]* @{{[^ ]+}}, i32 2)

source_filename = "test_hint.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"(), "QUAL.OMP.NAME"([5 x i8] c"xaxis"), "QUAL.OMP.HINT"(i32 2) ]
  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.CRITICAL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"clang version 9.0.0"}
