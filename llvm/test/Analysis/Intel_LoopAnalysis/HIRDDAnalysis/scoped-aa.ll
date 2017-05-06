; Check that we do not form edges between (%a)[i1 + %i] and (%a)[i1 + %j] when scoped alias analysis is on.

; RUN: opt -hir-ssa-deconstruction %s | opt -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region | FileCheck %s

; CHECK-DAG: (%a)[i1 + %j] --> (%a)[i1 + %i] ANTI
; CHECK-DAG: (%a)[i1 + %i] --> (%a)[i1 + %j] FLOW

; RUN: opt -hir-ssa-deconstruction < %s | opt -scoped-noalias -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region | FileCheck --check-prefix=SCOPED-AA %s

; SCOPED-AA-NOT: (%a)[i1 + %j] --> (%a)[i1 + %i] ANTI
; SCOPED-AA-NOT: (%a)[i1 + %i] --> (%a)[i1 + %j] FLOW

; HIR-
; + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; |   %0 = (%a)[i1 + %i];
; |   %1 = (%a)[i1 + %j];
; |   %mul33.i = %mul  *  %1;
; |   %add34.i = %0  +  %mul33.i;
; |   (%a)[i1 + %i] = %add34.i;
; + END LOOP

define void @dgefa(float* nocapture %a, i64 %i, i64 %j, i32 %n, float %mul) {
entry:
  %arrayidx79 = getelementptr inbounds float, float* %a, i64 %i
  %arrayidx43 = getelementptr inbounds float, float* %a, i64 %j
  br label %for.body28.i

for.body28.i:                                     ; preds = %entry, %for.body28.i
  %indvars.iv.i196 = phi i64 [ %indvars.iv.next.i197, %for.body28.i ], [ 0, %entry ]
  %arrayidx30.i = getelementptr inbounds float, float* %arrayidx79, i64 %indvars.iv.i196
  %0 = load float, float* %arrayidx30.i, align 4, !tbaa !22, !alias.scope !62, !noalias !65
  %arrayidx32.i = getelementptr inbounds float, float* %arrayidx43, i64 %indvars.iv.i196
  %1 = load float, float* %arrayidx32.i, align 4, !tbaa !22, !alias.scope !65, !noalias !62
  %mul33.i = fmul float %mul, %1
  %add34.i = fadd float %0, %mul33.i
  store float %add34.i, float* %arrayidx30.i, align 4, !tbaa !22, !alias.scope !62, !noalias !65
  %indvars.iv.next.i197 = add nuw nsw i64 %indvars.iv.i196, 1
  %lftr.wideiv210 = trunc i64 %indvars.iv.next.i197 to i32
  %exitcond211 = icmp eq i32 %lftr.wideiv210, %n
  br i1 %exitcond211, label %exit, label %for.body28.i

exit:                                            ; preds = for.body28.i
  ret void
}

!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!13 = !{!"float", !3, i64 0}
!22 = !{!13, !13, i64 0}
!62 = !{!63}
!63 = distinct !{!63, !64, !"daxpy: %dy"}
!64 = distinct !{!64, !"daxpy"}
!65 = !{!66}
!66 = distinct !{!66, !64, !"daxpy: %dx"}
