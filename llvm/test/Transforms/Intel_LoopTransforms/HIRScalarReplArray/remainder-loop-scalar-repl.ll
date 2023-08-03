; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that scalar-replactment works correctly for remainder loops.

; CHECK: Function

; CHECK: + DO i1 = 4 * %tgu, zext.i32.i64(%indvars.iv16) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK: |   %1 = (@sour)[0][i1 + 1];
; CHECK: |   %3 = (@dest)[0][%__index.addr.014][i1];
; CHECK: |   (@dest)[0][%__index.addr.014][i1 + 1] = %1 + %3;
; CHECK: + END LOOP


; CHECK: Function

; CHECK:  %scalarepl13 = (@dest)[0][%__index.addr.014][4 * %tgu];
; CHECK:  + DO i1 = 4 * %tgu, zext.i32.i64(%indvars.iv16) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK:  |   %1 = (@sour)[0][i1 + 1];
; CHECK:  |   %3 = %scalarepl13;
; CHECK:  |   %scalarepl14 = %1 + %3;
; CHECK:  |   (@dest)[0][%__index.addr.014][i1 + 1] = %scalarepl14;
; CHECK:  |   %scalarepl13 = %scalarepl14;
; CHECK:  + END LOOP
;

;Module Before HIR; ModuleID = 'cq560929.c'
source_filename = "cq560929.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sour = common local_unnamed_addr global [64 x i32] zeroinitializer, align 16
@dest = common local_unnamed_addr global [64 x [64 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @roo(i32 %u1, i32 %__index.addr.014, i32 %indvars.iv16) local_unnamed_addr {
entry:
  %0 = icmp sgt i32 %u1, 0
  br label %for.body.i.lr.ph

for.body.i.lr.ph:                                 ; preds = %entry
  %idxprom7.i = sext i32 %__index.addr.014 to i64
  %wide.trip.count = zext i32 %indvars.iv16 to i64
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.lr.ph
  %indvars.iv = phi i64 [ 1, %for.body.i.lr.ph ], [ %indvars.iv.next, %for.body.i ]
  %arrayidx.i = getelementptr inbounds [64 x i32], ptr @sour, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx.i, align 4, !tbaa !3
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx11.i = getelementptr inbounds [64 x [64 x i32]], ptr @dest, i64 0, i64 %idxprom7.i, i64 %2
  %3 = load i32, ptr %arrayidx11.i, align 4, !tbaa !8
  %add12.i = add nsw i32 %3, %1
  %arrayidx18.i = getelementptr inbounds [64 x [64 x i32]], ptr @dest, i64 0, i64 %idxprom7.i, i64 %indvars.iv
  store i32 %add12.i, ptr %arrayidx18.i, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %exit, label %for.body.i, !llvm.loop !10

exit:                                           ; preds = %for.body.i
  ret void
}

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20848) (llvm/branches/loopopt 20950)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA64_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA64_A64_i", !4, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.unroll.count", i32 4}

