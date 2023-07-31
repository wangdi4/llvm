; Test that we can build VPlan in stress test mode for an uncountable loop.
; RUN: opt -S -passes="vplan-vec" -vpo-vplan-build-stress-test -debug < %s 2>&1 | FileCheck %s
; REQUIRES: asserts
; CHECK: Vectorization Plan{{.*}} Plain CFG
; CHECK-LABEL: @foo(
; CHECK: ret
%struct.S1 = type { i32, ptr }

; Function Attrs: noinline nounwind uwtable
define void @foo(ptr %s1p) {
entry:
  %cmp4 = icmp eq ptr %s1p, null
  br i1 %cmp4, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %s1p.addr.05 = phi ptr [ %0, %for.body ], [ %s1p, %for.body.preheader ]
  %next = getelementptr inbounds %struct.S1, ptr %s1p.addr.05, i64 0, i32 1
  %0 = load ptr, ptr %next, align 8
  tail call void @free(ptr %s1p.addr.05) #2
  %cmp = icmp eq ptr %0, null
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind
declare dso_local void @free(ptr nocapture) local_unnamed_addr #1
