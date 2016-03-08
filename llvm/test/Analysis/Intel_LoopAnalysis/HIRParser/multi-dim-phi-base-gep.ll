; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the phi base (%ref.06) whose higher dimension is inductive in the loop is handled correctly.
; CHECK: DO i1 = 0, 99
; CHECK-NEXT: %0 = (%inp)[i1][0];
; CHECK-NEXT: %x.07 = %0  +  %x.07;
; CHECK-NEXT: END LOOP


;Module Before HIR; ModuleID = 'tr16757.c'

; Function Attrs: nounwind readonly uwtable
define i32 @doit([32 x i32]* nocapture readonly %inp) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %k.08 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %x.07 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %ref.06 = phi [32 x i32]* [ %inp, %entry ], [ %incdec.ptr, %for.body ]
  %incdec.ptr = getelementptr inbounds [32 x i32], [32 x i32]* %ref.06, i32 1
  %arraydecay = getelementptr inbounds [32 x i32], [32 x i32]* %ref.06, i32 0, i32 0
  %0 = load i32, i32* %arraydecay, align 4
  %add = add i32 %0, %x.07
  %inc = add nuw nsw i32 %k.08, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa
}

