; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Checks that in the SPIR64 target compilation we replace printf()
; with the OCL builtin version _Z18__spirv_ocl_printfPU3AS2cz()
;
; #include <stdio.h>
; int main() {
;   #pragma omp target
;   {
;      int var = 123;
;      printf("\n\nHello %d %f %s \n\n\n", var, 456.0, "finally!");
;   }
; }

; Paropt creates this prototype for OCL printf
; CHECK: [[STR:@.str.*]] = private target_declare addrspace(2) constant [21 x i8] c"\0A\0AHello %d %f %s \0A\0A\0A\00"
; CHECK: declare spir_func i32 @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) noundef, ...)

; Make sure the original printf is removed
; CHECK-NOT: call {{.*}} (ptr addrspace(4), ...) @printf

; Calling the OCL printf
; CHECK: call i32 (ptr addrspace(2), ...) @_Z18__spirv_ocl_printfPU3AS2cz(ptr addrspace(2) [[STR]], i32 %{{.*}}, double 4.560000e+02, ptr addrspace(4) addrspacecast (ptr addrspace(1) @.str.1 to ptr addrspace(4)))

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [21 x i8] c"\0A\0AHello %d %f %s \0A\0A\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [9 x i8] c"finally!\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define protected i32 @main() {
entry:
  %retval = alloca i32, align 4
  %var = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %var.ascast = addrspacecast ptr %var to ptr addrspace(4)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %var.ascast, i32 0, i32 1) ]

  store i32 123, ptr addrspace(4) %var.ascast, align 4
  %1 = load i32, ptr addrspace(4) %var.ascast, align 4
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i32 noundef %1, double noundef 4.560000e+02, ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str.1 to ptr addrspace(4)))

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 47770958, !"_Z4main", i32 3, i32 0, i32 0}
