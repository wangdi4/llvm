; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the two regions verifying that second region adds .lcssa1344 as the livein value when parsing %n.026.i instead of tracing it back to the first region (creating additional liveout value for the first region which isn't handled correctly). 
; CHECK: Region 2
; CHECK: LiveIns:
; CHECK-NOT: minLen.01105
; CHECK-SAME: %.lcssa1344(%.lcssa1344)

; Check that the condition %cmp4.i based on %n.026.i is parsed in terms of %.lcssa1344
; CHECK: if (%2 == i1 + %.lcssa1344)


@len = common global [6 x [258 x i8]] zeroinitializer, align 16
@code = common global [6 x [258 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sendMTFValues(i32 %add1, i1 %cmp41168, i64 %indvars.iv1208, i64 %t1) {
entry:
  br label %for.body346

for.body346:                                      ; preds = %entry, %for.body346
  %indvars.iv1204 = phi i64 [ %indvars.iv.next1205, %for.body346 ], [ 0, %entry ]
  %maxLen.01106 = phi i32 [ %conv351.maxLen.0, %for.body346 ], [ 0, %entry ]
  %minLen.01105 = phi i32 [ %1, %for.body346 ], [ 32, %entry ]
  %arrayidx350 = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i64 0, i64 %indvars.iv1208, i64 %indvars.iv1204
  %0 = load i8, i8* %arrayidx350, align 1
  %conv351 = zext i8 %0 to i32
  %cmp352 = icmp sgt i32 %conv351, %maxLen.01106
  %conv351.maxLen.0 = select i1 %cmp352, i32 %conv351, i32 %maxLen.01106
  %cmp366 = icmp slt i32 %conv351, %minLen.01105
  %1 = select i1 %cmp366, i32 %conv351, i32 %minLen.01105
  %indvars.iv.next1205 = add nuw nsw i64 %indvars.iv1204, 1
  %cmp344 = icmp slt i64 %indvars.iv.next1205, %t1
  br i1 %cmp344, label %for.body346, label %for.end377

for.end377:                                       ; preds = %for.body346
  %.lcssa1344 = phi i32 [ %1, %for.body346 ]
  %conv351.maxLen.0.lcssa = phi i32 [ %conv351.maxLen.0, %for.body346 ]
  br label %for.cond1.preheader.i.preheader

for.cond1.preheader.i.preheader:                  ; preds = %if.end385
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.cond1.preheader.i.preheader, %for.end.i
  %vec.027.i = phi i32 [ %shl.i, %for.end.i ], [ 0, %for.cond1.preheader.i.preheader ]
  %n.026.i = phi i32 [ %inc10.i, %for.end.i ], [ %.lcssa1344, %for.cond1.preheader.i.preheader ]
  br i1 %cmp41168, label %for.body3.i.preheader, label %for.end.i

for.body3.i.preheader:                            ; preds = %for.cond1.preheader.i
  br label %for.body3.i

for.body3.i:                                      ; preds = %for.body3.i.preheader, %for.inc.i
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.inc.i ], [ 0, %for.body3.i.preheader ]
  %vec.123.i = phi i32 [ %vec.2.i, %for.inc.i ], [ %vec.027.i, %for.body3.i.preheader ]
  %arrayidx.i = getelementptr inbounds [6 x [258 x i8]], [6 x [258 x i8]]* @len, i64 0, i64 %indvars.iv1208, i64 %indvars.iv.i
  %2 = load i8, i8* %arrayidx.i, align 1
  %conv.i = zext i8 %2 to i32
  %cmp4.i = icmp eq i32 %conv.i, %n.026.i
  br i1 %cmp4.i, label %if.then.i, label %for.inc.i

if.then.i:                                        ; preds = %for.body3.i
  %arrayidx7.i = getelementptr inbounds [6 x [258 x i32]], [6 x [258 x i32]]* @code, i64 0, i64 %indvars.iv1208, i64 %indvars.iv.i
  store i32 %vec.123.i, i32* %arrayidx7.i, align 4
  %inc.i = add nsw i32 %vec.123.i, 1
  br label %for.inc.i

for.inc.i:                                        ; preds = %if.then.i, %for.body3.i
  %vec.2.i = phi i32 [ %inc.i, %if.then.i ], [ %vec.123.i, %for.body3.i ]
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %lftr.wideiv1206 = trunc i64 %indvars.iv.next.i to i32
  %exitcond1207 = icmp eq i32 %lftr.wideiv1206, %add1
  br i1 %exitcond1207, label %for.end.i.loopexit, label %for.body3.i

for.end.i.loopexit:                               ; preds = %for.inc.i
  %vec.2.i.lcssa = phi i32 [ %vec.2.i, %for.inc.i ]
  br label %for.end.i

for.end.i:                                        ; preds = %for.end.i.loopexit, %for.cond1.preheader.i
  %vec.1.lcssa.i = phi i32 [ %vec.027.i, %for.cond1.preheader.i ], [ %vec.2.i.lcssa, %for.end.i.loopexit ]
  %shl.i = shl i32 %vec.1.lcssa.i, 1
  %inc10.i = add nsw i32 %n.026.i, 1
  %cmp.i = icmp slt i32 %n.026.i, %conv351.maxLen.0.lcssa
  br i1 %cmp.i, label %for.cond1.preheader.i, label %hbAssignCodes.exit.loopexit

hbAssignCodes.exit.loopexit:                      ; preds = %for.end.i
  ret void
}
