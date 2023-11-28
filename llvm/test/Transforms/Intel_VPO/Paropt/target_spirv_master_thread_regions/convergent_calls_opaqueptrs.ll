; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Master thread regions can't contain calls to functions that need convergent
; control flow, like barriers.
define spir_func void @convergent_calls() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
  call spir_func void @bar()
  call spir_func void @foo()
  call spir_func void @bar()
  call spir_func void @__kmpc_barrier()

; CHECK:         call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK-NEXT:    call spir_func void @bar()
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    call spir_func void @bar()
; CHECK-NEXT:    br label %[[FALLTHRU]]

; CHECK:       [[FALLTHRU]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    call spir_func void @__kmpc_barrier()

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func void @bar() nounwind memory(none)
declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32)
declare spir_func void @__kmpc_barrier()

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"convergent_calls", i32 0, i32 0, i32 0, i32 0}
