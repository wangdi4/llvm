; Check that the remark that was added pre-HIR "survived" HIR phase.
;
;void foo(int *A, int N){
;  int i;
;  for(i = 0; i < N; ++i){
;    A[i] = i;
;  }
;}

; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-vec-dir-insert,hir-vplan-vec,hir-optreport-emitter,hir-cg" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
;
; OPTREPORT: LOOP BEGIN
; OPTREPORT:    remark #15339: pragma format ignored
; OPTREPORT:    remark #15300: LOOP WAS VECTORIZED
; OPTREPORT:    remark #15305: vectorization support: vector length {{.*}}
; OPTREPORT: LOOP END{{[[:space:]]}}
; OPTREPORT: LOOP BEGIN
; OPTREPORT: <Remainder loop for vectorization>
; OPTREPORT: LOOP END{{[[:space:]]}}
;
;Module Before HIR; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: norecurse nounwind writeonly
define dso_local void @foo(ptr nocapture %A, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp5 = icmp sgt i32 %N, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i32 %i.06
  store i32 %i.06, ptr %arrayidx, align 4, !tbaa !3
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !7

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a66ec9ba1f7107c4d02c79535caf41af08db3679)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = distinct !{!"intel.optreport", !10}
!10 = !{!"intel.optreport.remarks", !11}
; There is currently no remark created for unswitching, so we simply use
; an arbitrary remark here.
!11 = !{!"intel.optreport.remark", i32 15339}
