; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output <%s 2>&1 | FileCheck %s

; Test checks that we do not delinearize CE with non-trivial denominator.

;            BEGIN REGION { }
;                  + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;                  |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
;                  |   |   %0 = (%a)[(%n * i1 + i2 + 1)/u32];
;                  |   |   %s.040 = %s.040  +  %0;
;                  |   |   (%a)[(%n * i1 + i2 + 1)/u32] = i1 + i2;
;                  |   + END LOOP
;                  |
;                  |   %s.040.out = %s.040;
;                  + END LOOP
;            END REGION

; CHECK-DAG: 10:15 (%a)[(%n * i1 + i2 + 1)/u32] --> (%a)[(%n * i1 + i2 + 1)/u32] ANTI (* *) (? ?)
; CHECK-DAG: 15:10 (%a)[(%n * i1 + i2 + 1)/u32] --> (%a)[(%n * i1 + i2 + 1)/u32] FLOW (* *) (? ?)
; CHECK-DAG: 15:15 (%a)[(%n * i1 + i2 + 1)/u32] --> (%a)[(%n * i1 + i2 + 1)/u32] OUTPUT (* *) (? ?)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local i32 @foo(ptr nocapture noundef %a, i64 noundef %n, i32 noundef %m) local_unnamed_addr {
entry:
  %cmp38.not = icmp eq i64 %n, 0
  br i1 %cmp38.not, label %for.end15, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc13
  %s.040 = phi i64 [ %add5.lcssa, %for.inc13 ], [ 0, %for.cond1.preheader.preheader ]
  %i.039 = phi i64 [ %inc14, %for.inc13 ], [ 0, %for.cond1.preheader.preheader ]
  %mul = mul i64 %i.039, %n
  %add = add i64 %mul, 1
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %s.137 = phi i64 [ %s.040, %for.cond1.preheader ], [ %add5, %for.body3 ]
  %j.036 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %add4 = add i64 %add, %j.036
  %div35 = lshr i64 %add4, 5
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %div35
  %0 = load i32, ptr %arrayidx, align 4
  %conv42 = zext i32 %0 to i64
  %add5 = add i64 %s.137, %conv42
  %add6 = add i64 %j.036, %i.039
  %conv7 = trunc i64 %add6 to i32
  store i32 %conv7, ptr %arrayidx, align 4
  %inc = add nuw i64 %j.036, 1
  %exitcond.not = icmp eq i64 %inc, %n
  br i1 %exitcond.not, label %for.inc13, label %for.body3

for.inc13:                                        ; preds = %for.body3
  %add5.lcssa = phi i64 [ %add5, %for.body3 ]
  %inc14 = add nuw i64 %i.039, 1
  %exitcond41.not = icmp eq i64 %inc14, %n
  br i1 %exitcond41.not, label %for.end15.loopexit, label %for.cond1.preheader

for.end15.loopexit:                               ; preds = %for.inc13
  %add5.lcssa.lcssa = phi i64 [ %add5.lcssa, %for.inc13 ]
  %1 = trunc i64 %add5.lcssa.lcssa to i32
  br label %for.end15

for.end15:                                        ; preds = %for.end15.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %1, %for.end15.loopexit ]
  ret i32 %s.0.lcssa 
}

