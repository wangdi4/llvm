; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-post-vec-complete-unroll,hir-lmm,print<hir>" -aa-pipeline="basic-aa" -hir-complete-unroll-loopnest-trip-threshold=50 < %s 2>&1 | FileCheck %s
; (This test is based on Interchange/matmul-coremark.ll)
;
;
; LIMM's Opportunity:
; - LILH:   (0)
; - LISS:   (0)
; - LILHSS:(25)
;
;
; CHECK: Function
; CHECK: matmul
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           + DO i1
; CHECK:           |   + DO i2
; CHECK:           |   |      %limm = (@vy)[0][i1 + sext.i32.i64(%n) * i2];
; CHECK:           |   |      %limm4 = (@px)[0][25 * i1];
; CHECK:           |   |      %limm5 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 1];
; CHECK:           |   |      %limm6 = (@px)[0][25 * i1 + 1];
; CHECK:           |   |      %limm8 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 2];
; CHECK:           |   |      %limm9 = (@px)[0][25 * i1 + 2];
; CHECK:           |   |      %limm11 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 3];
; CHECK:           |   |      %limm12 = (@px)[0][25 * i1 + 3];
; CHECK:           |   |      %limm14 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 4];
; CHECK:           |   |      %limm15 = (@px)[0][25 * i1 + 4];
; CHECK:           |   |      %limm17 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 5];
; CHECK:           |   |      %limm18 = (@px)[0][25 * i1 + 5];
; CHECK:           |   |      %limm20 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 6];
; CHECK:           |   |      %limm21 = (@px)[0][25 * i1 + 6];
; CHECK:           |   |      %limm23 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 7];
; CHECK:           |   |      %limm24 = (@px)[0][25 * i1 + 7];
; CHECK:           |   |      %limm26 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 8];
; CHECK:           |   |      %limm27 = (@px)[0][25 * i1 + 8];
; CHECK:           |   |      %limm29 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 9];
; CHECK:           |   |      %limm30 = (@px)[0][25 * i1 + 9];
; CHECK:           |   |      %limm32 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 10];
; CHECK:           |   |      %limm33 = (@px)[0][25 * i1 + 10];
; CHECK:           |   |      %limm35 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 11];
; CHECK:           |   |      %limm36 = (@px)[0][25 * i1 + 11];
; CHECK:           |   |      %limm38 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 12];
; CHECK:           |   |      %limm39 = (@px)[0][25 * i1 + 12];
; CHECK:           |   |      %limm41 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 13];
; CHECK:           |   |      %limm42 = (@px)[0][25 * i1 + 13];
; CHECK:           |   |      %limm44 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 14];
; CHECK:           |   |      %limm45 = (@px)[0][25 * i1 + 14];
; CHECK:           |   |      %limm47 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 15];
; CHECK:           |   |      %limm48 = (@px)[0][25 * i1 + 15];
; CHECK:           |   |      %limm50 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 16];
; CHECK:           |   |      %limm51 = (@px)[0][25 * i1 + 16];
; CHECK:           |   |      %limm53 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 17];
; CHECK:           |   |      %limm54 = (@px)[0][25 * i1 + 17];
; CHECK:           |   |      %limm56 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 18];
; CHECK:           |   |      %limm57 = (@px)[0][25 * i1 + 18];
; CHECK:           |   |      %limm59 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 19];
; CHECK:           |   |      %limm60 = (@px)[0][25 * i1 + 19];
; CHECK:           |   |      %limm62 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 20];
; CHECK:           |   |      %limm63 = (@px)[0][25 * i1 + 20];
; CHECK:           |   |      %limm65 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 21];
; CHECK:           |   |      %limm66 = (@px)[0][25 * i1 + 21];
; CHECK:           |   |      %limm68 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 22];
; CHECK:           |   |      %limm69 = (@px)[0][25 * i1 + 22];
; CHECK:           |   |      %limm71 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 23];
; CHECK:           |   |      %limm72 = (@px)[0][25 * i1 + 23];
; CHECK:           |   |      %limm74 = (@vy)[0][i1 + sext.i32.i64(%n) * i2 + 24];
; CHECK:           |   |      %limm75 = (@px)[0][25 * i1 + 24];
; CHECK:           |   |   + DO i3
; CHECK:           |   |   + END LOOP
; CHECK:           |   |      (@px)[0][25 * i1 + 24] = %limm75;
; CHECK:           |   |      (@px)[0][25 * i1 + 23] = %limm72;
; CHECK:           |   |      (@px)[0][25 * i1 + 22] = %limm69;
; CHECK:           |   |      (@px)[0][25 * i1 + 21] = %limm66;
; CHECK:           |   |      (@px)[0][25 * i1 + 20] = %limm63;
; CHECK:           |   |      (@px)[0][25 * i1 + 19] = %limm60;
; CHECK:           |   |      (@px)[0][25 * i1 + 18] = %limm57;
; CHECK:           |   |      (@px)[0][25 * i1 + 17] = %limm54;
; CHECK:           |   |      (@px)[0][25 * i1 + 16] = %limm51;
; CHECK:           |   |      (@px)[0][25 * i1 + 15] = %limm48;
; CHECK:           |   |      (@px)[0][25 * i1 + 14] = %limm45;
; CHECK:           |   |      (@px)[0][25 * i1 + 13] = %limm42;
; CHECK:           |   |      (@px)[0][25 * i1 + 12] = %limm39;
; CHECK:           |   |      (@px)[0][25 * i1 + 11] = %limm36;
; CHECK:           |   |      (@px)[0][25 * i1 + 10] = %limm33;
; CHECK:           |   |      (@px)[0][25 * i1 + 9] = %limm30;
; CHECK:           |   |      (@px)[0][25 * i1 + 8] = %limm27;
; CHECK:           |   |      (@px)[0][25 * i1 + 7] = %limm24;
; CHECK:           |   |      (@px)[0][25 * i1 + 6] = %limm21;
; CHECK:           |   |      (@px)[0][25 * i1 + 5] = %limm18;
; CHECK:           |   |      (@px)[0][25 * i1 + 4] = %limm15;
; CHECK:           |   |      (@px)[0][25 * i1 + 3] = %limm12;
; CHECK:           |   |      (@px)[0][25 * i1 + 2] = %limm9;
; CHECK:           |   |      (@px)[0][25 * i1 + 1] = %limm6;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:  END REGION
;
;
; ModuleID = 'matmul-coremark.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@px = common global [1000 x float] zeroinitializer, align 16
@vy = common global [1000 x float] zeroinitializer, align 16
@cx = common global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @matmul(i32 %loop, i32 %n) #0 {
entry:
  %cmp59 = icmp slt i32 %loop, 1
  br i1 %cmp59, label %for.end34, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp854 = icmp sgt i32 %n, 0
  %0 = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc32, %for.cond1.preheader.lr.ph
  %l.060 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %inc33, %for.inc32 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc29, %for.cond1.preheader
  %indvars.iv68 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next69, %for.inc29 ]
  %1 = mul nsw i64 %indvars.iv68, %0
  %2 = trunc i64 %indvars.iv68 to i32
  %add16 = add nuw i32 %2, %l.060
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.inc26, %for.cond4.preheader
  %indvars.iv64 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next65, %for.inc26 ]
  br i1 %cmp854, label %for.body9.lr.ph, label %for.inc26

