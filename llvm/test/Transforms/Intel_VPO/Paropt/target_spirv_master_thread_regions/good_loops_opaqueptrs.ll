; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; If there's nothing to stop expansion in a loop, the whole loop can be in the
; same master thread region.
define spir_func void @good_loops() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %bottom_tested

; CHECK:         call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK:         br label %bottom_tested

bottom_tested:
  call spir_func void @foo()
  %cond0 = call spir_func i1 @baz()
  br i1 %cond0, label %bottom_tested, label %top_tested

top_tested:
  %cond1 = call spir_func i1 @baz()
  br i1 %cond1, label %top_tested.body, label %multi_exit.A

top_tested.body:
  call spir_func void @bar()
  br label %top_tested

multi_exit.A:
  %cond2 = call spir_func i1 @baz()
  br i1 %cond2, label %multi_exit.B, label %evil

multi_exit.B:
  %cond3 = call spir_func i1 @baz()
  br i1 %cond3, label %multi_exit.A, label %evil

evil:
  %cond4 = call spir_func i1 @baz()
  br i1 %cond4, label %evil.A, label %evil.B

evil.A:
  %cond5 = call spir_func i1 @baz()
  br i1 %cond5, label %evil.B, label %end

evil.B:
  %cond6 = call spir_func i1 @baz()
  br i1 %cond6, label %evil.A, label %end

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
declare spir_func void @bar() nounwind memory(none)
declare spir_func i1 @baz() nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"good_loops", i32 0, i32 0, i32 0, i32 0}
