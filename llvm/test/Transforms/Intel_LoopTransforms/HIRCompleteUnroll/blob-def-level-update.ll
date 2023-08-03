; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -hir-details 2>&1 < %s | FileCheck %s

; Verify that we update the definition level of %0 from 2 to 1 in the loop upper and ztt after unrolling i2 loop.

; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   + DO i2 = 0, 1, 1   <DO_LOOP> <unroll = 3>
; |   |   %0 = (%A)[i2];
; |   |
; |   |   + DO i3 = 0, sext.i32.i64(%0) + -1, 1   <DO_LOOP>
; |   |   |   %1 = (%B)[i3];
; |   |   |   (%B)[i3] = %1 + 1;
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP


; CHECK: DO i64 i2 = 0, sext.i32.i64(%0) + -1
; CHECK-NEXT: <RVAL-REG> LINEAR i64 sext.i32.i64(%0) + -1{def@1}
; CHECK-NEXT:   <BLOB> LINEAR i32 %0{def@1}
; CHECK-NEXT: <ZTT-REG> LINEAR i32 %0{def@1}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture readonly %A, ptr nocapture %B) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc13, %entry
  %i.028 = phi i32 [ 0, %entry ], [ %inc14, %for.inc13 ]
  br label %for.body3

for.body3:                                        ; preds = %for.end, %for.cond1.preheader
  %indvars.iv29 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next30, %for.end ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %indvars.iv29
  %0 = load i32, ptr %arrayidx, align 4
  %cmp525 = icmp sgt i32 %0, 0
  br i1 %cmp525, label %for.body6.preheader, label %for.end

for.body6.preheader:                              ; preds = %for.body3
  %wide.trip.count = sext i32 %0 to i64
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.preheader
  %indvars.iv = phi i64 [ 0, %for.body6.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx8 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx8, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body6

for.end.loopexit:                                 ; preds = %for.body6
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body3
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond31 = icmp eq i64 %indvars.iv.next30, 2
  br i1 %exitcond31, label %for.inc13, label %for.body3, !llvm.loop !6

for.inc13:                                        ; preds = %for.end
  %inc14 = add nuw nsw i32 %i.028, 1
  %exitcond32 = icmp eq i32 %inc14, 10
  br i1 %exitcond32, label %for.end15, label %for.cond1.preheader

for.end15:                                        ; preds = %for.inc13
  ret void
}

!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.unroll.count", i32 3}
