; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; INTEL_CUSTOMIZATION
; ! Fortran test source:
;
; program main
;
;   implicit none
;   external times_two
;
;   call cg_dev(times_two)
;
; end program
;
; subroutine cg_dev(matrix_mult)
;
;   implicit none
;   external :: matrix_mult
;
;   call times_two()
;
; end
;
; subroutine times_two()
;
;   !$omp target
;   !$omp end target
;
; end
; end INTEL_CUSTOMIZATION

; This LIT test validates that when a global value for device is eliminated, the corresponding global alias should be also removed.
;
; CHECK-NOT: void @foo()
; CHECK-NOT: @alias

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@alias = internal alias void (...), addrspacecast (ptr @foo to ptr addrspace(1))

define void @ham() {
bb:
  %alloca = alloca [8 x i64], align 8
  call void @zot(ptr addrspace(4) addrspacecast (ptr addrspace(1) @alias to ptr addrspace(4)))
  ret void
}

define internal void @zot(ptr addrspace(4) %arg) {
bb:
  call void (...) @foo.1(ptr addrspace(4) %arg)
  ret void
}

define void @foo() {
bb:
  %alloca = alloca [8 x i64], align 8
  br label %bb1

bb1:                                              ; preds = %bb
  %call = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call void @llvm.directive.region.exit(token %call) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @foo.1(...)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66306, i32 93073897, !"foo", i32 22, i32 0, i32 0}
