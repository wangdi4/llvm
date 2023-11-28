; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec" -print-before=hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec -debug-only=hir-loop-distribute -disable-output < %s 2>&1 | FileCheck %s

; Verify that loop distribution does not try to distribute this loop and set the
; modified flag. It deduced control nodes (ifs) as vectorizable and the rest of
; the instructions forming a PiBlock as not vectorizable. It then tried to
; distribute them but eventually produced identical loop as the PiBlock
; containing instructions was control dependant on the ifs.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, %n + -2, 1   <DO_LOOP>
; CHECK: |   %t222 = (%t220)[0];
; CHECK: |   if (i1 + 1 >= %v1)
; CHECK: |   {
; CHECK: |      if (i1 + 1 <= %v2)
; CHECK: |      {
; CHECK: |         %t229 = (%t222)[0].2  |  1;
; CHECK: |         (%t222)[0].2 = %t229;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   %t220 = &((%t222)[0].4);
; CHECK: + END LOOP

; CHECK: LOOP DISTRIBUTION: Found no valid distribution points

; CHECK: Dump After
; CHECK-NOT: modified

; CHECK: + DO i1 = 0, %n + -2, 1   <DO_LOOP>
; CHECK: |   %t222 = (%t220)[0];
; CHECK: |   if (i1 + 1 >= %v1)
; CHECK: |   {
; CHECK: |      if (i1 + 1 <= %v2)
; CHECK: |      {
; CHECK: |         %t229 = (%t222)[0].2  |  1;
; CHECK: |         (%t222)[0].2 = %t229;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   %t220 = &((%t222)[0].4);
; CHECK: + END LOOP


%struct._ZTS8strand_t.strand_t = type { ptr, i32, i32, ptr, ptr, i32, i32, ptr }

define void @foo(ptr %p, i32 %n, i32 %v1, i32 %v2) {
entry:
  br label %loop

loop:                                              ; preds = %latch, %entry
  %t220 = phi ptr [ %p, %entry ], [ %t232, %latch ]
  %iv = phi i32 [ 1, %entry ], [ %iv.inc, %latch ]
  %t222 = load ptr, ptr %t220, align 8
  %t223 = icmp slt i32 %iv, %v1
  br i1 %t223, label %latch, label %t224

t224:                                              ; preds = %loop
  %t225 = icmp sgt i32 %iv, %v2
  br i1 %t225, label %latch, label %t226

t226:                                              ; preds = %t224
  %t227 = getelementptr inbounds %struct._ZTS8strand_t.strand_t, ptr %t222, i64 0, i32 2
  %t228 = load i32, ptr %t227, align 4
  %t229 = or i32 %t228, 1
  store i32 %t229, ptr %t227, align 4
  br label %latch

latch:                                              ; preds = %t226, %t224, %loop
  %iv.inc = add nuw nsw i32 %iv, 1
  %t232 = getelementptr inbounds %struct._ZTS8strand_t.strand_t, ptr %t222, i64 0, i32 4
  %t233 = icmp eq i32 %iv.inc, %n
  br i1 %t233, label %exit, label %loop

exit:
  ret void
}
