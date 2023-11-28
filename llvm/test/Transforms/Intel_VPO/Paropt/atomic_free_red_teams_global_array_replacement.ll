; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-atomic-free-reduction-slm=false <%s | FileCheck -check-prefix=NOSLM %s
; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring),vpo-paropt" -S -vpo-paropt-atomic-free-reduction-slm=false <%s | FileCheck -check-prefix=NOSLM %s

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S -vpo-paropt-atomic-free-reduction-slm=true -vpo-paropt-atomic-free-red-local-buf-size=0 <%s | FileCheck -check-prefix=SLM %s
; RUN: opt -switch-to-offload -passes="function(vpo-cfg-restructuring),vpo-paropt" -S -vpo-paropt-atomic-free-reduction-slm=true -vpo-paropt-atomic-free-red-local-buf-size=0 <%s | FileCheck -check-prefix=SLM %s

; Test src:
;
; int b[5] = {0, 0, 0, 0, 0};
;
; void foo() {
;   #pragma omp target teams reduction(+:b)
;     b[1] = 111;
; }

; The test IR is a reduced version of the IR after vpo-restore-operands pass,
; for the above test.

; Check for the replacement of the original "b" inside the teams region
; Without SLM - with a team id based offset into the reduction buffer.

; NOSLM: [[GROUP_ID:%.+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; NOSLM: %team.buf.baseptr = getelementptr inbounds [5 x i32], ptr addrspace(1) %red_buf, i64 [[GROUP_ID]], i32 0
; NOSLM: %i1 = getelementptr inbounds [5 x i32], ptr addrspace(1) %team.buf.baseptr, i64 0, i64 1
; NOSLM: store i32 111, ptr addrspace(1) %i1, align 4

; With SLM - with a private copy of "b" in addrspace 3.
; SLM: %i1 = getelementptr inbounds [5 x i32], ptr addrspace(3) @b.red.__local, i64 0, i64 1
; SLM: store i32 111, ptr addrspace(3) %i1, align 4

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@b = external addrspace(1) global [5 x i32]

define spir_func void @widget() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %tmp = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %bb2

bb2:                                              ; preds = %bb1
  %tmp3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(1) @b, i32 0, i64 5) ]

  %i = addrspacecast ptr addrspace(1) @b to ptr addrspace(4)
  %i1 = getelementptr inbounds [5 x i32], ptr addrspace(4) %i, i64 0, i64 1
  store i32 111, ptr addrspace(4) %i1, align 4
  br label %bb4

bb4:                                              ; preds = %bb2
  call void @llvm.directive.region.exit(token %tmp3) [ "DIR.OMP.END.TEAMS"() ]
  br label %bb5

bb5:                                              ; preds = %bb4
  call void @llvm.directive.region.exit(token %tmp) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 46789301, !"_Z3foo", i32 4, i32 0, i32 0, i32 0}
