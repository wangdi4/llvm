; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -S %s 2>&1 | FileCheck %s

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

; The test IR is a reduced version of the IR for the above src.

; CHECK:     collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %n.val '
; CHECK:     captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'i64 addrspace(4)* {{%n.val.addr.*}}'

; Check that the kernel function has arguments for the VLA and the captured VLA size.
; CHECK:     define {{.*}} void @__omp_offloading{{.*}}main{{.*}}(i32 addrspace(1)* %vla.ascast, i64 [[SIZE_ARG:%n.val.addr.*]])

; Check that no extra local copy is made for the VLA inside the kernel, and the argument passed-in is used directly.
; CHECK:       [[VLA_CAST:%.+]] = addrspacecast i32 addrspace(1)* %vla.ascast to i32 addrspace(4)*
; CHECK:       [[SIZE_FP:%n.val.addr.*fpriv]] = alloca i64, align 8
; CHECK:       store i64 [[SIZE_ARG]], i64* [[SIZE_FP]], align 8
; CHECK:       call {{.*}} @_Z18__spirv_ocl_printfPU3AS2cz({{.*}}, i32 addrspace(4)* [[VLA_CAST]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%p\0A\00", align 1

define protected i32 @main() {
entry:
  %n = alloca i64, align 8
  %n.ascast = addrspacecast i64* %n to i64 addrspace(4)*
  store i64 2, i64 addrspace(4)* %n.ascast, align 8

  %n.val = load i64, i64 addrspace(4)* %n.ascast, align 8

  %vla = alloca i32, i64 %n.val, align 4
  %vla.ascast = addrspacecast i32* %vla to i32 addrspace(4)*

  %dir = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %vla.ascast) ]

  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* noundef getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 addrspace(4)* noundef %vla.ascast)

  call void @llvm.directive.region.exit(token %dir) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func i32 @printf(i8 addrspace(4)* noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 135942768, !"_Z4main", i32 6, i32 0, i32 0}
