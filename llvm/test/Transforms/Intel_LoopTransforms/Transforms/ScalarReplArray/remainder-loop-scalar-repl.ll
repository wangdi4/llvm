; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s

; Check that scalar-replactment works correctly for remainder loops.

; CHECK: Before HIR Scalar Replacement

; CHECK: + DO i1 = 4 * %tgu, zext.i32.i64(%indvars.iv16) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK: |   %1 = (@sour)[0][i1 + 1];
; CHECK: |   %3 = (@dest)[0][%__index.addr.014][i1];
; CHECK: |   (@dest)[0][%__index.addr.014][i1 + 1] = %1 + %3;
; CHECK: + END LOOP

; CHECK: After HIR Scalar Replacement

; CHECK: %scalarepl5 = (@dest)[0][%__index.addr.014][4 * %tgu];
; CHECK: + DO i1 = 4 * %tgu, zext.i32.i64(%indvars.iv16) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK: |   %1 = (@sour)[0][i1 + 1];
; CHECK: |   %3 = %scalarepl5;
; CHECK: |   %scalarepl6 = %1 + %3;
; CHECK: |   (@dest)[0][%__index.addr.014][i1 + 1] = %scalarepl6;
; CHECK: |   %scalarepl5 = %scalarepl6;
; CHECK: + END LOOP

;Module Before HIR; ModuleID = 'cq560929.c'
source_filename = "cq560929.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sour = common local_unnamed_addr global [64 x i32] zeroinitializer, align 16
@dest = common local_unnamed_addr global [64 x [64 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @roo(i32 %u1, i32 %u2) local_unnamed_addr {
entry:
  %0 = icmp sgt i32 %u1, 0
  br i1 %0, label %for.cond.i.preheader.preheader, label %if.end, !llvm.loop !1

for.cond.i.preheader.preheader:                   ; preds = %entry
  br label %for.cond.i.preheader

for.cond.i.preheader:                             ; preds = %for.cond.i.preheader.preheader, %__simd_for_helper.exit
  %indvars.iv16 = phi i32 [ %indvars.iv.next17, %__simd_for_helper.exit ], [ %u2, %for.cond.i.preheader.preheader ]
  %__index.addr.014 = phi i32 [ %4, %__simd_for_helper.exit ], [ 0, %for.cond.i.preheader.preheader ]
  %add.i = add nsw i32 %__index.addr.014, %u2
  %cmp.i12 = icmp sgt i32 %add.i, 1
  br i1 %cmp.i12, label %for.body.i.lr.ph, label %__simd_for_helper.exit

for.body.i.lr.ph:                                 ; preds = %for.cond.i.preheader
  %idxprom7.i = sext i32 %__index.addr.014 to i64
  %wide.trip.count = zext i32 %indvars.iv16 to i64
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.lr.ph
  %indvars.iv = phi i64 [ 1, %for.body.i.lr.ph ], [ %indvars.iv.next, %for.body.i ]
  %arrayidx.i = getelementptr inbounds [64 x i32], [64 x i32]* @sour, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx.i, align 4, !tbaa !3
  %2 = add nsw i64 %indvars.iv, -1
  %arrayidx11.i = getelementptr inbounds [64 x [64 x i32]], [64 x [64 x i32]]* @dest, i64 0, i64 %idxprom7.i, i64 %2
  %3 = load i32, i32* %arrayidx11.i, align 4, !tbaa !8
  %add12.i = add nsw i32 %3, %1
  %arrayidx18.i = getelementptr inbounds [64 x [64 x i32]], [64 x [64 x i32]]* @dest, i64 0, i64 %idxprom7.i, i64 %indvars.iv
  store i32 %add12.i, i32* %arrayidx18.i, align 4, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %__simd_for_helper.exit.loopexit, label %for.body.i

__simd_for_helper.exit.loopexit:                  ; preds = %for.body.i
  br label %__simd_for_helper.exit

__simd_for_helper.exit:                           ; preds = %__simd_for_helper.exit.loopexit, %for.cond.i.preheader
  %4 = add nuw i32 %__index.addr.014, 1
  %indvars.iv.next17 = add i32 %indvars.iv16, 1
  %exitcond18 = icmp eq i32 %4, %u1
  br i1 %exitcond18, label %if.end.loopexit, label %for.cond.i.preheader, !llvm.loop !1

if.end.loopexit:                                  ; preds = %__simd_for_helper.exit
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %entry
  ret void
}

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20848) (llvm/branches/loopopt 20950)"}
!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.vectorize.enable", i1 true}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA64_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA64_A64_i", !4, i64 0}
