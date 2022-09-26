; Test to verify correctness of the PHI node fixing algorithm for a simple case
; of if-else diamond for a single variable within HCFG.

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-dump-external-defs-hir=0 -S -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-external-defs-hir=0 -S -disable-output < %s 2>&1 | FileCheck %s


; Input HIR
; <34>    + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>     |   %0 = (@a)[0][i1];
; <7>     |   %t1.0 = %0;
; <8>     |   if (i1 <= %0)
; <8>     |   {
; <13>    |      %t1.0 = (@a)[0][i1 + 1];
; <8>     |   }
; <18>    |   (@b)[0][i1] = %t1.0;
; <21>    |   (@c)[0][i1] = %t1.0 + 2 * %N;
; <23>    |   %3 = (@d)[0][i1];
; <27>    |   %spec.select = (i1 > %t1.0 + 2 * %N + %3) ? %t1.0 : %3;
; <28>    |   (@d)[0][i1] = %spec.select;
; <34>    + END LOOP

; Here a PHI node will be inserted for the DDRef %t1.0, incoming values are
; obtained from predecessor VPBBs

; Check the plain CFG structure and correctness of incoming values of PHI nodes
; CHECK-LABEL:  VPlan after importing plain CFG
; CHECK-NEXT:  VPlan IR for: foo:HIR
; CHECK-NEXT:    [[BB0:BB[0-9]+]]:
; CHECK-NEXT:     br [[BB1:BB[0-9]+]]
; CHECK:         [[BB1]]:
; CHECK-NEXT:     br [[BB2:BB[0-9]+]]
; CHECK:         [[BB2]]:
; CHECK-NEXT:     i64 [[VP0:%.*]] = phi  [ i64 0, [[BB1]] ],  [ i64 [[VP1:%.*]], [[BB3:BB[0-9]+]] ]
; CHECK:          i32 [[VP4:%.*]] = hir-copy i32 {{%vp.*}} , OriginPhiId: -1
; CHECK:          br i1 [[VP6:%vp.*]], [[BB4:BB[0-9]+]], [[BB3]]
; CHECK:           [[BB4]]:
; CHECK:            i32 [[VP9:%.*]] = hir-copy i32 {{%vp.*}} , OriginPhiId: -1
; CHECK-NEXT:       br [[BB3]]
; CHECK:         [[BB3]]:
; CHECK-NEXT:     i32 [[VP10:%.*]] = phi  [ i32 [[VP9]], [[BB4]] ],  [ i32 [[VP4]], [[BB2]] ]
; CHECK:          i64 [[VP1]] = add i64 [[VP0]] i64 1
; CHECK-NEXT:     i1 [[VP24:%.*]] = icmp slt i64 [[VP1]] i64 1024
; CHECK-NEXT:     br i1 [[VP24]], [[BB2]], [[BB5:BB[0-9]+]]
; CHECK:         [[BB5]]:
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK:         [[BB6]]:
; CHECK-NEXT:     br <External Block>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@d = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  %mul8 = shl nsw i32 %N, 1
  br label %for.body

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %.pre, %if.end ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %1 = sext i32 %0 to i64
  %cmp1 = icmp sgt i64 %indvars.iv, %1
  %.pre = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp1, label %if.end, label %if.else

if.else:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %.pre, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %for.body, %if.else
  %t1.0 = phi i32 [ %2, %if.else ], [ %0, %for.body ]
  %arrayidx7 = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %t1.0, i32* %arrayidx7, align 4, !tbaa !2
  %add9 = add nsw i32 %t1.0, %mul8
  %arrayidx11 = getelementptr inbounds [1024 x i32], [1024 x i32]* @c, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add9, i32* %arrayidx11, align 4, !tbaa !2
  %arrayidx15 = getelementptr inbounds [1024 x i32], [1024 x i32]* @d, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %3 = load i32, i32* %arrayidx15, align 4, !tbaa !2
  %add16 = add nsw i32 %3, %add9
  %4 = sext i32 %add16 to i64
  %cmp17 = icmp sgt i64 %indvars.iv, %4
  %spec.select = select i1 %cmp17, i32 %t1.0, i32 %3
  store i32 %spec.select, i32* %arrayidx15, align 4, !tbaa !2
  %exitcond = icmp eq i64 %.pre, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %if.end
  %5 = load i32, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @b, i64 0, i64 0), align 16, !tbaa !2
  %6 = load i32, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @c, i64 0, i64 0), align 16, !tbaa !2
  %add24 = add nsw i32 %6, %5
  %7 = load i32, i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @d, i64 0, i64 0), align 16, !tbaa !2
  %add25 = add nsw i32 %add24, %7
  ret i32 %add25
}



!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
