; Test to check if the canon exprs level are
; updated after the opt predicate transformation.

; Source code
; float A[1000][1000], B[1000][1000], C[1000][1000];
; void sub3 (long int n, long int m) {
;     long int i, j, t;
;    for (i=1; i < 1000; i++) {
;        t = A[i][i];
;        for (j=1; j < 1000; j++) {
;            A[j][i] = B[i][j] + C[j][i] + B[i+1][j] + B[i][j+1];
;            if(t>10) {
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
;    }}



; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -hir-details < %s 2>&1 | FileCheck %s

; Traverse till the loop
; CHECK: i1 = 0, 998, 1

; CHECK: %[[TMP:[a-zA-Z0-9.]+]] = fptosi.float.i64(%0)
; CHECK: <LVAL-REG> NON-LINEAR i64 %[[TMP]]

; Check opt predicated HLIf which should have non-linear marking.
; Here TMP is defined at the same level.
; CHECK: if (%[[TMP]] > 10)
; CHECK: <RVAL-REG> NON-LINEAR i64 %conv

; Check opt predicated If which should be linear and placed after the (%t > 10).
; %m is live-in variable.
; CHECK: if (%m > 10)
; <RVAL-REG> LINEAR i64 %m

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x float]] zeroinitializer, align 16
@B = common global [1000 x [1000 x float]] zeroinitializer, align 16
@C = common global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n, i64 %m) #0 {
entry:
  %cmp28 = icmp sgt i64 %m, 10
  br label %for.body

for.body:                                         ; preds = %for.end, %entry
  %i.087 = phi i64 [ 1, %entry ], [ %add10, %for.end ]
  %arrayidx1 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %i.087, i64 %i.087
  %0 = load float, float* %arrayidx1, align 4, !tbaa !1
  %conv = fptosi float %0 to i64
  %add10 = add nuw nsw i64 %i.087, 1
  %cmp20 = icmp sgt i64 %conv, 10
  %mul = shl nsw i64 %i.087, 1
  %conv35 = sitofp i64 %mul to float
  %arrayidx7.phi.trans.insert = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.087, i64 1
  %.pre = load float, float* %arrayidx7.phi.trans.insert, align 4, !tbaa !1
  br label %for.body.5

for.body.5:                                       ; preds = %for.cond.2.backedge, %for.body
  %1 = phi float [ %.pre, %for.body ], [ %4, %for.cond.2.backedge ]
  %j.086 = phi i64 [ 1, %for.body ], [ %add14, %for.cond.2.backedge ]
  %arrayidx7 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.087, i64 %j.086
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %j.086, i64 %i.087
  %2 = load float, float* %arrayidx9, align 4, !tbaa !1
  %add = fadd float %1, %2
  %arrayidx12 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add10, i64 %j.086
  %3 = load float, float* %arrayidx12, align 4, !tbaa !1
  %add13 = fadd float %add, %3
  %add14 = add nuw nsw i64 %j.086, 1
  %arrayidx16 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %i.087, i64 %add14
  %4 = load float, float* %arrayidx16, align 4, !tbaa !1
  %add17 = fadd float %add13, %4
  %arrayidx19 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.086, i64 %i.087
  store float %add17, float* %arrayidx19, align 4, !tbaa !1
  br i1 %cmp20, label %if.then, label %if.else.38

if.then:                                          ; preds = %for.body.5
  %arrayidx27 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %add10, i64 %add14
  store float %add17, float* %arrayidx27, align 4, !tbaa !1
  br i1 %cmp28, label %if.then.30, label %if.else

if.then.30:                                       ; preds = %if.then
  %add31 = add nuw nsw i64 %j.086, %i.087
  %conv32 = sitofp i64 %add31 to float
  %arrayidx34 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.087, i64 %j.086
  store float %conv32, float* %arrayidx34, align 4, !tbaa !1
  br label %for.cond.2.backedge

if.else:                                          ; preds = %if.then
  store float %conv35, float* %arrayidx7, align 4, !tbaa !1
  br label %for.cond.2.backedge

if.else.38:                                       ; preds = %for.body.5
  %arrayidx42 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.087, i64 %j.086
  %5 = load float, float* %arrayidx42, align 4, !tbaa !1
  %add43 = fadd float %add17, %5
  store float %add43, float* %arrayidx12, align 4, !tbaa !1
  br label %for.cond.2.backedge

for.cond.2.backedge:                              ; preds = %if.else.38, %if.else, %if.then.30
  %exitcond = icmp eq i64 %add14, 1000
  br i1 %exitcond, label %for.end, label %for.body.5

for.end:                                          ; preds = %for.cond.2.backedge
  %conv48 = sitofp i64 %i.087 to float
  %arrayidx50 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %i.087, i64 2
  store float %conv48, float* %arrayidx50, align 8, !tbaa !1
  %exitcond88 = icmp eq i64 %add10, 1000
  br i1 %exitcond88, label %for.end.53, label %for.body

for.end.53:                                       ; preds = %for.end
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind argmemonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1548)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
