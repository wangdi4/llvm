<<<<<<< HEAD
; RUN: opt < %s -basic-aa -gvn -S | FileCheck %s
=======
; RUN: opt < %s -passes=gvn -S | FileCheck %s
>>>>>>> 881c6c0d46ae1b72fb60bbb6a547577f79a5d14f

declare i32 @foo(i32) readnone

define i1 @bar() {
; CHECK-LABEL: @bar(
  %a = call i32 @foo (i32 0) readnone
  %b = call i32 @foo (i32 0) readnone
  %c = and i32 %a, %b
  %x = call i32 @foo (i32 %a) readnone
  %y = call i32 @foo (i32 %c) readnone
  %z = icmp eq i32 %x, %y
  ret i1 %z
; CHECK: ret i1 true
} 
