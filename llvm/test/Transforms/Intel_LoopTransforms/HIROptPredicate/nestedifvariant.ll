; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -S < %s 2>&1 | FileCheck %s

; This test checks that the innermost If condition was hoisted out of the loop
; even if the outer If couldn't be hoisted.

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

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%m > 10)
; CHECK:       {
; CHECK:          + DO i1 = 0, 998, 1   <DO_LOOP>
; CHECK:          |   %conv29 = sitofp.i64.float(2 * i1 + 2);
; CHECK:          |   %.pre = (@B)[0][i1 + 1][1];
; CHECK:          |   %0 = %.pre;
; CHECK:          |   
; CHECK:          |   + DO i2 = 0, 998, 1   <DO_LOOP>
; CHECK:          |   |   %1 = (@C)[0][i2 + 1][i1 + 1];
; CHECK:          |   |   %add = %0  +  %1;
; CHECK:          |   |   %2 = (@B)[0][i1 + 2][i2 + 1];
; CHECK:          |   |   %add10 = %add  +  %2;
; CHECK:          |   |   %3 = (@B)[0][i1 + 1][i2 + 2];
; CHECK:          |   |   %add14 = %add10  +  %3;
; CHECK:          |   |   (@A)[0][i2 + 1][i1 + 1] = %add14;
; CHECK:          |   |   if (i2 + 1 > 10)
; CHECK:          |   |   {
; CHECK:          |   |      (@B)[0][i1 + 2][i2 + 2] = %add14;
; CHECK:          |   |      %conv = sitofp.i64.float(i1 + i2 + 2);
; CHECK:          |   |      (@C)[0][i1 + 1][i2 + 1] = %conv;
; CHECK:          |   |   }
; CHECK:          |   |   else
; CHECK:          |   |   {
; CHECK:          |   |      %4 = (@C)[0][i1 + 1][i2 + 1];
; CHECK:          |   |      %add37 = %add14  +  %4;
; CHECK:          |   |      (@B)[0][i1 + 2][i2 + 1] = %add37;
; CHECK:          |   |   }
; CHECK:          |   |   %0 = %3;
; CHECK:          |   + END LOOP
; CHECK:          |   
; CHECK:          |   %conv42 = sitofp.i64.float(i1 + 1);
; CHECK:          |   (@C)[0][i1 + 1][2] = %conv42;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          + DO i1 = 0, 998, 1   <DO_LOOP>
; CHECK:          |   %conv29 = sitofp.i64.float(2 * i1 + 2);
; CHECK:          |   %.pre = (@B)[0][i1 + 1][1];
; CHECK:          |   %0 = %.pre;
; CHECK:          |   
; CHECK:          |   + DO i2 = 0, 998, 1   <DO_LOOP>
; CHECK:          |   |   %1 = (@C)[0][i2 + 1][i1 + 1];
; CHECK:          |   |   %add = %0  +  %1;
; CHECK:          |   |   %2 = (@B)[0][i1 + 2][i2 + 1];
; CHECK:          |   |   %add10 = %add  +  %2;
; CHECK:          |   |   %3 = (@B)[0][i1 + 1][i2 + 2];
; CHECK:          |   |   %add14 = %add10  +  %3;
; CHECK:          |   |   (@A)[0][i2 + 1][i1 + 1] = %add14;
; CHECK:          |   |   if (i2 + 1 > 10)
; CHECK:          |   |   {
; CHECK:          |   |      (@B)[0][i1 + 2][i2 + 2] = %add14;
; CHECK:          |   |      (@B)[0][i1 + 1][i2 + 1] = %conv29;
; CHECK:          |   |   }
; CHECK:          |   |   else
; CHECK:          |   |   {
; CHECK:          |   |      %4 = (@C)[0][i1 + 1][i2 + 1];
; CHECK:          |   |      %add37 = %add14  +  %4;
; CHECK:          |   |      (@B)[0][i1 + 2][i2 + 1] = %add37;
; CHECK:          |   |   }
; CHECK:          |   |   %0 = %3;
; CHECK:          |   + END LOOP
; CHECK:          |   
; CHECK:          |   %conv42 = sitofp.i64.float(i1 + 1);
; CHECK:          |   (@C)[0][i1 + 1][2] = %conv42;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION


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
  %arrayidx4.phi.trans.insert = getelementptr inbounds [1000 x [1000 x float]], ptr @B, i64 0, i64 %i.079, i64 1
  %.pre = load float, ptr %arrayidx4.phi.trans.insert, align 4, !tbaa !1
  br label %for.body.3

for.body.3:                                       ; preds = %for.cond.1.backedge, %for.cond.1.preheader
  %0 = phi float [ %.pre, %for.cond.1.preheader ], [ %3, %for.cond.1.backedge ]
  %j.078 = phi i64 [ 1, %for.cond.1.preheader ], [ %add11, %for.cond.1.backedge ]
  %arrayidx4 = getelementptr inbounds [1000 x [1000 x float]], ptr @B, i64 0, i64 %i.079, i64 %j.078
  %arrayidx6 = getelementptr inbounds [1000 x [1000 x float]], ptr @C, i64 0, i64 %j.078, i64 %i.079
  %1 = load float, ptr %arrayidx6, align 4, !tbaa !1
  %add = fadd float %0, %1
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x float]], ptr @B, i64 0, i64 %add7, i64 %j.078
  %2 = load float, ptr %arrayidx9, align 4, !tbaa !1
  %add10 = fadd float %add, %2
  %add11 = add nuw nsw i64 %j.078, 1
  %arrayidx13 = getelementptr inbounds [1000 x [1000 x float]], ptr @B, i64 0, i64 %i.079, i64 %add11
  %3 = load float, ptr %arrayidx13, align 4, !tbaa !1
  %add14 = fadd float %add10, %3
  %arrayidx16 = getelementptr inbounds [1000 x [1000 x float]], ptr @A, i64 0, i64 %j.078, i64 %i.079
  store float %add14, ptr %arrayidx16, align 4, !tbaa !1
  %cmp17 = icmp sgt i64 %j.078, 10
  br i1 %cmp17, label %if.then, label %if.else.32

