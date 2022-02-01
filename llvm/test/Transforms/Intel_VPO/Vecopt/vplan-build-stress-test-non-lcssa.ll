; Test that we can build VPlan in stress test mode for a loop not in LCSSA form.
; RUN: opt -S -enable-new-pm=0 -vplan-vec -vpo-vplan-build-stress-test -debug < %s 2>&1 | FileCheck %s
; RUN: opt -S -passes="lcssa,vplan-vec" -vpo-vplan-build-stress-test -debug < %s 2>&1 | FileCheck %s
; REQUIRES: asserts
; CHECK: Vectorization Plan{{.*}} Plain CFG
; CHECK-LABEL: @foo(
; CHECK: ret
%struct.coordinate = type { i32, i32 }
define i32 @foo(%struct.coordinate* nocapture %A, i32 %n) nounwind uwtable readonly ssp {
entry:
  %cmp4 = icmp sgt i32 %n, 0
  br i1 %cmp4, label %tmpblk, label %for.end2

tmpblk:
 br label %for.body

for.body:                                         ; preds = %tmpblk, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %tmpblk ]
  %sum.05 = phi i32 [ %add, %for.body ], [ 0, %tmpblk ]
  %x = getelementptr inbounds %struct.coordinate, %struct.coordinate* %A, i64 %indvars.iv, i32 0
  %0 = load i32, i32* %x, align 4
  %add = add nsw i32 %0, %sum.05
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add

for.end2:                                          ; preds = %entry
  ret i32 0
}
