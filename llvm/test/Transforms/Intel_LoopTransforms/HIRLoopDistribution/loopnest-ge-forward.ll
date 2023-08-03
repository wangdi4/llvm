; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -S -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-loopnest" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Check that DO i2 loop will not be distributed because of forward edge with i2 direction (>=).

; BEGIN REGION { }
;       + DO i1 = 0, 0, 1   <DO_LOOP>
;       |   + DO i2 = 0, i1 + 5, 1   <DO_LOOP>  <MAX_TC_EST = 6>
;       |   |   + DO i3 = 0, 0, 1   <DO_LOOP>
; <1>   |   |   |   (%z3)[0][i3 + 3][-1 * i1 + i2 + 3] = 1;
;       |   |   + END LOOP
;       |   |
; <2>   |   |   (%z3)[0][-1 * i1 + i2 + 3][-1 * i1 + 5] = 0;
;       |   + END LOOP
;       + END LOOP
; END REGION

; 1:2 (%z3)[0][i3 + 3][-1 * i1 + i2 + 3] --> (%z3)[0][-1 * i1 + i2 + 3][-1 * i1 + 5] OUTPUT (<= >=)
;
; Stmt\Iter	0	1	2	3	4	5
; <1>		[3][3]	[3][4]	[3][5]	[3][6]	[3][7]	[3][8]
; <2>		[3][5]	[4][5]	[5][5]	[6][5]	[7][5]	[8][5]

; CHECK: BEGIN REGION
; CHECK: DO i1
; CHECK:   DO i2
; CHECK-NOT: DO i2

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRLoopDistributionForLoopNest

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %z3 = alloca [100 x [100 x i32]], align 16
  %j = alloca [100 x [100 x i32]], align 16
  %0 = bitcast ptr %z3 to ptr
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(40000) %0, i8 0, i64 40000, i1 false)
  %1 = bitcast ptr %j to ptr
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(40000) %1, i8 0, i64 40000, i1 false)
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %for.cond.cleanup5, %entry
  %indvars.iv111 = phi i64 [ %indvars.iv.next112, %for.cond.cleanup5 ], [ 4, %entry ]
  %cmp4103 = icmp ult i64 %indvars.iv111, 10
  br i1 %cmp4103, label %for.cond7.preheader.lr.ph, label %for.cond.cleanup5

for.cond7.preheader.lr.ph:                        ; preds = %for.cond3.preheader
  %2 = add nuw nsw i64 %indvars.iv111, 1
  br label %for.cond7.preheader

for.body.i74:                                     ; preds = %for.body.i74.preheader, %cond.end.i
  %indvars.iv.i72 = phi i64 [ %indvars.iv.next.i76, %cond.end.i ], [ 0, %for.body.i74.preheader ]
  %sum.015.i = phi i32 [ %add.i, %cond.end.i ], [ 0, %for.body.i74.preheader ]
  %ptridx.i75 = getelementptr inbounds [100 x [100 x i32]], ptr %z3, i64 0, i64 0, i64 %indvars.iv.i72
  %3 = load i32, ptr %ptridx.i75, align 4
  br label %cond.end.i

cond.end.i:                                       ; preds = %for.body.i74
  %add.i = add i32 %3, %sum.015.i
  %indvars.iv.next.i76 = add nuw nsw i64 %indvars.iv.i72, 1
  %exitcond.not.i77 = icmp eq i64 %indvars.iv.next.i76, 10000
  br i1 %exitcond.not.i77, label %checkSum.exit, label %for.body.i74

checkSum.exit:                                    ; preds = %cond.end.i
  %add.i.lcssa = phi i32 [ %add.i, %cond.end.i ]
  %call48 = tail call i32 (ptr, ...) @printf(ptr nonnull dereferenceable(1) @.str, i32 %add.i.lcssa)
  ret i32 0

for.cond7.preheader:                              ; preds = %for.cond.cleanup9, %for.cond7.preheader.lr.ph
  %indvars.iv113 = phi i64 [ %indvars.iv111, %for.cond7.preheader.lr.ph ], [ %indvars.iv.next114, %for.cond.cleanup9 ]
  ;%cmp8100 = icmp ult i64 %indvars.iv113, 9
  ;br i1 true, label %for.cond11.preheader.preheader, label %for.cond.cleanup9
  br label %for.cond11.preheader.preheader

for.cond11.preheader.preheader:                   ; preds = %for.cond7.preheader
  br label %for.cond11.preheader

for.cond.cleanup5.loopexit:                       ; preds = %for.cond.cleanup9
  br label %for.cond.cleanup5

for.cond.cleanup5:                                ; preds = %for.cond.cleanup5.loopexit, %for.cond3.preheader
  %indvars.iv.next112 = add nsw i64 %indvars.iv111, -1
  %cmp = icmp ugt i64 %indvars.iv111, 4
  br i1 %cmp, label %for.cond3.preheader, label %for.body.i74.preheader

for.body.i74.preheader:                           ; preds = %for.cond.cleanup5
  br label %for.body.i74

for.cond11.preheader:                             ; preds = %for.cond11.preheader.preheader
  %4 = add nsw i64 %indvars.iv113, -1
  br label %for.body14

for.cond.cleanup9.loopexit:                       ; preds = %for.cond.cleanup13
  br label %for.cond.cleanup9

for.cond.cleanup9:                                ; preds = %for.cond.cleanup9.loopexit, %for.cond7.preheader
  %indvars.iv.next114 = add nuw nsw i64 %indvars.iv113, 1
  %5 = add nsw i64 %indvars.iv113, -1
  %arrayidx31 = getelementptr inbounds [100 x [100 x i32]], ptr %z3, i64 0, i64 %5, i64 %2
  store i32 0, ptr %arrayidx31, align 4
  %exitcond124.not = icmp eq i64 %indvars.iv.next114, 10
  br i1 %exitcond124.not, label %for.cond.cleanup5.loopexit, label %for.cond7.preheader

for.cond.cleanup13:                               ; preds = %for.body14
  br label %for.cond.cleanup9.loopexit

for.body14:                                       ; preds = %for.body14, %for.cond11.preheader
  %indvars.iv = phi i64 [ 3, %for.cond11.preheader ], [ %indvars.iv.next, %for.body14 ]
  %inc = add i32 0, 1
  %arrayidx16 = getelementptr inbounds [100 x [100 x i32]], ptr %z3, i64 0, i64 %indvars.iv, i64 %4
  %add = add i32 0, %inc
  store i32 %add, ptr %arrayidx16, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %for.cond.cleanup13, label %for.body14
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

declare dso_local i32 @printf(ptr, ...) local_unnamed_addr #0

