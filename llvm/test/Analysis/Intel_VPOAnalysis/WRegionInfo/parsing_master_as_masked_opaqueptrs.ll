; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; SRC:

; #include <omp.h>
; int main() {
;  #pragma omp master
;  { }
; }

; Check in the debug log that MASTER is parsed as MASKED.
; CHECK: === updateWRGraph found: DIR.OMP.MASTER
; CHECk-NEXT: Created WRNMaskedNode <1>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  br label %DIR.OMP.MASTER.1

DIR.OMP.MASTER.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  br label %DIR.OMP.MASTER.2

DIR.OMP.MASTER.2:                                 ; preds = %DIR.OMP.MASTER.1
  fence acquire
  fence release
  br label %DIR.OMP.END.MASTER.3

DIR.OMP.END.MASTER.3:                             ; preds = %DIR.OMP.MASTER.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.MASTER"() ]
  br label %DIR.OMP.END.MASTER.4

DIR.OMP.END.MASTER.4:                             ; preds = %DIR.OMP.END.MASTER.3
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"clang version 13.0.0"}
