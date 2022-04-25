; The test checks that getGraph() returns only relevant edges.

; The option -hir-dd-analysis-dump-nodes=-1 forces the HIRDDAnalysis pass to call getGraph() for each HLNode in the runOnFunction().
; DD Graphs are printed to the output.

; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-dump-nodes=-1 < %s 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-dump-nodes=-1 2>&1 | FileCheck %s

; Source:
; void foo(char * a, char * b, char * c, int n) {
;   int i;
;   int j;
;   for (i=0;i<n;++i) {
;     for (j=0;j<n;++j) {
;       b[i] = a[0];
;     }
;     for (j=0;j<n;++j) {
;       c[i] = a[0];
;     }
;   }
; }

; HIR:
;         BEGIN REGION { }
;            + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
;            |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
;            |   |   %0 = (%a)[0];
;            |   |   (%b)[i1] = %0;
;            |   + END LOOP
;            |
;            |
;            |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
;            |   |   %1 = (%a)[0];
;            |   |   (%c)[i1] = %1;
;            |   + END LOOP
;            + END LOOP
;         END REGION

; The DDG will be printed for each HLNode in the following order: Region, Outer Loop, 1st Inner Loop, 2nd Inner Loop
; We have to check that there are no edges between the inner loops in graphs for inner loops.
;
; First we skip graph for the Region. In this case it will be the same as for the Outer Loop.
; CHECK: Graph

; Now we check the graph for the Outer Loop.

; CHECK: Graph
; CHECK-DAG: [[N6:.*]]:[[N7:.*]] %0 --> %0
; CHECK-DAG: [[N19:.*]]:[[N20:.*]] %1 --> %1
; CHECK-DAG: [[N7]]:[[N6]] (%b)[i1] --> (%a)[0]
; CHECK-DAG: [[N20]]:[[N6]] (%c)[i1] --> (%a)[0]
; CHECK-DAG: [[N6]]:[[N7]] (%a)[0] --> (%b)[i1]
; CHECK-DAG: [[N6]]:[[N20]] (%a)[0] --> (%c)[i1]
; CHECK-DAG: [[N7]]:[[N7]] (%b)[i1] --> (%b)[i1]
; CHECK-DAG: [[N19]]:[[N7]] (%a)[0] --> (%b)[i1]
; CHECK-DAG: [[N20]]:[[N7]] (%c)[i1] --> (%b)[i1]
; CHECK-DAG: [[N7]]:[[N19]] (%b)[i1] --> (%a)[0]
; CHECK-DAG: [[N7]]:[[N20]] (%b)[i1] --> (%c)[i1]
; CHECK-DAG: [[N20]]:[[N19]] (%c)[i1] --> (%a)[0]
; CHECK-DAG: [[N19]]:[[N20]] (%a)[0] --> (%c)[i1]
; CHECK-DAG: [[N20]]:[[N20]] (%c)[i1] --> (%c)[i1]
; CHECK-NOT: -->

; And the graphs for the Inner Loops. Note that there should be no edges outside of the loop.

; CHECK: Graph
; CHECK-DAG: [[N6]]:[[N7]] %0 --> %0
; CHECK-DAG: [[N7]]:[[N6]] (%b)[i1] --> (%a)[0]
; CHECK-DAG: [[N6]]:[[N7]] (%a)[0] --> (%b)[i1]
; CHECK-DAG: [[N7]]:[[N7]] (%b)[i1] --> (%b)[i1]
; CHECK-NOT: -->

; CHECK: Graph
; CHECK-DAG: [[N19]]:[[N20]] %1 --> %1
; CHECK-DAG: [[N20]]:[[N19]] (%c)[i1] --> (%a)[0]
; CHECK-DAG: [[N19]]:[[N20]] (%a)[0] --> (%c)[i1]
; CHECK-DAG: [[N20]]:[[N20]] (%c)[i1] --> (%c)[i1]
; CHECK-NOT: -->

;Module Before HIR; ModuleID = 'lmm.c'
source_filename = "lmm.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i8* nocapture readonly %a, i8* nocapture %b, i8* nocapture %c, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp32 = icmp sgt i32 %n, 0
  br i1 %cmp32, label %for.body3.lr.ph.preheader, label %for.end16

for.body3.lr.ph.preheader:                        ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc14
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc14 ], [ 0, %for.body3.lr.ph.preheader ]
  %arrayidx4 = getelementptr inbounds i8, i8* %b, i64 %indvars.iv
  br label %for.body3

for.body7.lr.ph:                                  ; preds = %for.body3
  %arrayidx10 = getelementptr inbounds i8, i8* %c, i64 %indvars.iv
  br label %for.body7

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %j.029 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.body3 ]
  %0 = load i8, i8* %a, align 1
  store i8 %0, i8* %arrayidx4, align 1
  %inc = add nuw nsw i32 %j.029, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.body7.lr.ph, label %for.body3

for.body7:                                        ; preds = %for.body7, %for.body7.lr.ph
  %j.131 = phi i32 [ 0, %for.body7.lr.ph ], [ %inc12, %for.body7 ]
  %1 = load i8, i8* %a, align 1
  store i8 %1, i8* %arrayidx10, align 1
  %inc12 = add nuw nsw i32 %j.131, 1
  %exitcond34 = icmp eq i32 %inc12, %n
  br i1 %exitcond34, label %for.inc14, label %for.body7

for.inc14:                                        ; preds = %for.body7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond35 = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond35, label %for.end16.loopexit, label %for.body3.lr.ph

for.end16.loopexit:                               ; preds = %for.inc14
  br label %for.end16

for.end16:                                        ; preds = %for.end16.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


