; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir=0 -print-after=VPlanDriverHIR -hir-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -vplan-force-vf=4 -enable-vp-value-codegen-hir=1 -print-after=VPlanDriverHIR -hir-details -disable-output < %s 2>&1 | FileCheck %s
;
; LIT test to check that we preserve alias-analysis related metadata in HIR vector code generation

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64* %larr1, i64* readonly %larr2) {
; CHECK-LABEL:  *** IR Dump After VPlan Vectorization Driver HIR ***
; CHECK:     + DO i64 i1 = 0, 99, 4   <DO_LOOP> <nounroll> <novectorize>
; CHECK:     |   [[VEC:.*]] = (<4 x i64>*)(%larr2)[i1];
; CHECK:     |   <RVAL-REG> {al:8}(<4 x i64>*)(LINEAR i64* %larr2)[LINEAR i64 i1] inbounds  !tbaa !1 !alias.scope !5
; CHECK:     |   (<4 x i64>*)(%larr1)[i1] = [[VEC]] + 1;
; CHECK:     |   <LVAL-REG> {al:8}(<4 x i64>*)(LINEAR i64* %larr1)[LINEAR i64 i1] inbounds  !tbaa !1 !noalias !5
; CHECK:     + END LOOP
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %larr2, i64 %l1.06
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !2, !alias.scope !10
  %add = add nsw i64 %0, 1
  %arrayidx1 = getelementptr inbounds i64, i64* %larr1, i64 %l1.06
  store i64 %add, i64* %arrayidx1, align 8, !tbaa !2, !noalias !10
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.unroll.disable"}

; Scope domain
!8 = !{!0}
!9 = !{!9, !8}
!10 = !{!9}
