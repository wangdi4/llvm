; RUN: opt -disable-output "-passes=print<scalar-evolution>" < %s 2>&1 | FileCheck %s

; Enable ScalarEvolution for IV before it getting freezed.
; In the following case, we can still interprete %i but don't interprete %i.fr.

; CHECK: -->  {0,+,1}<%loop>
; CHECK: -->  %i.fr
; CHECK: -->  (1 + %i.fr)
define void @foo(i32 %n) {
entry: 
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ],[ %i.0, %inc ]
  %i.fr = freeze i32 %i
  br label %inc

inc:
  %i.0 = add i32 %i.fr, 1
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}
