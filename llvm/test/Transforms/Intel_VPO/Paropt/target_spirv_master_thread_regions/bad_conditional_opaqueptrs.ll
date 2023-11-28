; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Check handling of conditionals containing a parallel region. Since the branch
; needs to be done on all threads the branch condition cannot be calculated in a
; master thread region, and master thread regions on the other branch of the
; conditional should be confined to that branch.
define spir_func void @bad_conditional() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call spir_func void @bar()
  %cond = call spir_func i1 @baz()
  call spir_func void @foo()
  br label %pre

pre:
  call spir_func void @bar()
  br i1 %cond, label %then, label %else

; CHECK:         call spir_func void @bar()
; CHECK-NEXT:    %cond = call spir_func i1 @baz()
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE0:master.thread.code[0-9]*]], label %[[FALLTHRU0:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE0]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %pre

; CHECK:       pre:
; CHECK-NEXT:    call spir_func void @bar()
; CHECK-NEXT:    br label %[[FALLTHRU0]]

; CHECK:       [[FALLTHRU0]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %cond, label %then, label %else

then:
  %parallel = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel) [ "DIR.OMP.END.PARALLEL"() ]
  br label %end

else:
  call spir_func void @bar()
  call spir_func void @foo()
  call spir_func void @bar()
  br label %end

; CHECK:       else:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE1:master.thread.code[0-9]*]], label %[[FALLTHRU1:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE1]]:
; CHECK-NEXT:    call spir_func void @bar()
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    call spir_func void @bar()
; CHECK-NEXT:    br label %[[FALLTHRU1]]

; CHECK:       [[FALLTHRU1]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %end

end:
  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func void @bar() nounwind memory(none)
declare spir_func i1 @baz() nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"bad_conditional", i32 0, i32 0, i32 0, i32 0}
