; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Make sure blocks aren't split in between phis.
define spir_func void @splitphi() {
entry:
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  %x = call spir_func i1 @baz()
  call spir_func void @foo()
  %cond = call spir_func i1 @baz()
  br i1 %cond, label %then, label %end

; CHECK:         %x = call spir_func i1 @baz()
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU]]

; CHECK:       [[FALLTHRU]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    %cond = call spir_func i1 @baz()
; CHECK-NEXT:    br i1 %cond, label %then, label %end

then:
  %y = call spir_func i1 @baz()
  br label %end

end:
  %a = phi i1 [ %x, %entry ], [ %y, %then ]
  %b = phi i1 [ true, %entry ], [ false, %then ]
  %parallel = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel) [ "DIR.OMP.END.PARALLEL"() ]
  call spir_func void @quux(i1 %b)

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func i1 @baz() nounwind memory(none)
declare spir_func void @quux(i1) nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"splitphi", i32 0, i32 0, i32 0, i32 0}
