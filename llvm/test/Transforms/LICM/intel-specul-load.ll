; RUN: opt -aa-pipeline="basic-aa,tbaa" -passes=licm -S %s | FileCheck %s

; For the code below, p->z is loaded conditionally.
; LICM can hoist the load p->z, based on the following:

; There exists a load p->y which is unconditional, and it is
; close in distance to p->z (<128b)
; The structure "thing" has a total size that is large (>512b). This is for
; performance reasons to avoid speculation of data that would fit in the cache
; with good locality.

;struct thing {
;  int x;
;  char pad[512];
;  int y;
;  int z;
;};

;int speculate(thing *p, bool cond) {
;  int sum = 0;
;  for (int i = 0; i < 100; i++) {
;    sum += p->y;
;    if (cond) {
;      sum += p->z;
;    }
;  }
;  return sum;
;}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.thing = type { i32, [512 x i8], i32, i32 }
%struct.thing_small = type { i32, i32, i32 }

; CHECK-LABEL: speculate
; CHECK: entry:
; CHECK: load{{.*}} %y
; CHECK: load{{.*}} %z
; CHECK: for.cond

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @speculate(ptr noundef %p, i1 noundef zeroext %cond) local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %sum.0 = phi i32 [ 0, %entry ], [ %sum.1, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret i32 %sum.0

for.body:                                         ; preds = %for.cond
  %y = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 2, !intel-tbaa !3
  %0 = load i32, ptr %y, align 4, !tbaa !3
  %add = add nsw i32 %sum.0, %0
  br i1 %cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %z = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 3, !intel-tbaa !9
  %1 = load i32, ptr %z, align 4, !tbaa !9
  %add1 = add nsw i32 %add, %1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %sum.1 = phi i32 [ %add1, %if.then ], [ %add, %for.body ]
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !10
}

; p->x cannot be used to prove safety of p-z, as it is too far away.
; CHECK-LABEL: dist_too_large
; CHECK: entry:
; CHECK: load{{.*}} %x
; CHECK-NOT: load{{.*}} %z
; CHECK: for.cond

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @dist_too_large(ptr noundef %p, i1 noundef zeroext %cond) local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %sum.0 = phi i32 [ 0, %entry ], [ %sum.1, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret i32 %sum.0

for.body:                                         ; preds = %for.cond
  %x = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 0, !intel-tbaa !12
  %0 = load i32, ptr %x, align 4, !tbaa !12
  %add = add nsw i32 %sum.0, %0
  br i1 %cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %z = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 3, !intel-tbaa !9
  %1 = load i32, ptr %z, align 4, !tbaa !9
  %add1 = add nsw i32 %add, %1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %sum.1 = phi i32 [ %add1, %if.then ], [ %add, %for.body ]
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !13
}

; This structure is too small, and speculation may not improve performance.

; CHECK-LABEL: struct_too_small
; CHECK: entry:
; CHECK: load{{.*}} %y
; CHECK-NOT: load{{.*}} %z
; CHECK: for.cond

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @struct_too_small(ptr noundef %p, i1 noundef zeroext %cond) local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %sum.0 = phi i32 [ 0, %entry ], [ %sum.1, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret i32 %sum.0

for.body:                                         ; preds = %for.cond
  %y = getelementptr inbounds %struct.thing_small, ptr %p, i64 0, i32 1, !intel-tbaa !14
  %0 = load i32, ptr %y, align 4, !tbaa !14
  %add = add nsw i32 %sum.0, %0
  br i1 %cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %z = getelementptr inbounds %struct.thing_small, ptr %p, i64 0, i32 2, !intel-tbaa !16
  %1 = load i32, ptr %z, align 4, !tbaa !16
  %add1 = add nsw i32 %add, %1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %sum.1 = phi i32 [ %add1, %if.then ], [ %add, %for.body ]
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !17
}

; Both fields are guarded; neither can be speculated.
; if (cond) {
;    ... p->y
;    if (cond2)
;       ... p->z
; etc.

; CHECK-LABEL: twocond
; CHECK: br i1 %cond
; CHECK: for.body
; CHECK: load i32, ptr %y
; CHECK: br i1 %cond2
; CHECK: load i32, ptr %z
define dso_local noundef i32 @twocond(ptr nocapture noundef readonly %p, i1 noundef zeroext %cond, i1 noundef zeroext %cond2) local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %sum.0 = phi i32 [ 0, %entry ], [ %sum.1, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %sum.0.lcssa = phi i32 [ %sum.0, %for.cond ]
  ret i32 %sum.0.lcssa

for.body:                                         ; preds = %for.cond
  br i1 %cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %y = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 2, !intel-tbaa !3
  %0 = load i32, ptr %y, align 4, !tbaa !3
  %add = add nsw i32 %0, %sum.0
  br i1 %cond2, label %if.then3, label %for.inc

if.then3:                                         ; preds = %if.then
  %z = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 3, !intel-tbaa !9
  %1 = load i32, ptr %z, align 4, !tbaa !9
  %add4 = add nsw i32 %1, %add
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then3, %if.then
  %sum.1 = phi i32 [ %add4, %if.then3 ], [ %add, %if.then ], [ %sum.0, %for.body ]
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !10
}

; Both fields are guarded; neither can be speculated.
; if (cond) {
;    ... p->y
;    ... p->z
; etc.

; CHECK-LABEL: samecond
; CHECK: for.body
; CHECK: br i1 %cond
; CHECK: load i32, ptr %y
; CHECK: load i32, ptr %z
define dso_local noundef i32 @samecond(ptr nocapture noundef readonly %p, i1 noundef zeroext %cond) local_unnamed_addr #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %sum.0 = phi i32 [ 0, %entry ], [ %sum.1, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %sum.0.lcssa = phi i32 [ %sum.0, %for.cond ]
  ret i32 %sum.0.lcssa

for.body:                                         ; preds = %for.cond
  br i1 %cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %y = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 2, !intel-tbaa !3
  %0 = load i32, ptr %y, align 4, !tbaa !3
  %add = add nsw i32 %sum.0, %0
  %z = getelementptr inbounds %struct.thing, ptr %p, i64 0, i32 3, !intel-tbaa !9
  %1 = load i32, ptr %z, align 4, !tbaa !9
  %add1 = add nsw i32 %add, %1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %sum.1 = phi i32 [ %add1, %if.then ], [ %sum.0, %for.body ]
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !10
}

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 516}
!4 = !{!"struct@_ZTS5thing", !5, i64 0, !8, i64 4, !5, i64 516, !5, i64 520}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!"array@_ZTSA512_c", !6, i64 0}
!9 = !{!4, !5, i64 520}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = !{!4, !5, i64 0}
!13 = distinct !{!13, !11}
!14 = !{!15, !5, i64 4}
!15 = !{!"struct@_ZTS11thing_small", !5, i64 0, !5, i64 4, !5, i64 8}
!16 = !{!15, !5, i64 8}
!17 = distinct !{!17, !11}
