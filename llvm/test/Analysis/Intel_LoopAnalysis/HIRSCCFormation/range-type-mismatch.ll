; RUN: opt < %s -enable-new-pm=0 -analyze -hir-scc-formation | FileCheck %s

; We use the --allow-empty flag with FileCheck for the new-format opt because:
;
; - new-format opt output is empty for this test, (old-format opt emits just one
;       line: Printing analysis 'HIR SCC Formation' for function...).
; - The check consists of 'CHECK-NOT' only, and has no 'CHECK' lines.
;
; TODO: If the lit-test is modified, and new-format opt is no longer empty,
;     please make sure to remove the --allow-empty flag, and this comment.
;
; RUN: opt %s -passes="print<hir-scc-formation>" -disable-output 2>&1 | FileCheck --allow-empty %s

; Verify that the test compiles successfully.
; It was failing during range comparison of %bf.lshr87140 and %cmp82.not139
; which have mismatched types.

; CHECK-NOT: SCC1


define void @foo(i1 %call32) {
entry:
  br label %land.rhs23

land.rhs23:                                       ; preds = %while.body, %entry
  %bf.lshr87140 = phi i64 [ 64, %entry ], [ %bf.lshr87, %while.body ]
  %cmp82.not139 = phi i1 [ false, %entry ], [ %cmp82.not, %while.body ]
  %Ty.sroa.0.0137 = phi i64 [ 256, %entry ], [ %and.70i57, %while.body ]
  br i1 %call32, label %if.end39.loopexit, label %while.body

while.body:                                       ; preds = %land.rhs23
  %conv.71 = and i64 %bf.lshr87140, 65535
  %mul.72 = select i1 %cmp82.not139, i64 %conv.71, i64 1
  %t9 = mul nuw nsw i64 %bf.lshr87140, %mul.72
  %add.i75 = add nuw nsw i64 %t9, 7
  %t10 = lshr i64 %add.i75, 1
  %and.70i57 = and i64 %t10, 2147483644
  %cmp82.not = icmp eq i64 %and.70i57, 0
  %bf.lshr87 = lshr exact i64 %and.70i57, 2
  %cmp.i80 = icmp ult i64 %bf.lshr87, %t10
  br i1 %cmp.i80, label %land.rhs23, label %if.end39.loopexit

if.end39.loopexit:                                ; preds = %while.body, %land.rhs23
  %Ty.sroa.0.1.ph = phi i64 [ %Ty.sroa.0.0137, %land.rhs23 ], [ %and.70i57, %while.body ]
  ret void
}
