; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
;struct s1 {
;  double d[10];
;};
;void foo() {
;  struct s1 a;
;#pragma omp target firstprivate(a)
;  a.d[7] = 1234567;
;}

; Verify that the pass-as-kernel-argument is applied and
; that the store to a.d[7] is not guarded with a master-thread check.
; That would be incorrect, because the same store must be done
; in all threads.
; CHECK: define{{.*}}spir_kernel void @__omp_offloading_805_be2278__Z3foov_l6(ptr byval(<{ [10 x i64] }>)
; Master-thread check will generate get_local_id calls:
; CHECK-NOT: get_local_id

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.s1 = type { [10 x double] }

define hidden spir_func void @_Z3foov() {
entry:
  %a = alloca %struct.s1, align 8
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %a.ascast, %struct.s1 zeroinitializer, i32 1),
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %a.ascast, i64 80, i64 161, ptr null, ptr null) ]

  %arrayidx = getelementptr inbounds [10 x double], ptr addrspace(4) %a.ascast, i64 0, i64 7
  store double 0x4132D68700000000, ptr addrspace(4) %arrayidx, align 8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2053, i32 12460664, !"_Z3foov", i32 6, i32 0, i32 0}
