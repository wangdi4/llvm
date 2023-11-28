; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S < %s | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Master thread regions cannot contain instructions with uses or thread-local
; stores with loads that the region can't be expanded to cover.
define spir_func void @load_use() {
  %X = alloca i64, align 8
  %target = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr %X, i64 0, i32 1) ]

  store i64 0, ptr %X
  call spir_func void @bar()
  call spir_func void @foo()
  call spir_func void @bar()
  %bad_use = load i64, ptr %X

; CHECK:         store i64 0, ptr %X.priv
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    br i1 %is.master.thread, label %[[CODE:master.thread.code[0-9]*]], label %[[FALLTHRU:master.thread.fallthru[0-9]*]]

; CHECK:       [[CODE]]:
; CHECK-NEXT:    call spir_func void @bar()
; CHECK-NEXT:    call spir_func void @foo()
; CHECK-NEXT:    call spir_func void @bar()
; CHECK-NEXT:    br label %[[FALLTHRU]]

; CHECK:       [[FALLTHRU]]:
; CHECK-NEXT:    call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT:    %bad_use = load i64, ptr %X.priv

  %parallel = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %parallel) [ "DIR.OMP.END.PARALLEL"() ]

  %private_load = load i64, ptr %X
  %bad_user = add i64 %bad_use, 0

  call void @llvm.directive.region.exit(token %target) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func void @foo()
declare spir_func void @bar() nounwind memory(none)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 88872534, !"load_use", i32 0, i32 0, i32 0, i32 0}
