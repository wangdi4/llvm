; Test for OptPredicate with iv condition.
; This is not currently handled, so there should
; be no change.

; Source code:
;for (i=1; i < 1000; i++) {
;        for (j=1; j < 1000; j++) {
;            A[j][i] = B[i][j] + C[j][i] + B[i+1][j] + B[i][j+1];
;            if(j>10) {
;            } else {
;                B[i+1][j] = A[j][i] + C[i][j];
;            }
;        }
;        C[i][2] = i;
;    }

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s

; Check that region is not modified.
; CHECK-NOT: REGION { modified }
; CHECK: DO i1 =
; CHECK: DO i2 =
; CHECK-NOT: DO i1 = 
; CHECK-NOT: DO i2 =

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [1000 x [1000 x float]] zeroinitializer, align 16
@C = common global [1000 x [1000 x float]] zeroinitializer, align 16
@A = common global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.end, %entry
  %i.051 = phi i64 [ 1, %entry ], [ %add7, %for.end ]
  %add7 = add nuw nsw i64 %i.051, 1
  %arrayidx4.phi.trans.insert = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.051, i64 1
  %.pre = load float, float* %arrayidx4.phi.trans.insert, align 4, !tbaa !1
  br label %for.body.3

for.body.3:                                       ; preds = %for.cond.1.backedge, %for.cond.1.preheader
  %0 = phi float [ %.pre, %for.cond.1.preheader ], [ %3, %for.cond.1.backedge ]
  %j.050 = phi i64 [ 1, %for.cond.1.preheader ], [ %add11, %for.cond.1.backedge ]
  %arrayidx6 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %j.050, i64 %i.051
  %1 = load float, float* %arrayidx6, align 4, !tbaa !1
  %add = fadd float %0, %1
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add7, i64 %j.050
  %2 = load float, float* %arrayidx9, align 4, !tbaa !1
  %add10 = fadd float %add, %2
  %add11 = add nuw nsw i64 %j.050, 1
  %arrayidx13 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.051, i64 %add11
  %3 = load float, float* %arrayidx13, align 4, !tbaa !1
  %add14 = fadd float %add10, %3
  %arrayidx16 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.050, i64 %i.051
  store float %add14, float* %arrayidx16, align 4, !tbaa !1
  %cmp17 = icmp sgt i64 %j.050, 10
  br i1 %cmp17, label %if.then, label %for.cond.1.backedge

for.cond.1.backedge:                              ; preds = %for.body.3, %if.then
  %exitcond = icmp eq i64 %add11, 1000
  br i1 %exitcond, label %for.end, label %for.body.3

if.then:                                          ; preds = %for.body.3
  %arrayidx23 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add7, i64 %add11
  store float %add14, float* %arrayidx23, align 4, !tbaa !1
  br label %for.cond.1.backedge

for.end:                                          ; preds = %for.cond.1.backedge
  %conv = sitofp i64 %i.051 to float
  %arrayidx25 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.051, i64 2
  store float %conv, float* %arrayidx25, align 8, !tbaa !1
  %exitcond52 = icmp eq i64 %add7, 1000
  br i1 %exitcond52, label %for.end.28, label %for.cond.1.preheader

for.end.28:                                       ; preds = %for.end
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1359)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
