; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; This loop has basic blocks that can't be reached from the new expanded region
; start/end without going through known enclosed basic blocks; region expansion
; should still be able to handle this.
define spir_func void @tricky_reachability() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %loop

; CHECK:         call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK:         br label %loop

loop:
  %cond0 = call spir_func i1 @baz()
  br i1 %cond0, label %if, label %end

if:
  %cond1 = call spir_func i1 @baz()
  br i1 %cond1, label %then, label %else

then:
  call spir_func void @foo()
  br label %latch

else:
  call spir_func void @foo()
  br label %latch

latch:
  br label %loop

end:

; CHECK:       end:
; CHECK:         br label %[[FALLTHRU]]

; CHECK:       [[FALLTHRU]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func i1 @baz() nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"tricky_reachability", i32 0, i32 0, i32 0, i32 0}
