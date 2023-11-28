; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Regions should not extend into or out of loop headers if they can't contain
; the whole loop and its preheader.
define spir_func void @across_loop_header() {
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  call spir_func void @foo()
  br label %L1

; CHECK:         call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE0:master.thread.code[0-9]*]], label %[[FALLTHRU0:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE0]]:
; CHECK:         call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU0]]

; CHECK:       [[FALLTHRU0]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br label %L1

L1:
  call spir_func void @foo()
  br label %L1.B

; CHECK:       L1:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE1:master.thread.code[0-9]*]], label %[[FALLTHRU1:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE1]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK:         br label %[[FALLTHRU1]]

; CHECK:       [[FALLTHRU1]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii

L1.B:
  %parallel0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel0) [ "DIR.OMP.END.PARALLEL"() ]
  %cond0 = call spir_func i1 @baz()
  br i1 %cond0, label %L1, label %L2.pre

L2.pre:
  %cond1 = call spir_func i1 @baz()
  br i1 %cond1, label %L2, label %L2.else

L2:
  call spir_func void @foo()
  %cond2 = call spir_func i1 @baz()
  br i1 %cond2, label %L2, label %end

; CHECK:       L2:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE2:master.thread.code[0-9]*]], label %[[FALLTHRU2:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE2]]:
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    br label %[[FALLTHRU2]]

; CHECK:       [[FALLTHRU2]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    %cond2 = call spir_func i1 @baz()
; CHECK-NEXT:    br i1 %cond2, label %L2, label %end

L2.else:
  %parallel1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel1) [ "DIR.OMP.END.PARALLEL"() ]
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

!0 = !{i32 0, i32 66306, i32 88872534, !"across_loop_header", i32 0, i32 0, i32 0, i32 0}
