; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s

; INTEL_CUSTOMIZATION
; CMPLRLLVM-32892:
; Empty section creates a single dominating switch-case which was not
; accounted for in the DomTree.
; LoopInfo must be updated after section-loop and switch creation.

; Test src:
;       integer b,c
;
; !$omp sections
; !$omp end sections
;
; !$omp do collapse(2)
;       do b=1,2
;       do c=1,2
;       end do
;       end do
;       end
; end INTEL_CUSTOMIZATION

; CHECK: entry{{.*}}OMP.SECTIONS
; CHECK-DAG: header{{.*}}:
; CHECK-DAG: body{{.*}}:
; CHECK-DAG: case{{.*}}:
; CHECK-DAG: SECTIONS{{.*}}:
; CHECK-DAG: sw.succ{{.*}}:
; CHECK: exit{{.*}}OMP.END.SECTIONS
; CHECK: entry{{.*}}OMP.LOOP
; CHECK: exit{{.*}}OMP.END.LOOP

define void @foo() {
alloca_0:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTIONS"() ]
  br label %bb_new8

omp.pdo.cond5:                                    ; preds = %bb_new8, %do.epilog13
  br i1 false, label %omp.pdo.body6, label %omp.pdo.epilog7

omp.pdo.body6:                                    ; preds = %omp.pdo.cond5
  %do.norm.lb_fetch.13 = load i64, i64* null, align 1
  store i64 %do.norm.lb_fetch.13, i64* undef, align 1
  br label %do.cond11

do.cond11:                                        ; preds = %do.body12, %omp.pdo.body6
  br i1 false, label %do.body12, label %do.epilog13

do.body12:                                        ; preds = %do.cond11
  br label %do.cond11

do.epilog13:                                      ; preds = %do.cond11
  br label %omp.pdo.cond5

bb_new8:                                          ; preds = %alloca_0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.NORMALIZED.IV"(i64* undef, i64* undef), "QUAL.OMP.NORMALIZED.UB"(i64* undef, i64* undef) ]
  %omp.pdo.norm.lb_fetch.4 = load i64, i64* null, align 1
  store i64 %omp.pdo.norm.lb_fetch.4, i64* undef, align 1
  br label %omp.pdo.cond5

omp.pdo.epilog7:                                  ; preds = %omp.pdo.cond5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