for.body9.lr.ph:                                  ; preds = %for.cond7.preheader
  %3 = add nsw i64 %indvars.iv64, %1
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %for.body9.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next, %for.body9 ]
  %4 = mul nuw nsw i64 %indvars.iv, 25
  %5 = add nuw nsw i64 %4, %indvars.iv64
  %arrayidx = getelementptr inbounds [1000 x float], ptr @px, i64 0, i64 %5
  %6 = load float, ptr %arrayidx, align 4, !tbaa !1
  %7 = add nsw i64 %3, %indvars.iv
  %arrayidx14 = getelementptr inbounds [1000 x float], ptr @vy, i64 0, i64 %7
  %8 = load float, ptr %arrayidx14, align 4, !tbaa !1
  %9 = trunc i64 %4 to i32
  %add17 = add i32 %add16, %9
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds [1000 x float], ptr @cx, i64 0, i64 %idxprom18
  %10 = load float, ptr %arrayidx19, align 4, !tbaa !1
  %mul20 = fmul float %8, %10
  %add21 = fadd float %6, %mul20
  store float %add21, ptr %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc26, label %for.body9

for.inc26:                                        ; preds = %for.body9, %for.cond7.preheader
  %indvars.iv.next65 = add nuw nsw i64 %indvars.iv64, 1
  %exitcond67 = icmp eq i64 %indvars.iv.next65, 25
  br i1 %exitcond67, label %for.inc29, label %for.cond7.preheader

for.inc29:                                        ; preds = %for.inc26
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1
  %exitcond71 = icmp eq i64 %indvars.iv.next69, 25
  br i1 %exitcond71, label %for.inc32, label %for.cond4.preheader

for.inc32:                                        ; preds = %for.inc29
  %inc33 = add nuw nsw i32 %l.060, 1
  %exitcond72 = icmp eq i32 %l.060, %loop
  br i1 %exitcond72, label %for.end34, label %for.cond1.preheader

for.end34:                                        ; preds = %for.inc32, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 8655)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
