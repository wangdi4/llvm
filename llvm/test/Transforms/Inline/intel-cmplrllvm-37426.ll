; RUN: opt < %s -passes='default<O3>' -inlinedefault-threshold=255 -S | FileCheck %s --check-prefix=CHECK255
; RUN: opt < %s -passes='default<O3>' -inlinedefault-threshold=265 -S | FileCheck %s --check-prefix=CHECK265

; Check that the inlinedefault-threshold does alter the inline threshold at
; -O3 if over the aggressive default value of 250.

; CHECK-LABEL: define i32 @outer() 
; CHECK255: call i32 @inner260()
; CHECK265-NOT: call i32 @inner260()

@a = global i32 4

define i32 @inner260() "function-inline-cost"="260" {
entry:
  %a1 = load volatile i32, i32* @a
  %x1 = add i32 %a1,  %a1
  %cmp = icmp eq i32 %x1, -1
  br i1 %cmp, label %exit, label %cont
cont:
  %x2 = add i32 %x1,  %a1
  %cmp1 = icmp eq i32 %x2, 1000
  br i1 %cmp1, label %exit, label %cont
exit:
  %x3 = phi i32 [ %x2, %cont ], [ %x1, %entry ]
  ret i32 %x3
}

define i32 @outer() {
  %r = call i32 @inner260()
  ret i32 %r
}
