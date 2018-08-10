; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that we support "llvm.loop.vectorize.enable" metadata on the loop.

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP> <vectorize>
; CHECK: |   if (i1 >u 5)
; CHECK: |   {
; CHECK: |      %0 = (@a)[0][i1];
; CHECK: |      (@a)[0][i1] = %0 + 3;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %1 = (@b)[0][i1];
; CHECK: |      %s.013 = %1  +  %s.013;
; CHECK: |   }
; CHECK: |   %s.013.out = %s.013;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo(i32 %n) local_unnamed_addr {
entry:
  %cmp11 = icmp sgt i32 %n, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %s.013 = phi i32 [ 0, %for.body.preheader ], [ %s.1, %for.inc ]
  %cmp1 = icmp ugt i64 %indvars.iv, 5
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, 3
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %add4 = add nsw i32 %1, %s.013
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %s.1 = phi i32 [ %s.013, %if.then ], [ %add4, %if.else ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop !7

for.end.loopexit:                                 ; preds = %for.inc
  %s.1.lcssa = phi i32 [ %s.1, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %s.1.lcssa, %for.end.loopexit ]
  ret i32 %s.0.lcssa
}

!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.vectorize.enable", i1 true}
