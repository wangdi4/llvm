; INTEL_FEATURE_CSA
; RUN: opt -vpo-paropt -S %s
; REQUIRES: csa-registered-target
;
; Check that paropt does not assert on this module (see CMPLRLLVM-8058 for details).

target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3barii(i32 %X, i32 %Y) local_unnamed_addr #0 {
entry:
  %Y.addr = alloca i32, align 4
  store i32 %Y, i32* %Y.addr, align 4, !tbaa !2
  %cmp = icmp sgt i32 %X, 0
  br i1 %cmp, label %DIR.OMP.PARALLEL.LOOP.1, label %omp.precond.end

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* null), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(i32* null), "QUAL.OMP.PRIVATE"(i32* null), "QUAL.OMP.SHARED"(i32* %Y.addr) ]
  br label %DIR.OMP.PARALLEL.LOOP.119

DIR.OMP.PARALLEL.LOOP.119:                        ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %.pre = load i32, i32* %Y.addr, align 4, !tbaa !2, !alias.scope !6, !noalias !8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %for.cond.cleanup, %DIR.OMP.PARALLEL.LOOP.119
  %1 = phi i32 [ %2, %for.cond.cleanup ], [ %.pre, %DIR.OMP.PARALLEL.LOOP.119 ]
  %.omp.iv.0 = phi i32 [ %add7, %for.cond.cleanup ], [ 0, %DIR.OMP.PARALLEL.LOOP.119 ]
  %cmp616 = icmp sgt i32 %1, 0
  br i1 %cmp616, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body, %omp.inner.for.body
  %2 = phi i32 [ %1, %omp.inner.for.body ], [ %3, %for.body ]
  %add7 = add nuw nsw i32 %.omp.iv.0, 1
  %exitcond = icmp eq i32 %add7, %X
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body, !llvm.loop !11

for.body:                                         ; preds = %omp.inner.for.body, %for.body
  %storemerge17 = phi i32 [ %inc, %for.body ], [ 0, %omp.inner.for.body ]
  call void @_Z3fooii(i32 %.omp.iv.0, i32 %storemerge17) #1
  %inc = add nuw nsw i32 %storemerge17, 1
  %3 = load i32, i32* %Y.addr, align 4, !tbaa !2, !alias.scope !6, !noalias !8
  %cmp6 = icmp slt i32 %inc, %3
  br i1 %cmp6, label %for.body, label %for.cond.cleanup

omp.loop.exit:                                    ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3fooii(i32, i32) local_unnamed_addr #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 953588de7f0fed1e9a591c0eb292a6ef8ee4d5cb) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a64a72c3ce814cb12bfb5529c21fe136bdeccd18)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = distinct !{!6, !7, !"OMPAliasScope"}
!7 = distinct !{!7, !"OMPDomain"}
!8 = !{!9, !10}
!9 = distinct !{!9, !7, !"OMPAliasScope"}
!10 = distinct !{!10, !7, !"OMPAliasScope"}
!11 = distinct !{!11, !12}
!12 = distinct !{!"llvm.loop.optreport", !13}
!13 = distinct !{!"intel.loop.optreport", !14}
!14 = !{!"intel.optreport.remarks", !15}
!15 = !{!"intel.optreport.remark", !"CSA: OpenMP parallel loop will be pipelined"}
; end INTEL_FEATURE_CSA
