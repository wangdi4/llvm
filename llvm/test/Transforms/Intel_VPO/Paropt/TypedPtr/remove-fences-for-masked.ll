; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt-prepare' -S <%s | FileCheck %s

; SRC:

; #include <omp.h>
; int main() {
; #pragma omp target parallel
; #pragma omp masked
;  { }
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.PARALLEL.4

DIR.OMP.PARALLEL.4:                               ; preds = %DIR.OMP.TARGET.3
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.PARALLEL.5

DIR.OMP.PARALLEL.5:                               ; preds = %DIR.OMP.PARALLEL.4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASKED"() ]
  br label %DIR.OMP.MASKED.6

; Check that paropt-prepare removes compiler generated fences for WRNMasked
; CHECK-NOT: fence acquire
; CHECK-NOT: fence release

DIR.OMP.MASKED.6:                                 ; preds = %DIR.OMP.PARALLEL.5
  fence acquire
  fence release
  br label %DIR.OMP.END.MASKED.7

DIR.OMP.END.MASKED.7:                             ; preds = %DIR.OMP.MASKED.6
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.MASKED"() ]
  br label %DIR.OMP.END.MASKED.8

DIR.OMP.END.MASKED.8:                             ; preds = %DIR.OMP.END.MASKED.7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.9

DIR.OMP.END.PARALLEL.9:                           ; preds = %DIR.OMP.END.MASKED.8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.10

DIR.OMP.END.TARGET.10:                            ; preds = %DIR.OMP.END.PARALLEL.9
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone uwtable "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2055, i32 16377346, !"_Z4main", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"uwtable", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 13.0.0"}
