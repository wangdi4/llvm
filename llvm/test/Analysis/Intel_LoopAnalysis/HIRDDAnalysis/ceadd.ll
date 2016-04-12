; Test for basic CE adds during DD Testing
; example: i and i-1

; Check for DD test output 
; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region | FileCheck %s
; CHECK-DAG: {al:4}(%b)[i1] --> {al:4}(%b)[i1 + -1] ANTI (<)
; CHECK-DAG: {al:4}(%b)[i1 + 1] --> {al:4}(%b)[i1] FLOW (<)
; CHECK-DAG: {al:4}(%b)[i1 + 1] --> {al:4}(%b)[i1 + -1] OUTPUT (<)

; # Source Code
; int sub(unsigned  *b,  unsigned deg ) {
; for (unsigned i=0; i< 3; i++ ) {
;    b[i+1] = b[i] * (deg -1);
;    b[i-1] = b[i] * (deg +1);
; }
; return 0;
; }


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @_Z3subPjj(i32* nocapture %b, i32 %deg) #0 {
entry:
  %sub = add i32 %deg, -1
  %add5 = add i32 %deg, 1
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 0

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %mul = mul i32 %0, %sub
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv.next
  store i32 %mul, i32* %arrayidx2, align 4, !tbaa !1
  %mul6 = mul i32 %0, %add5
  %sub7 = add i64 %indvars.iv, 4294967295
  %idxprom8 = and i64 %sub7, 4294967295
  %arrayidx9 = getelementptr inbounds i32, i32* %b, i64 %idxprom8
  store i32 %mul6, i32* %arrayidx9, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2121) (llvm/branches/loopopt 2246)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
