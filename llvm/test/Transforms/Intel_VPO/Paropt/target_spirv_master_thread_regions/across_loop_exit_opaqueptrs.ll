; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Regions should not extend across loop exits if they can't contain the whole
; loop either.
define spir_func void @across_loop_exit() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %bottom_tested

bottom_tested:
  %parallel0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %bottom_tested.B

bottom_tested.B:
  call spir_func void @foo()
  %cond0 = call spir_func i1 @baz()
  br i1 %cond0, label %bottom_tested, label %bottom_tested.end

; CHECK:       bottom_tested.B:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE0:master.thread.code[0-9]*]], label %[[FALLTHRU0:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE0]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU0]]

; CHECK:       [[FALLTHRU0]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    %cond0 = call spir_func i1 @baz()
; CHECK-NEXT:    br i1 %cond0, label %bottom_tested, label %bottom_tested.end

bottom_tested.end:
  call spir_func void @foo()
  br label %top_tested

; CHECK:       bottom_tested.end:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE1:master.thread.code[0-9]*]], label %[[FALLTHRU1:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE1]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK:         br label %[[FALLTHRU1]]

; CHECK:       [[FALLTHRU1]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii

top_tested:
  call spir_func void @foo()
  %cond1 = call spir_func i1 @baz()
  br i1 %cond1, label %top_tested.B, label %top_tested.end

; CHECK:       top_tested:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE2:master.thread.code[0-9]*]], label %[[FALLTHRU2:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE2]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU2]]

; CHECK:       [[FALLTHRU2]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    %cond1 = call spir_func i1 @baz()
; CHECK-NEXT:    br i1 %cond1, label %top_tested.B, label %top_tested.end

top_tested.B:
  call spir_func void @foo()
  %parallel1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %top_tested

; CHECK:       top_tested.B:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE3:master.thread.code[0-9]*]], label %[[FALLTHRU3:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE3]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK:         br label %[[FALLTHRU3]]

; CHECK:       [[FALLTHRU3]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii

top_tested.end:
  call spir_func void @foo()

; CHECK:       top_tested.end:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE4:master.thread.code[0-9]*]], label %[[FALLTHRU4:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE4]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK:         br label %[[FALLTHRU4]]

; CHECK:       [[FALLTHRU4]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func i1 @baz() nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"across_loop_exit", i32 0, i32 0, i32 0, i32 0}
