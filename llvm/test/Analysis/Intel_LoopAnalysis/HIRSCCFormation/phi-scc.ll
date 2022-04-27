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

; Check that we don't from any SCC for %v_gp.020 due to live range overlap.
; The SCC nodes %v_gp.020 and %cond8 are both live together at %xor.

; CHECK-NOT: SCC1

; Function Attrs: uwtable
define i32 @main() {
entry:
  br label %for.body

for.body:                                         ; preds = %cond.end.7, %entry
  %v_gp.020 = phi i32 [ 40, %entry ], [ %xor, %cond.end.7 ]
  %v_r.019 = phi i32 [ 0, %entry ], [ %inc, %cond.end.7 ]
  %conv = trunc i32 %v_gp.020 to i8
  %tobool = icmp eq i8 %conv, 0
  br i1 %tobool, label %cond.end.7, label %cond.end

cond.end:                                         ; preds = %for.body
  %conv3 = and i32 %v_gp.020, 255
  %rem = srem i32 -38, %conv3
  %rem5 = urem i32 %rem, %conv3
  br label %cond.end.7

cond.end.7:                                       ; preds = %for.body, %cond.end
  %cond8 = phi i32 [ %rem5, %cond.end ], [ 0, %for.body ]
  %xor = xor i32 %cond8, %v_gp.020
  %inc = add nuw nsw i32 %v_r.019, 1
  %exitcond = icmp eq i32 %inc, 56
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  ret i32 0
}

