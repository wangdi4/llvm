; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s --check-prefixes CHECK,NEW
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-master-thread-region-expansion=false -S < %s | FileCheck %s --check-prefixes CHECK,OLD

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@x = internal addrspace(1) global i32 0

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1

; This case is one where it's believed that it's possible that the old guarding
; strategy could have a slight performance advantage; this test checks that the
; expected code is generated for both.
define spir_func void @parallel_load_as_prefetch() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  ; This store needs to be guarded.
  store i32 111, ptr addrspace(1) @x

; CHECK:         call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK:         store i32 111, ptr addrspace(1) @x

  ; This load can be guarded and is by the new strategy, but the old strategy
  ; executes it on all threads. Though there are no directly visible
  ; side-effects from this redundant execution, it's possible that this can act
  ; as a prefetch bringing @x into the cache for the non-master threads.
  %load.1 = load i32, ptr addrspace(1) @x

; OLD-NEXT:      br label %[[FALLTHRU]]

; OLD:         [[FALLTHRU]]:
; OLD-NEXT:      call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    %load.1 = load i32, ptr addrspace(1) @x

  ; This printf needs to be guarded.
  %printf.1 = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) getelementptr inbounds ([4 x i8], ptr addrspace(4) addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i64 0, i64 0), i32 %load.1)

; OLD-NEXT:      call spir_func void @_Z22__spirv_ControlBarrieriii
; OLD-NEXT:      br i1 %is.master.thread, label %[[CODE1:master.thread.code[0-9]*]], label %[[FALLTHRU1:master.thread.fallthru[0-9]*]]

; OLD:         [[CODE1]]:
; CHECK-NEXT:    call {{.*}}__spirv_ocl_printf{{.*}}, i32 %load.1)

  ; This load should not be guarded because it's used in the parallel region.
  %load.2 = load i32, ptr addrspace(1) @x

; OLD-NEXT:      br label %[[FALLTHRU1]]
; NEW-NEXT:      br label %[[FALLTHRU]]

; OLD:         [[FALLTHRU1]]:
; NEW:         [[FALLTHRU]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii

  %parallel = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  %printf.2 = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) getelementptr inbounds ([4 x i8], ptr addrspace(4) addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i64 0, i64 0), i32 %load.2)
  call void @llvm.directive.region.exit(token %parallel) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func i32 @printf(ptr addrspace(4), ...)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"parallel_load_as_prefetch", i32 0, i32 0, i32 0, i32 0}
