; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-cg -force-hir-cg -S 2>&1 < %s | FileCheck %s

; Check that CG adds an unreachable instruction in the merge bblock of the top 'if' of the function level region to maintain sanity of CFG.

; HIR-

; <1>             if (undef #UNDEF# undef)
; <1>             {
; <7>                unreachable ;
; <1>             }
; <1>             else
; <1>             {
; <5>                ret ;
; <1>             }


; CHECK: region.{{[0-9]}}:
; CHECK-NEXT: br i1 undef

; CHECK: then.{{[0-9]}}:
; CHECK-NEXT: unreachable

; CHECK: else.{{[0-9]}}:
; CHECK: ret void

; CHECK: ifmerge.{{[0-9]}}:
; CHECK: unreachable


define void @foo() {
entry:
  br i1 undef, label %unreachable, label %exit

unreachable:                                      ; preds = %entry
  unreachable

exit:
  ret void;
}
