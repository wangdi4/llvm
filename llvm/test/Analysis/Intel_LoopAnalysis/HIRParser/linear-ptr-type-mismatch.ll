; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that we can parse %ptBlist2.017 and %ptBlist1.016 cleanly in case of type mismatch of the base ptr in the SCEV {al:4}(%ptr) which is i64* and the type of the phi which is i32*.

; CHECK: DO i1 = 0, 4
; CHECK-NEXT: %2 = {al:4}(%0)[i1]
; CHECK-NEXT: %3 = {al:4}(%1)[i1]
; CHECK-NEXT: %or = %3  ||  %2
; CHECK-NEXT: {al:4}(%1)[i1] = %or
; CHECK-NEXT: END LOOP


; Function Attrs: nounwind
define void @foo(i64* %ptr) {
entry:
  %0 = bitcast i64* %ptr to i32*
  %1 = bitcast i64* %ptr to i32*
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.018 = phi i32 [ 5, %entry ], [ %dec, %for.body ]
  %ptBlist2.017 = phi i32* [ %0, %entry ], [ %incdec.ptr, %for.body ]
  %ptBlist1.016 = phi i32* [ %1, %entry ], [ %incdec.ptr71, %for.body ]
  %incdec.ptr = getelementptr i32, i32* %ptBlist2.017, i32 1
  %2 = load i32, i32* %ptBlist2.017, align 4
  %incdec.ptr71 = getelementptr i32, i32* %ptBlist1.016, i32 1
  %3 = load i32, i32* %ptBlist1.016, align 4
  %or = or i32 %3, %2
  store i32 %or, i32* %ptBlist1.016, align 4
  %dec = add i32 %i.018, -1
  %cmp69 = icmp eq i32 %dec, 0
  br i1 %cmp69, label %for.end.loopexit, label %for.body

for.end.loopexit:
  ret void
}
