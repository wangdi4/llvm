; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Test src (pseudo):
;
; void f2(int*);
; void foo() {
;   int i;
;   #pragma omp target private(i)
;   {
;   #pragma omp parallel for shared(i)
;     for (int i = 0; i < 1; i++) {
;       i = 111;
;       f2(&i);
;     }
;   f2(&i);
;   }
; }

; The test contains hand-modified IR to simulate a case when a single-iteration
; "parallel for" inside a "target" construct is optimized away, leaving behind
; the body of the iteration.

; But despite that, the "parallel.loop" directive need to be retained until
; for correct codegen in Paropt, to ensure that:
; 1. The instructions inside the "parallel.loop" are not guarded by
;    master-thread guards.
; 2. An implicit barrier is added after the "parallel.loop".

; Check that the code inside the "parallel.loop" region is not guarded by master-thread guard.
; CHECK-NOT: br i1 %is.master.thread
; CHECK:           store i32 111, ptr addrspace(1) @i.ascast.priv.__global, align 4
; CHECK:           call spir_func void @f2(ptr addrspace(4) addrspacecast (ptr addrspace(1) @i.ascast.priv.__global to ptr addrspace(4)))

; Check for the implicit barrier at the end of the "parallel.loop" region.
; CHECK:           call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)

; Check for the master-thread-check around the call to "f2()" after the parallel loop.
; CHECK:           call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK:           br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru
; CHECK:         master.thread.code:
; CHECK:           call spir_func void @f2(ptr addrspace(4) addrspacecast (ptr addrspace(1) @i.ascast.priv.__global to ptr addrspace(4)))
; CHECK:           call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)

declare spir_func void @f2(ptr addrspace(4) readonly ) nounwind

define protected spir_func void @foo() {
entry:
  %i = alloca i32, align 4
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %i1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

  %i3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) undef, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) undef, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) undef, i32 0),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %entry
  store i32 111, ptr addrspace(4) %i.ascast, align 4
  call spir_func void @f2(ptr addrspace(4) %i.ascast)
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %i3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call spir_func void @f2(ptr addrspace(4) %i.ascast)

  call void @llvm.directive.region.exit(token %i1) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 53, i32 -1916375442, !"foo", i32 2, i32 0, i32 0}
