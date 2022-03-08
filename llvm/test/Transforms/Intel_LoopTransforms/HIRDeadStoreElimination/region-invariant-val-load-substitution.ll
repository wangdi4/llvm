; RUN: opt -hir-ssa-deconstruction -hir-create-function-level-region -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s

; Verify that we eliminate the first store to (%A)[0] by forward
; substituting the intermediate load with its RHS which is region
; invariant.

; CHECK: Before

; CHECK: (%A)[0] = %t;
; CHECK: if (%cond != 0)
; CHECK: {
; CHECK:    (%B)[0] = 1;
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    (%B)[0] = 2;
; CHECK: }
; CHECK: %ld = (%A)[0];
; CHECK: (%A)[0] = 5;


; CHECK: After

; CHECK: modified

; CHECK: if (%cond != 0)
; CHECK: {
; CHECK:    (%B)[0] = 1;
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    (%B)[0] = 2;
; CHECK: }
; CHECK: %ld = %t;
; CHECK: (%A)[0] = 5;


define void @foo(i32* noalias %A, i32* noalias %B, i32 %t, i1 %cond) {
entry:
  br label %bb

bb:                                         ; preds = %entry
  store i32 %t, i32* %A, align 4
  br i1 %cond, label %then, label %else

then:
  store i32 1, i32* %B, align 4
  br label %end

else:
  store i32 2, i32* %B, align 4
  br label %end
 
end:
  %ld = load i32, i32* %A, align 4
  store i32 5, i32* %A, align 4
  ret void
}

