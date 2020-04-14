; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s

; Verify that call with returned attribute is parsed successfully.
; The test was compfailing in parser.

; CHECK: + DO i64 i1 = 0, 0, 1   <DO_LOOP>
; CHECK: |   %call = @shawrite(&((%ptr)[0]),  i1 + %t);

; Verify that lval was parsed the same as the second argument which has the returned attribute.
; CHECK-NEXT: <LVAL-REG> LINEAR i64 i1 + %t

; CHECK: + END LOOP

define void @foo(i8* %ptr, i64 %t) {
entry:
 br label %loop

loop:                                               ; preds = %loop, %entry
  %iv = phi i64 [ 0, %entry ], [ %inc, %loop ]
  %add = add nsw i64 %iv, %t
  %call = call fastcc i64 @shawrite(i8* %ptr, i64 %add)
  %inc = add nsw i64 %iv, 1
  %cmp = icmp eq i64 %inc, 5
  br i1 %cmp, label %loop, label %exit

exit:
  %cal.lcssa = phi i64 [ %call, %loop ]
  ret void
}

declare hidden fastcc i64 @shawrite(i8*, i64 returned)
