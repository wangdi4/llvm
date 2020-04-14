; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Check that we allow an unknown loop without any IV node.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   for.body:
; CHECK: |   %0 = (%p)[0];
; CHECK: |   if (%0 != 5)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto for.body;
; CHECK: |   }
; CHECK: + END LOOP

define i32 @main(i32* %p)  {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %0 = load i32, i32* %p, align 4
  %exitcond = icmp eq i32 %0, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}
