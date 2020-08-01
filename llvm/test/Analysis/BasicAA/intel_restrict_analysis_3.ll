; BasicAA shouldn't compute %b and %arrayidx3 are NoAlias.
; %b is marked as noalias (restrict) argument and %arrayidx3
; is a copy of %b.
;
; RUN: opt < %s -basic-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; Run: opt < %s -convert-to-subscript -S | opt -basic-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK-NOT:  NoAlias:      i32* %arrayidx3, i32* %b

define void @foo(i32* noalias %a, i32* noalias %b, i32* %c, i32 %n) {
entry:
  store i32 0, i32* %b, align 4
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx1, align 4
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %add = add i32 %0, 1
  %add4 = add i32 %add, %1
  store i32 %add4, i32* %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}
