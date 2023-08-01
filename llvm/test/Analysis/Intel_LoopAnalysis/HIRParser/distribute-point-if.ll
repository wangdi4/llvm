; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we mark HLIf as the distribute point.

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK: |   %0 = (@b)[0][i1];
; CHECK: |   %s.014 = %0  +  %s.014;
; CHECK: |   if (%n > 5) <distribute_point>
; CHECK: |   {
; CHECK: |      %2 = (@a)[0][i1];
; CHECK: |      (@a)[0][i1] = %2 + 3;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@a = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 %n) local_unnamed_addr {
entry:
  %cmp12 = icmp sgt i32 %n, 0
  br i1 %cmp12, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %cmp1 = icmp sgt i32 %n, 5
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %if.end ]
  %s.014 = phi i32 [ 0, %for.body.lr.ph ], [ %add, %if.end ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @b, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, %s.014
  %1 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i32], ptr @a, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx3, align 4
  %add4 = add nsw i32 %2, 3
  store i32 %add4, ptr %arrayidx3, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  %add.lcssa = phi i32 [ %add, %if.end ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %add.lcssa, %for.end.loopexit ]
  ret i32 %s.0.lcssa
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

