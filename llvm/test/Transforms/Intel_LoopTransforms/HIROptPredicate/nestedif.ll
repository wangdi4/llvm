; Test for OptPredicate with nested if-else block.

; Source code:
; for (i=1; i < 1000; i++) {
;        for (j=1; j < 1000; j++) {
;            A[j][i] = B[i][j] + C[j][i] + B[i+1][j] + B[i][j+1];
;            if(n>10) {
;                B[i+1][j+1] = A[j][i];
;                if(m>10) {
;                  C[i][j] = i+j;
;                } else {
;                  B[i][j] = 2*i;
;                }
;            } else {
;                B[i+1][j] = A[j][i] + C[i][j];
;            }
;        }
;        C[i][2] = i;
;    }

; BEGIN REGION { }
; <62>         + DO i1 = 0, 998, 1   <DO_LOOP>
; <4>          |   %conv29 = sitofp.i64.float(2 * i1 + 2);
; <6>          |   %.pre = (@B)[0][i1 + 1][1];
; <7>          |   %hir.de.ssa.copy0.in = %.pre;
; <63>         |   + DO i2 = 0, 998, 1   <DO_LOOP>
; <13>         |   |   %1 = (@C)[0][i2 + 1][i1 + 1];
; <14>         |   |   %add = %hir.de.ssa.copy0.in  +  %1;
; <16>         |   |   %2 = (@B)[0][i1 + 2][i2 + 1];
; <17>         |   |   %add10 = %add  +  %2;
; <20>         |   |   %3 = (@B)[0][i1 + 1][i2 + 2];
; <21>         |   |   %add14 = %add10  +  %3;
; <23>         |   |   (@A)[0][i2 + 1][i1 + 1] = %add14;
; <24>         |   |   if (%n > 10)
; <24>         |   |   {
; <29>         |   |      (@B)[0][i1 + 2][i2 + 2] = %add14;
; <30>         |   |      if (%m > 10)
; <30>         |   |      {
; <35>         |   |         %conv = sitofp.i64.float(i1 + i2 + 2);
; <37>         |   |         (@C)[0][i1 + 1][i2 + 1] = %conv;
; <30>         |   |      }
; <30>         |   |      else
; <30>         |   |      {
; <40>         |   |         (@B)[0][i1 + 1][i2 + 1] = %conv29;
; <30>         |   |      }
; <24>         |   |   }
; <24>         |   |   else
; <24>         |   |   {
; <58>         |   |      %4 = (@C)[0][i1 + 1][i2 + 1];
; <59>         |   |      %add37 = %add14  +  %4;
; <60>         |   |      (@B)[0][i1 + 2][i2 + 1] = %add37;
; <24>         |   |   }
; <44>         |   |   %hir.de.ssa.copy0.in = %3;
; <63>         |   + END LOOP
; <49>         |   %conv42 = sitofp.i64.float(i1 + 1);
; <51>         |   (@C)[0][i1 + 1][2] = %conv42;
; <62>         + END LOOP
;          END REGION


; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s

; Check the then loop.
; CHECK: REGION { modified }
; CHECK: if (%n > 10)

; Check the nested if.
; CHECK: if (%m > 10)
; CHECK: DO i2 = 
; CHECK: (@B)[0][i1 + 2][i2 + 2] = 
; CHECK: (@C)[0][i1 + 1][i2 + 1] = 
; CHECK: END LOOP

; Check nested else.
; CHECK: else
; CHECK: DO i2 = 
; CHECK: (@B)[0][i1 + 2][i2 + 2] = 
; CHECK-NOT: (@C)[0][i1 + 1][i2 + 1] = 
; CHECK: (@B)[0][i1 + 1][i2 + 1] = 
; CHECK: END LOOP

; Check else block.
; CHECK: else
; CHECK: DO i2 = 0, 998, 1
; CHECK: (@B)[0][i1 + 2][i2 + 1] =
; CHECK: END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [1000 x [1000 x float]] zeroinitializer, align 16
@C = common global [1000 x [1000 x float]] zeroinitializer, align 16
@A = common global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n, i64 %m) #0 {
entry:
  %cmp17 = icmp sgt i64 %n, 10
  %cmp24 = icmp sgt i64 %m, 10
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.end, %entry
  %i.078 = phi i64 [ 1, %entry ], [ %add7, %for.end ]
  %add7 = add nuw nsw i64 %i.078, 1
  %mul = shl nsw i64 %i.078, 1
  %conv29 = sitofp i64 %mul to float
  %arrayidx4.phi.trans.insert = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.078, i64 1
  %.pre = load float, float* %arrayidx4.phi.trans.insert, align 4, !tbaa !1
  br label %for.body.3

for.body.3:                                       ; preds = %for.cond.1.backedge, %for.cond.1.preheader
  %0 = phi float [ %.pre, %for.cond.1.preheader ], [ %3, %for.cond.1.backedge ]
  %j.077 = phi i64 [ 1, %for.cond.1.preheader ], [ %add11, %for.cond.1.backedge ]
  %arrayidx4 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.078, i64 %j.077
  %arrayidx6 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %j.077, i64 %i.078
  %1 = load float, float* %arrayidx6, align 4, !tbaa !1
  %add = fadd float %0, %1
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add7, i64 %j.077
  %2 = load float, float* %arrayidx9, align 4, !tbaa !1
  %add10 = fadd float %add, %2
  %add11 = add nuw nsw i64 %j.077, 1
  %arrayidx13 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.078, i64 %add11
  %3 = load float, float* %arrayidx13, align 4, !tbaa !1
  %add14 = fadd float %add10, %3
  %arrayidx16 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.077, i64 %i.078
  store float %add14, float* %arrayidx16, align 4, !tbaa !1
  br i1 %cmp17, label %if.then, label %if.else.32

if.then:                                          ; preds = %for.body.3
  %arrayidx23 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add7, i64 %add11
  store float %add14, float* %arrayidx23, align 4, !tbaa !1
  br i1 %cmp24, label %if.then.25, label %if.else

if.then.25:                                       ; preds = %if.then
  %add26 = add nuw nsw i64 %j.077, %i.078
  %conv = sitofp i64 %add26 to float
  %arrayidx28 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.078, i64 %j.077
  store float %conv, float* %arrayidx28, align 4, !tbaa !1
  br label %for.cond.1.backedge

if.else:                                          ; preds = %if.then
  store float %conv29, float* %arrayidx4, align 4, !tbaa !1
  br label %for.cond.1.backedge

if.else.32:                                       ; preds = %for.body.3
  %arrayidx36 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.078, i64 %j.077
  %4 = load float, float* %arrayidx36, align 4, !tbaa !1
  %add37 = fadd float %add14, %4
  store float %add37, float* %arrayidx9, align 4, !tbaa !1
  br label %for.cond.1.backedge

for.cond.1.backedge:                              ; preds = %if.else.32, %if.else, %if.then.25
  %exitcond = icmp eq i64 %add11, 1000
  br i1 %exitcond, label %for.end, label %for.body.3

for.end:                                          ; preds = %for.cond.1.backedge
  %conv42 = sitofp i64 %i.078 to float
  %arrayidx44 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.078, i64 2
  store float %conv42, float* %arrayidx44, align 8, !tbaa !1
  %exitcond79 = icmp eq i64 %add7, 1000
  br i1 %exitcond79, label %for.end.47, label %for.cond.1.preheader

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

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1359)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