if.then:                                          ; preds = %for.body.3
  %arrayidx23 = getelementptr inbounds [1000 x [1000 x float]], ptr @B, i64 0, i64 %add7, i64 %add11
  store float %add14, ptr %arrayidx23, align 4, !tbaa !1
  br i1 %cmp24, label %if.then.25, label %if.else

if.then.25:                                       ; preds = %if.then
  %add26 = add nuw nsw i64 %j.078, %i.079
  %conv = sitofp i64 %add26 to float
  %arrayidx28 = getelementptr inbounds [1000 x [1000 x float]], ptr @C, i64 0, i64 %i.079, i64 %j.078
  store float %conv, ptr %arrayidx28, align 4, !tbaa !1
  br label %for.cond.1.backedge

if.else:                                          ; preds = %if.then
  store float %conv29, ptr %arrayidx4, align 4, !tbaa !1
  br label %for.cond.1.backedge

if.else.32:                                       ; preds = %for.body.3
  %arrayidx36 = getelementptr inbounds [1000 x [1000 x float]], ptr @C, i64 0, i64 %i.079, i64 %j.078
  %4 = load float, ptr %arrayidx36, align 4, !tbaa !1
  %add37 = fadd float %add14, %4
  store float %add37, ptr %arrayidx9, align 4, !tbaa !1
  br label %for.cond.1.backedge

for.cond.1.backedge:                              ; preds = %if.else.32, %if.else, %if.then.25
  %exitcond = icmp eq i64 %add11, 1000
  br i1 %exitcond, label %for.end, label %for.body.3

for.end:                                          ; preds = %for.cond.1.backedge
  %conv42 = sitofp i64 %i.079 to float
  %arrayidx44 = getelementptr inbounds [1000 x [1000 x float]], ptr @C, i64 0, i64 %i.079, i64 2
  store float %conv42, ptr %arrayidx44, align 8, !tbaa !1
  %exitcond80 = icmp eq i64 %add7, 1000
  br i1 %exitcond80, label %for.end.47, label %for.cond.1.preheader

for.end.47:                                       ; preds = %for.end
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1312) (llvm/branches/loopopt 1412)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
