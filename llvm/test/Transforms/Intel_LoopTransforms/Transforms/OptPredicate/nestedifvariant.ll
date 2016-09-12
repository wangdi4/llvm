; Test for OptPredicate with nested if and where the inner IF is linear. Such IF could not be hoisted
; as the containing IF is already at the topmost level.

; Source Code
; void sub3 (long int n, long int m) {
;    long int i, j;
;    for (i=1; i < 1000; i++) {
;        for (j=1; j < 1000; j++) {
;            A[j][i] = B[i][j] + C[j][i] + B[i+1][j] + B[i][j+1];
;            if(j>10) {
;                B[i+1][j+1] = A[j][i];
;                if(m>10) {
;                  C[i][j] = i+j;
;                } else {
;                  B[i][j] = 2*i;
;                }
;            } else {
;               B[i+1][j] = A[j][i] + C[i][j];
;            }
;        }
;        C[i][2] = i;
;    } }

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s

; CHECK: After
; CHECK: REGION { }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [1000 x [1000 x float]] zeroinitializer, align 16
@C = common global [1000 x [1000 x float]] zeroinitializer, align 16
@A = common global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n, i64 %m) #0 {
entry:
  %cmp24 = icmp sgt i64 %m, 10
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.end, %entry
  %i.079 = phi i64 [ 1, %entry ], [ %add7, %for.end ]
  %add7 = add nuw nsw i64 %i.079, 1
  %mul = shl nsw i64 %i.079, 1
  %conv29 = sitofp i64 %mul to float
  %arrayidx4.phi.trans.insert = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.079, i64 1
  %.pre = load float, float* %arrayidx4.phi.trans.insert, align 4, !tbaa !1
  br label %for.body.3

for.body.3:                                       ; preds = %for.cond.1.backedge, %for.cond.1.preheader
  %0 = phi float [ %.pre, %for.cond.1.preheader ], [ %3, %for.cond.1.backedge ]
  %j.078 = phi i64 [ 1, %for.cond.1.preheader ], [ %add11, %for.cond.1.backedge ]
  %arrayidx4 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.079, i64 %j.078
  %arrayidx6 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %j.078, i64 %i.079
  %1 = load float, float* %arrayidx6, align 4, !tbaa !1
  %add = fadd float %0, %1
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add7, i64 %j.078
  %2 = load float, float* %arrayidx9, align 4, !tbaa !1
  %add10 = fadd float %add, %2
  %add11 = add nuw nsw i64 %j.078, 1
  %arrayidx13 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.079, i64 %add11
  %3 = load float, float* %arrayidx13, align 4, !tbaa !1
  %add14 = fadd float %add10, %3
  %arrayidx16 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.078, i64 %i.079
  store float %add14, float* %arrayidx16, align 4, !tbaa !1
  %cmp17 = icmp sgt i64 %j.078, 10
  br i1 %cmp17, label %if.then, label %if.else.32

if.then:                                          ; preds = %for.body.3
  %arrayidx23 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add7, i64 %add11
  store float %add14, float* %arrayidx23, align 4, !tbaa !1
  br i1 %cmp24, label %if.then.25, label %if.else

if.then.25:                                       ; preds = %if.then
  %add26 = add nuw nsw i64 %j.078, %i.079
  %conv = sitofp i64 %add26 to float
  %arrayidx28 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.079, i64 %j.078
  store float %conv, float* %arrayidx28, align 4, !tbaa !1
  br label %for.cond.1.backedge

if.else:                                          ; preds = %if.then
  store float %conv29, float* %arrayidx4, align 4, !tbaa !1
  br label %for.cond.1.backedge

if.else.32:                                       ; preds = %for.body.3
  %arrayidx36 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.079, i64 %j.078
  %4 = load float, float* %arrayidx36, align 4, !tbaa !1
  %add37 = fadd float %add14, %4
  store float %add37, float* %arrayidx9, align 4, !tbaa !1
  br label %for.cond.1.backedge

for.cond.1.backedge:                              ; preds = %if.else.32, %if.else, %if.then.25
  %exitcond = icmp eq i64 %add11, 1000
  br i1 %exitcond, label %for.end, label %for.body.3

for.end:                                          ; preds = %for.cond.1.backedge
  %conv42 = sitofp i64 %i.079 to float
  %arrayidx44 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.079, i64 2
  store float %conv42, float* %arrayidx44, align 8, !tbaa !1
  %exitcond80 = icmp eq i64 %add7, 1000
  br i1 %exitcond80, label %for.end.47, label %for.cond.1.preheader

for.end.47:                                       ; preds = %for.end
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1412)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
