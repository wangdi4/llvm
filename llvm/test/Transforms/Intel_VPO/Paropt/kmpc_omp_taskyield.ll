; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt-prepare' -S <%s | FileCheck %s

; SRC:
; #include <omp.h>
; int main() {
; #pragma omp taskyield
;  { }
; }

; CHECK: call void @__kmpc_omp_taskyield(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 0)

source_filename = "a.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKYIELD"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKYIELD"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
