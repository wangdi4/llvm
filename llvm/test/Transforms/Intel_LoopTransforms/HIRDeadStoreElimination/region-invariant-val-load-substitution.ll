; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we eliminate the first store to (%A)[0] by replacing it and the
; intermediate load with %temp and then propagate the temp into the use.

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
; CHECK: ret %ld;


; CHECK: After

; CHECK: modified

; CHECK-NOT:  %temp =
; CHECK:      if (%cond != 0)
; CHECK:      {
; CHECK:         (%B)[0] = 1;
; CHECK:      }
; CHECK:      else
; CHECK:      {
; CHECK:         (%B)[0] = 2;
; CHECK:      }
; CHECK-NEXT: (%A)[0] = 5;
; CHECK:      ret %t;


define i32 @foo(ptr noalias %A, ptr noalias %B, i32 %t, i1 %cond) {
entry:
  br label %bb

bb:                                         ; preds = %entry
  store i32 %t, ptr %A, align 4
  br i1 %cond, label %then, label %else

then:
  store i32 1, ptr %B, align 4
  br label %end

else:
  store i32 2, ptr %B, align 4
  br label %end
 
end:
  %ld = load i32, ptr %A, align 4
  store i32 5, ptr %A, align 4
  ret i32 %ld
}

