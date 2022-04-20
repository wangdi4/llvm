; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -instcombine -vpo-cfg-restructuring -vpo-paropt -S <%s 2>&1 | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring,instcombine,vpo-cfg-restructuring),vpo-paropt" -S <%s 2>&1 | FileCheck %s

; Test src:

;     integer b(1)
;     b = 0
;
;     !$omp target
;     !$omp end target
;
;     print *, b
;     end

; For now, this test is making sure that adding vpo-cfg-restructuring before
; instcombine prevents instcombine from inserting a fake liveout in the target
; region (thus causing a code-extractor assert). It may later be updated if we
; have a better fix for this issue.

; CHECK-NOT: CodeExtractor captured out-of-clause

target device_triples = "spir64"

define void @MAIN__() {
alloca_0:
  %argblock = alloca <{ i64, i8 addrspace(4)* }>, align 8
  %"ascastB$val" = addrspacecast <{ i64, i8 addrspace(4)* }>* %argblock to <{ i64, i8 addrspace(4)* }> addrspace(4)*
  br label %loop_test3

loop_test3:                                       ; preds = %alloca_0
  br label %DIR.OMP.TARGET.15.split

DIR.OMP.TARGET.15.split:                          ; preds = %loop_test3
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %DIR.OMP.TARGET.2.split

DIR.OMP.TARGET.2.split:                           ; preds = %DIR.OMP.TARGET.15.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %"(i8 addrspace(4)*)ascastB$val$" = bitcast <{ i64, i8 addrspace(4)* }> addrspace(4)* %"ascastB$val" to i8 addrspace(4)*
  %func_result = call i32 (i8 addrspace(4)*, ...) @for_write_seq_lis(i8 addrspace(4)* %"(i8 addrspace(4)*)ascastB$val$")
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare i32 @for_write_seq_lis(i8 addrspace(4)*, ...)

attributes #0 = { nounwind }

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66, i32 -688087849, !"MAIN__", i32 4, i32 0, i32 0}
