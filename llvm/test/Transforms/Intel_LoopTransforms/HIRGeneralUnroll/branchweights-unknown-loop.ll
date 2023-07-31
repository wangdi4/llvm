; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-general-unroll,hir-cg" -aa-pipeline="basic-aa" -S  2>&1 < %s| FileCheck %s

; Check if bottem tests of unrolled unknown loops will have updated profile data.

; *** IR Dump After HIR General Unroll ***
; Function: foo
;
;         BEGIN REGION { modified }
;               + UNKNOWN LOOP i1 <nounroll>
;               |   <i1 = 0>
;               |   while.body:
;               |   %hir.de.ssa.copy0.out = %0;
;               |   %shl47 = 1  <<  trunc.i64.i5(%0);
;               |   %and49 = %in3  &&  %shl47;
;               |   %2 = (%and49 == 0) ? &((%next_node.0261)[0].2) : &((%next_node.0261)[0].3);
;               |   %3 = (%2)[0];
;               |   %4 = (%3)[0].1;
;               |   %0 = %4;
;               |   %next_node.0261 = &((%3)[0]);
;               |   if (%hir.de.ssa.copy0.out <=u %4)
;               |   {
;               |      goto loopexit.23;
;               |   } // intermediate bottom test in this form will appear 7 times.
;                   ....
;               |   if (%hir.de.ssa.copy0.out >u %4)
;               |   {
;               |      <i1 = i1 + 1>
;               |      goto while.body;
;               |   } // final bottom test. Notice the predicates are opposite to
;                     // those of intermedicate bottom tests.
;               + END LOOP

;CHECK: region.0:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INT_BOTTOM:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INT_BOTTOM]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INT_BOTTOM]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INT_BOTTOM]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INT_BOTTOM]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INT_BOTTOM]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_INT_BOTTOM]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_LAST_BOTTOM:[0-9]+]]


;CHECK-DAG: ![[PROF_INT_BOTTOM]] = !{!"branch_weights", i32 1, i32 12}
;CHECK-DAG: ![[PROF_LAST_BOTTOM]] = !{!"branch_weights", i32 12, i32 1}

; ModuleID = '<stdin>'
source_filename = "<stdin>"

%struct.trie_node = type { i64, i64, ptr, ptr }

define void @foo(i64 %in1, ptr %in2, i64 %in3) {
entry:
  br label %while.body

while.body:                                       ; preds = %while.body, %entry
  %0 = phi i64 [ %4, %while.body ], [ %in1, %entry ]
  %next_node.0261 = phi ptr [ %3, %while.body ], [ %in2, %entry ]
  %sh_prom46 = trunc i64 %0 to i32
  %1 = and i32 %sh_prom46, 31
  %shl47 = shl i32 1, %1
  %conv48 = sext i32 %shl47 to i64
  %and49 = and i64 %in3, %conv48
  %tobool50 = icmp eq i64 %and49, 0
  %llink54 = getelementptr inbounds %struct.trie_node, ptr %next_node.0261, i64 0, i32 2
  %rlink52 = getelementptr inbounds %struct.trie_node, ptr %next_node.0261, i64 0, i32 3
  %2 = select i1 %tobool50, ptr %llink54, ptr %rlink52
  %3 = load ptr, ptr %2, align 8
  %cmpbit41 = getelementptr inbounds %struct.trie_node, ptr %3, i64 0, i32 1
  %4 = load i64, ptr %cmpbit41, align 8
  %cmp42 = icmp ugt i64 %0, %4
  br i1 %cmp42, label %while.body, label %for.cond56.preheader.loopexit, !prof !0

for.cond56.preheader.loopexit:                    ; preds = %while.body
  ret void
}

!0 = !{!"branch_weights", i32 100, i32 10}
