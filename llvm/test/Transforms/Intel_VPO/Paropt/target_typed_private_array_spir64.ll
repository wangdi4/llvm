; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -debug-only=vpo-paropt-utils -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -switch-to-offload -debug-only=vpo-paropt-utils -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s
;
; INTEL_CUSTOMIZATION
; // To produce the "QUAL.OMP.PRIVATE:TYPED" IR, compile with
; // icx -c -fiopenmp -fopenmp-targets=spir64 -Xclang -fopenmp-typed-clauses
; end INTEL_CUSTOMIZATION
; void foo(int nnn) {
;   double aaa[nnn];
;   #pragma omp target parallel private(aaa)
;     aaa[7] = 789.0;
; }
;
; Check that the privatized VLA array is in addrspace(4)
; CHECK:  getItemInfo: Local Element Info for 'ptr addrspace(4) %vla.ascast' (Typed):: Type: double, NumElements: i64 %1, AddrSpace: 4
;
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @foo(i32 noundef %nnn) {
entry:
  %nnn.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %nnn.addr.ascast = addrspacecast ptr %nnn.addr to ptr addrspace(4)
  %saved_stack.ascast = addrspacecast ptr %saved_stack to ptr addrspace(4)
  %__vla_expr0.ascast = addrspacecast ptr %__vla_expr0 to ptr addrspace(4)
  %omp.vla.tmp.ascast = addrspacecast ptr %omp.vla.tmp to ptr addrspace(4)
  store i32 %nnn, ptr addrspace(4) %nnn.addr.ascast, align 4
  %0 = load i32, ptr addrspace(4) %nnn.addr.ascast, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr addrspace(4) %saved_stack.ascast, align 8
  %vla = alloca double, i64 %1, align 8
  %vla.ascast = addrspacecast ptr %vla to ptr addrspace(4)
  store i64 %1, ptr addrspace(4) %__vla_expr0.ascast, align 8
  store i64 %1, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %vla.ascast, double 0.000000e+00, i64 %1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %omp.vla.tmp.ascast, i64 0, i32 1) ]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %vla.ascast, double 0.000000e+00, i64 %1),
    "QUAL.OMP.SHARED"(ptr addrspace(4) %omp.vla.tmp.ascast) ]

  %5 = load i64, ptr addrspace(4) %omp.vla.tmp.ascast, align 8
  %arrayidx = getelementptr inbounds double, ptr addrspace(4) %vla.ascast, i64 7
  store double 7.890000e+02, ptr addrspace(4) %arrayidx, align 8
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  %6 = load ptr, ptr addrspace(4) %saved_stack.ascast, align 8
  call void @llvm.stackrestore(ptr %6)
  ret void
}

declare ptr @llvm.stacksave()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.stackrestore(ptr)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 57, i32 -681255226, !"_Z3foo", i32 3, i32 0, i32 0}
