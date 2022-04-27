; RUN: opt < %s -enable-new-pm=0 -loop-unswitch -S < %s 2>&1 | FileCheck %s
; RUN: opt < %s -simple-loop-unswitch -enable-nontrivial-unswitch -S < %s 2>&1 | FileCheck %s

; Verify that we skip unswitching for conditional branches (%cmp1) even for
; outermost loops in externally visible function when "pre_loopopt" and
; "intel-lang"="fortran" attributes are present.

; This is because after inlining they still may affect loopnest perfectness.

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; CHECK: @foo
; CHECK-NOT: for.body.outer.us
; CHECK: ret void

define void @foo(i64 %t) "pre_loopopt" "intel-lang"="fortran" {
entry:
  %cmp1 = icmp sgt i64 %t, 0
  br label %for.body.outer

for.body.outer:                                   ; preds = %for.inc.outer, %entry
  %indvars.iv.outer = phi i64 [ 0, %entry ], [ %indvars.iv.next.outer, %for.inc.outer ]
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.outer
  %indvars.iv = phi i64 [ 0, %for.body.outer ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx3, align 4
  %3 = trunc i64 %indvars.iv to i32
  %add4 = add nsw i32 %2, %3
  store i32 %add4, i32* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.inc.outer, label %for.body

for.inc.outer:                                    ; preds = %for.inc
  %indvars.iv.next.outer = add nuw nsw i64 %indvars.iv.outer, 1
  %exitcond.1 = icmp eq i64 %indvars.iv.next.outer, 20
  br i1 %exitcond.1, label %for.end, label %for.body.outer

for.end:                                          ; preds = %for.inc
  ret void
}

; CHECK: @bar
; CHECK: for.body.outer.us:
; CHECK: ret void

define void @bar(i64 %t) {
entry:
  %cmp1 = icmp sgt i64 %t, 0
  br label %for.body.outer

for.body.outer:                                   ; preds = %for.inc.outer, %entry
  %indvars.iv.outer = phi i64 [ 0, %entry ], [ %indvars.iv.next.outer, %for.inc.outer ]
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.outer
  %indvars.iv = phi i64 [ 0, %for.body.outer ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx3, align 4
  %3 = trunc i64 %indvars.iv to i32
  %add4 = add nsw i32 %2, %3
  store i32 %add4, i32* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.inc.outer, label %for.body

for.inc.outer:                                    ; preds = %for.inc
  %indvars.iv.next.outer = add nuw nsw i64 %indvars.iv.outer, 1
  %exitcond.1 = icmp eq i64 %indvars.iv.next.outer, 20
  br i1 %exitcond.1, label %for.end, label %for.body.outer

for.end:                                          ; preds = %for.inc
  ret void
}
