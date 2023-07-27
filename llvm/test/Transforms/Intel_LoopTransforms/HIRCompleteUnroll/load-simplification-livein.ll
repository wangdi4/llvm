; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -hir-complete-unroll-force-constprop -hir-details -disable-output 2>&1 < %s | FileCheck %s

; Verify that @const_glob1 and @const_glob2 which occur in HIR by simplification
; of load (@struct_glob)[0][i2].0 is marked as livein to i1 loop.

; CHECK: Dump Before

; CHECK: LiveIn symbases: [[ORIGSB:[0-9]+]]
; CHECK: + DO i64 i1 = 0, 399, 1   <DO_LOOP>
; CHECK: |   + DO i64 i2 = 0, 1, 1   <DO_LOOP>
; CHECK: |   |   %char_ld = (@struct_glob)[0][i2].0;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: LiveIn symbases: [[ORIGSB:[0-9]+]], [[GLOB1SB:[0-9]+]], [[GLOB2SB:[0-9]+]]

; CHECK: + DO i64 i1 = 0, 399, 1   <DO_LOOP>
; CHECK: |   %char_ld = &((@const_glob1)[0]);
; CHECK:     <BLOB> LINEAR ptr @const_glob1 {sb:[[GLOB1SB]]}

; CHECK: |   %char_ld = &((@const_glob2)[0]);
; CHECK:     <BLOB> LINEAR ptr @const_glob2 {sb:[[GLOB2SB]]}

; CHECK: + END LOOP


%stuct.func.ptr = type { ptr, i32 }

@const_glob1 = internal unnamed_addr constant i8 0
@const_glob2 = internal unnamed_addr constant i8 0
@struct_glob = internal unnamed_addr constant [2 x %stuct.func.ptr] [%stuct.func.ptr { ptr @const_glob1, i32 0 }, %stuct.func.ptr { ptr @const_glob2, i32 1 }]

define void @foo() {
entry:
  br label %outer.loop

outer.loop:
  %iv.outer = phi i64 [ 0, %entry], [ %iv.outer.inc, %outer.latch]
  br label %loop

loop:
  %iv = phi i64 [ 0, %outer.loop], [ %iv.inc, %loop]
  %gep1 = getelementptr [2 x %stuct.func.ptr], ptr @struct_glob, i64 0, i64 %iv, i32 0
  %char_ld = load ptr, ptr %gep1
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 2
  br i1 %cmp, label %outer.latch, label %loop

outer.latch:
  %char_ld.lcssa = phi ptr [ %char_ld, %loop ]
  %iv.outer.inc = add i64 %iv.outer, 1
  %cmp1 = icmp eq i64 %iv.outer.inc, 400
  br i1 %cmp1, label %exit, label %outer.loop

exit:
  %ld.lcssa = phi ptr [ %char_ld.lcssa, %outer.latch ]
  ret void
}

