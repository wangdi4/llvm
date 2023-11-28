; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Make sure regions aren't expanded to exit blocks where the predecessors aren't
; all included in the region.
define spir_func void @exitpreds() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  %cond0 = call spir_func i1 @baz()
  br i1 %cond0, label %A, label %B

A:
  %cond1 = call spir_func i1 @baz()
  br i1 %cond1, label %A.A, label %A.B

A.A:
  call spir_func void @foo()
  br label %end

; CHECK:       A.A:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU]]

; CHECK:       [[FALLTHRU]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %end

A.B:
  call spir_func void @foo()
  br label %end

; CHECK:       A.B:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU]]

; CHECK:       [[FALLTHRU]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %end

B:
  %parallel = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel) [ "DIR.OMP.END.PARALLEL"() ]
  br label %end

end:

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func i1 @baz() nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"exitpreds", i32 0, i32 0, i32 0, i32 0}
