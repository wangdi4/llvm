; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-lmm -hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,hir-unroll-and-jam,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we are able to handle liveout temps which do not have uses inside the region like %dec.lcssa49.

; HIR-
; + DO i1 = 0, 50, 1   <DO_LOOP>
; |   %t2 = trunc.i64.i32(i1 + 2);
; |
; |   + DO i2 = 0, i1 + umax(-2, (-1 * %t2)) + 2, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; |   |   %dec.lcssa49 = 9;
; |   |
; |   |      %limm = (%so)[0][0];
; |   |   + DO i3 = 0, -1 * i1 + 6, 1   <DO_LOOP>  <MAX_TC_EST = 7>
; |   |   |   (%so)[0][-1 * i3 + 9] = %limm;
; |   |   + END LOOP
; |   |      %dec.lcssa49 = i1 + 2;
; |   + END LOOP
; + END LOOP

; Check for unroll by 8.

; CHECK: |   %tgu = (i1 + umax(-2, (-1 * %t2)) + 3)/u8;
; CHECK: |   + DO i2 = 0, %tgu + -1, 1


define void @foo() {
entry:
  %so = alloca [100 x i32], align 16
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %so, i64 0, i64 0
  br label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %entry, %for.inc25
  %indvars.iv58 = phi i64 [ 2, %entry ], [ %indvars.iv.next59, %for.inc25 ]
  %indvars.iv54 = phi i32 [ 6, %entry ], [ %indvars.iv.next55, %for.inc25 ]
  %t1 = sub i32 8, %indvars.iv54
  %cmp541 = icmp ult i64 %indvars.iv58, 9
  %t2 = trunc i64 %indvars.iv58 to i32
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.lr.ph, %for.inc22
  %storemerge3748 = phi i32 [ %t2, %for.cond4.preheader.lr.ph ], [ %dec23, %for.inc22 ]
  br i1 %cmp541, label %for.body6.preheader, label %for.inc22

for.body6.preheader:                              ; preds = %for.cond4.preheader
  br label %for.body6

for.body6:                                        ; preds = %for.body6.preheader, %for.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body6 ], [ 9, %for.body6.preheader ]
  %t3 = load i32, i32* %arrayidx, align 16
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %so, i64 0, i64 %indvars.iv
  store i32 %t3, i32* %arrayidx7, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp5 = icmp ugt i64 %indvars.iv.next, %indvars.iv58
  br i1 %cmp5, label %for.body6, label %for.inc22.loopexit

for.inc22.loopexit:                               ; preds = %for.body6
  br label %for.inc22

for.inc22:                                        ; preds = %for.inc22.loopexit, %for.cond4.preheader
  %dec.lcssa49 = phi i32 [ 9, %for.cond4.preheader ], [ %t1, %for.inc22.loopexit ]
  %dec23 = add nsw i32 %storemerge3748, -1
  %cmp2 = icmp ugt i32 %dec23, 1
  br i1 %cmp2, label %for.cond4.preheader, label %for.inc25

for.inc25:                                        ; preds = %for.inc22
  %dec.lcssa49.lcssa = phi i32 [ %dec.lcssa49, %for.inc22 ]
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %indvars.iv.next55 = add nsw i32 %indvars.iv54, -1
  %exitcond = icmp eq i64 %indvars.iv.next59, 53
  br i1 %exitcond, label %for.end27, label %for.cond4.preheader.lr.ph

for.end27:                                        ; preds = %for.inc25
  %.lcssa = phi i32 [ %t2, %for.inc25 ]
  %dec.lcssa49.lcssa.lcssa = phi i32 [ %dec.lcssa49.lcssa, %for.inc25 ]
  ret void
}

