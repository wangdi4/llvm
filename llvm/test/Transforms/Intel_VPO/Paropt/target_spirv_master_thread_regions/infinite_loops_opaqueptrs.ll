; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Regions should not extend out of infinite loops.
define spir_func void @infinite_loops() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  %which = call spir_func i32 @iaz()
  switch i32 %which, label %small.ph [ i32 1, label %big.ph i32 2, label %evil.ph ]

small.ph:
  br label %small

small:
  call spir_func void @foo()
  br label %small

; CHECK:       small:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE0:master.thread.code[0-9]*]], label %[[FALLTHRU0:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE0]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU0]]

; CHECK:       [[FALLTHRU0]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %small

big.ph:
  br label %big.A

big.A:
  call spir_func void @foo()
  br label %big.B

; CHECK:       big.A:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE1:master.thread.code[0-9]*]], label %[[FALLTHRU1:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE1]]:
; CHECK-NEXT:    call spir_func void @foo()

big.B:
  call spir_func void @foo()
  br label %big.A

; CHECK:       big.B:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU1]]

; CHECK:       [[FALLTHRU1]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %big.A

evil.ph:
  %cond = call spir_func i1 @baz()
  br i1 %cond, label %evil.A, label %evil.B

evil.A:
  call spir_func void @foo()
  br label %evil.B

; CHECK:       evil.A:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE2:master.thread.code[0-9]*]], label %[[FALLTHRU2:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE2]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU2]]

; CHECK:       [[FALLTHRU2]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %evil.B

evil.B:
  call spir_func void @foo()
  br label %evil.A

; CHECK:       evil.B:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE3:master.thread.code[0-9]*]], label %[[FALLTHRU3:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE3]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU3]]

; CHECK:       [[FALLTHRU3]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %evil.A

end:
  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func i1 @baz() nounwind memory(none)
declare spir_func i32 @iaz() nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"infinite_loops", i32 0, i32 0, i32 0, i32 0}
