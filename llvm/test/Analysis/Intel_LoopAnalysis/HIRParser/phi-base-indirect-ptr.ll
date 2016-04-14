; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that indirect pointer with phi base is parsed correctly.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %0 = {al:8}(%p)[i1]
; CHECK-NEXT: {al:4}(%0)[0] = i1
; CHECK-NEXT: END LOOP



; Function Attrs: nounwind uwtable
define i32 @sub(i32** nocapture readonly %p, i32 %n) {
entry:
  %cmp.5 = icmp sgt i32 %n, 0
  br i1 %cmp.5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.07 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %p.addr.06 = phi i32** [ %incdec.ptr, %for.body ], [ %p, %for.body.preheader ]
  %0 = load i32*, i32** %p.addr.06, align 8
  store i32 %i.07, i32* %0, align 4
  %incdec.ptr = getelementptr inbounds i32*, i32** %p.addr.06, i64 1
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 0
}

