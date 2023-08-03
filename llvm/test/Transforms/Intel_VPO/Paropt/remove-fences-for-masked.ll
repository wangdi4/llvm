; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='vpo-cfg-restructuring,vpo-paropt-prepare' -S %s | FileCheck %s

; Test src:
;
; #include <omp.h>
; int main() {
; #pragma omp target parallel
; #pragma omp masked
;  { }
; }

; Check that paropt-prepare removes compiler generated fences for WRNMasked
; CHECK-NOT: fence acquire
; CHECK-NOT: fence release

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected i32 @main() {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASKED"() ]

  fence acquire
  fence release
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.MASKED"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 43, i32 -1929561971, !"_Z4main", i32 3, i32 0, i32 0, i32 0}
