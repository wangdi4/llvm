; REQUIRES: asserts
; RUN: opt -opaque-pointers=1 -enable-new-pm=0 -vpo-paropt-target-capture-non-pointers-using-map-to -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=1 -switch-to-offload -vpo-paropt-target-capture-non-pointers-using-map-to -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int main() {
;   int n = 2;
;   int x[n];
;
; #pragma omp target firstprivate(x)
;     printf("%p\n", x);
; }

; The test is a version of target_fp_map_vla_tgt.ll, but uses map(to)
; instead of firstprivate for capturing the VLA's size.

; CHECK:     collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %n.val i64 %n.val1 '
; CHECK:     captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr addrspace(4) [[SIZE_ADDR:%n.val.addr.*]]'
; CHECK:     captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr addrspace(4) [[SIZE_ADDR1:%n.val1.addr.*]]'

; Check that the kernel function has arguments for the VLA and the captured VLA size.
; CHECK:     define {{.*}} void @__omp_offloading{{.*}}main{{.*}}(ptr addrspace(1) %vla.ascast, ptr addrspace(1) noalias [[SIZE_ADDR]], ptr addrspace(1) noalias [[SIZE_ADDR1]])

; Check that no extra local copy is made for the VLA inside the kernel, and the argument passed-in is used directly.
; CHECK-NOT:   %{{.*}} = alloca i32, {{.*}}
; CHECK:       [[VLA_CAST:%.+]] = addrspacecast ptr addrspace(1) %vla.ascast to ptr addrspace(4)
; CHECK:       call {{.*}} @_Z18__spirv_ocl_printfPU3AS2cz({{.*}}, ptr addrspace(4) [[VLA_CAST]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%p\0A\00", align 1

define protected i32 @main() {
entry:
  %n = alloca i64, align 8
  %n.ascast = addrspacecast ptr %n to ptr addrspace(4)
  store i64 2, ptr addrspace(4) %n.ascast, align 8

  %n.val = load i64, ptr addrspace(4) %n.ascast, align 8
  %n.val1 = load i64, ptr addrspace(4) %n.ascast, align 8

  %vla = alloca i32, i64 %n.val1, align 4
  %vla.ascast = addrspacecast ptr %vla to ptr addrspace(4)

  %map.size = mul i64 %n.val, 4
  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %vla.ascast, i32 0, i64 %n.val),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %vla.ascast, ptr addrspace(4) %vla.ascast, i64 %map.size, i64 161, ptr null, ptr null) ]

  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), ptr addrspace(4) noundef %vla.ascast)

  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 135942768, !"_Z4main", i32 6, i32 0, i32 0}
