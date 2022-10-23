; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -disable-output < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-NEW

; In this test, foo() is inlined into goo(), producing a self-recursive
; function foo() with newly exposed callsites foo() itself.
; We verify that the inlining report can be generated without dying.

define internal fastcc i32 @foo(i32 %n) unnamed_addr {
  %call = call fastcc i32 @goo(i32 %n)
  ret i32 %call
}

define internal fastcc i32 @goo(i32 %n) unnamed_addr {
entry:
  switch i32 %n, label %if.end3 [
    i32 0, label %return
    i32 1, label %if.then2
  ]

if.then2:                                         ; preds = %entry
  br label %return

if.end3:                                          ; preds = %entry
  %sub = add nsw i32 %n, -1
  %call = call fastcc i32 @foo(i32 %sub)
  %sub4 = add nsw i32 %n, -2
  %call5 = call fastcc i32 @foo(i32 %sub4)
  %add = add nsw i32 %call, %call5
  br label %return

return:                            ; preds = %entry, %if.end3, %if.then2
  %retval.0 = phi i32 [ 1, %if.then2 ], [ %add, %if.end3 ], [ 0, %entry ]
  ret i32 %retval.0
}

define dso_local i32 @main() local_unnamed_addr {
  %call = call fastcc i32 @foo(i32 34)
  ret i32 %call
}
; CHECK-LABEL-OLD: DEAD STATIC FUNC: foo
; CHECK-LABEL-OLD: COMPILE FUNC: goo
; CHECK-OLD: INLINE: foo{{.*}}Callee is single basic block
; CHECK-OLD: goo{{.*}}Callee has recursion
; CHECK-OLD: INLINE: foo{{.*}}Callee is single basic block
; CHECK-OLD: goo{{.*}}Callee has recursion
; CHECK-LABEL-OLD: COMPILE FUNC: main
; CHECK-OLD: INLINE: foo{{.*}}Callee has single callsite and local linkage
; CHECK-OLD: goo{{.*}}Callee has recursion

; CHECK-LABEL-NEW: DEAD STATIC FUNC: goo
; CHECK-LABEL-NEW: COMPILE FUNC: foo
; CHECK-NEW: INLINE: goo{{.*}}Callee has single callsite and local linkage
; CHECK-NEW: foo{{.*}}Callee has recursion
; CHECK-NEW: foo{{.*}}Callee has recursion
; CHECK-LABEL-NEW: COMPILE FUNC: main
; CHECK-NEW: foo{{.*}}Callee has recursion


