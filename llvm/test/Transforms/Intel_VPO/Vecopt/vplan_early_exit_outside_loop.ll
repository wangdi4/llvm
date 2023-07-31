; RUN: opt -S < %s -passes=vplan-vec -vpo-vplan-build-stress-test

; This test is used to check mergeLoopExits handling of early exit blocks falling outside
; of a loopnest. Here, mergeLoopExits re-wires the CFG to add a cascaded if block with
; a successor block 'for.end' that falls outside of a loop. This test should not compfail.
; It was previously assumed that the successor blocks of cascaded if blocks always fell
; within a loopnest.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local global i32 0, align 4

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @fn1() local_unnamed_addr #0 {
for.body.thread:
  store i32 0, ptr @a, align 4
  br label %if.then

for.cond:                                         ; preds = %for.body, %if.then
  %i.010 = phi i32 [ %inc, %for.body ], [ %i.09, %if.then ]
  %inc = add nuw nsw i32 %i.010, 1
  %cmp = icmp ult i32 %inc, 6
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %cond = icmp eq i32 %inc, 3
  br i1 %cond, label %if.then.loopexit, label %for.cond

if.then.loopexit:                                 ; preds = %for.body
  br label %if.then

if.then:                                          ; preds = %if.then.loopexit, %for.body.thread
  %i.09 = phi i32 [ 0, %for.body.thread ], [ 3, %if.then.loopexit ]
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
