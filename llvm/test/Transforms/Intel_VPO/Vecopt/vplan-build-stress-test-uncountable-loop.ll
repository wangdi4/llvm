; Test that we can build VPlan in stress test mode for an uncountable loop.
; RUN: opt -S -VPlanDriver -vpo-vplan-build-stress-test -debug < %s 2>&1 | FileCheck %s
; RUN: opt -S -passes="vplan-driver" -vpo-vplan-build-stress-test -debug < %s 2>&1 | FileCheck %s
; REQUIRES: asserts
; CHECK: Vectorization Plan{{.*}} After building HCFG
; CHECK-LABEL: @foo(
; CHECK: ret
%struct.S1 = type { i32, %struct.S1* }

; Function Attrs: noinline nounwind uwtable
define void @foo(%struct.S1* %s1p) {
entry:
  %cmp4 = icmp eq %struct.S1* %s1p, null
  br i1 %cmp4, label %for.end, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %s1p.addr.05 = phi %struct.S1* [ %0, %for.body ], [ %s1p, %for.body.preheader ]
  %next = getelementptr inbounds %struct.S1, %struct.S1* %s1p.addr.05, i64 0, i32 1
  %0 = load %struct.S1*, %struct.S1** %next, align 8
  %1 = bitcast %struct.S1* %s1p.addr.05 to i8*
  tail call void @free(i8* %1) #2
  %cmp = icmp eq %struct.S1* %0, null
  br i1 %cmp, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: nounwind
declare dso_local void @free(i8* nocapture) local_unnamed_addr #1
