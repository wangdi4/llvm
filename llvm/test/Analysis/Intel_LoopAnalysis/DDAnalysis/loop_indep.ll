
;RUN: opt -S -basicaa -dda -dda-verify=Region -analyze %s | FileCheck %s
; matmul required suppressions of LICM and early CSE(did ld/st of c[][] in i3)
;          BEGIN REGION { }
;<35>         + DO i1 = 0, 99999, 1   <DO_LOOP>
;<34>         |   + DO i2 = 0, 99999, 1   <DO_LOOP>
;<5>          |   |   {al:4}(@C)[0][i1][i2] = 0.000000e+00;
;<33>         |   |   + DO i3 = 0, 99999, 1   <DO_LOOP>
;<9>          |   |   |   %0 = {al:4}(@A)[0][i1][i3];
;<11>         |   |   |   %1 = {al:4}(@B)[0][i3][i2];
;<12>         |   |   |   %mul = %0  *  %1;
;<13>         |   |   |   %2 = {al:4}(@C)[0][i1][i2];
;<14>         |   |   |   %add = %2  +  %mul;
;<15>         |   |   |   {al:4}(@C)[0][i1][i2] = %add;
;<33>         |   |   + END LOOP
;<34>         |   + END LOOP
;<35>         + END LOOP
;          END REGION

;CHECK: {al:4}(@C)[0][i1][i2] --> {al:4}(@C)[0][i1][i2] FLOW (= =)
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common global [100000 x [100000 x float]] zeroinitializer, align 16
@A = common global [100000 x [100000 x float]] zeroinitializer, align 16
@B = common global [100000 x [100000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @matrix_mul_matrix() #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.24, %entry
  %indvars.iv48 = phi i64 [ 0, %entry ], [ %indvars.iv.next49, %for.inc.24 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.inc.21, %for.cond.1.preheader
  %indvars.iv44 = phi i64 [ 0, %for.cond.1.preheader ], [ %indvars.iv.next45, %for.inc.21 ]
  %arrayidx5 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @C, i64 0, i64 %indvars.iv48, i64 %indvars.iv44
  store float 0.000000e+00, float* %arrayidx5, align 4, !tbaa !1
  br label %for.body.8

for.body.8:                                       ; preds = %for.body.8, %for.body.3
  %indvars.iv = phi i64 [ 0, %for.body.3 ], [ %indvars.iv.next, %for.body.8 ]
  %arrayidx12 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @A, i64 0, i64 %indvars.iv48, i64 %indvars.iv
  %0 = load float, float* %arrayidx12, align 4, !tbaa !1
  %arrayidx16 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv44
  %1 = load float, float* %arrayidx16, align 4, !tbaa !1
  %mul = fmul float %0, %1
  %2 = load float, float* %arrayidx5, align 4, !tbaa !1
  %add = fadd float %2, %mul
  store float %add, float* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond, label %for.inc.21, label %for.body.8

for.inc.21:                                       ; preds = %for.body.8
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 100000
  br i1 %exitcond46, label %for.inc.24, label %for.body.3

for.inc.24:                                       ; preds = %for.inc.21
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next49, 100000
  br i1 %exitcond50, label %for.end.26, label %for.cond.1.preheader

for.end.26:                                       ; preds = %for.inc.24
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1250) (llvm/branches/loopopt 1254)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
