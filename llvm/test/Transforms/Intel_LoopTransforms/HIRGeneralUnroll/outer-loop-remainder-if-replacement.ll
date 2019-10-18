; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-general-unroll -print-before=hir-general-unroll -print-after=hir-general-unroll -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-general-unroll,print<hir>" -S < %s 2>&1 | FileCheck %s

; Verify that the remainder loop of the outer loop which is unrolled by 2 is successfully replaced by its body.

; Dump Before-

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100> <unroll = 2>
; CHECK: |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100> <unroll = 2>
; CHECK: |   |   (@A)[0][i1 + i2] = i1 + i2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; Dump After-


; CHECK:   %tgu2 = (sext.i32.i64(%n))/u2;

; CHECK: + DO i1 = 0, %tgu2 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50> <nounroll>
; CHECK: |   %tgu = (sext.i32.i64(%n))/u2;
; CHECK: |
; CHECK: |   + DO i2 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50> <nounroll>
; CHECK: |   |   (@A)[0][2 * i1 + 2 * i2] = 2 * i1 + 2 * i2;
; CHECK: |   |   (@A)[0][2 * i1 + 2 * i2 + 1] = 2 * i1 + 2 * i2 + 1;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   if (2 * %tgu <u sext.i32.i64(%n))
; CHECK: |   {
; CHECK: |      (@A)[0][2 * i1 + 2 * %tgu] = 2 * i1 + (2 * %tgu);
; CHECK: |   }
; CHECK: |   %tgu = (sext.i32.i64(%n))/u2;
; CHECK: |
; CHECK: |   + DO i2 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50> <nounroll>
; CHECK: |   |   (@A)[0][2 * i1 + 2 * i2 + 1] = 2 * i1 + 2 * i2 + 1;
; CHECK: |   |   (@A)[0][2 * i1 + 2 * i2 + 2] = 2 * i1 + 2 * i2 + 2;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   if (2 * %tgu <u sext.i32.i64(%n))
; CHECK: |   {
; CHECK: |      (@A)[0][2 * i1 + 2 * %tgu + 1] = 2 * i1 + (2 * %tgu) + 1;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: if (2 * %tgu2 <u sext.i32.i64(%n))
; CHECK: {
; CHECK:   %tgu = (sext.i32.i64(%n))/u2;

; CHECK:   + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 50> <nounroll>
; CHECK:   |   (@A)[0][2 * i1 + 2 * %tgu2] = 2 * i1 + (2 * %tgu2);
; CHECK:   |   (@A)[0][2 * i1 + 2 * %tgu2 + 1] = 2 * i1 + (2 * %tgu2) + 1;
; CHECK:   + END LOOP

; CHECK:   if (2 * %tgu <u sext.i32.i64(%n))
; CHECK:   {
; CHECK:     (@A)[0][2 * %tgu + 2 * %tgu2] = (2 * %tgu) + (2 * %tgu2);
; CHECK:   }
; CHECK: }



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo(i32 %n) local_unnamed_addr {
entry:
  %cmp18 = icmp sgt i32 %n, 0
  br i1 %cmp18, label %for.cond1.preheader.lr.ph, label %for.end7

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count24 = sext i32 %n to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc5
  %indvars.iv22 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next23, %for.inc5 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv22
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %0
  %1 = trunc i64 %0 to i32
  store i32 %1, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count24
  br i1 %exitcond, label %for.inc5, label %for.body3, !llvm.loop !7

for.inc5:                                         ; preds = %for.body3
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next23, %wide.trip.count24
  br i1 %exitcond25, label %for.end7.loopexit, label %for.body3.preheader, !llvm.loop !9

for.end7.loopexit:                                ; preds = %for.inc5
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  ret void
}


!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll.count", i32 2}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.unroll.count", i32 2}
