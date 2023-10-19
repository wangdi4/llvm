; REQUIRES: asserts
; RUN: opt < %s -passes="print<hir-region-identification>" -hir-loop-or-bb-inst-threshold=5 -debug-only=hir-region-identification  2>&1 | FileCheck %s --check-prefix=EXCLUDE-PRE

; RUN: opt < %s -passes="print<hir-region-identification>" -hir-loop-or-bb-inst-threshold=10 -debug-only=hir-region-identification  2>&1 | FileCheck %s --check-prefix=INCLUDE-PRE

; Verify that we exclude preheader of small constant innermost loops if their
; size is over the threshold.


; EXCLUDE-PRE: Excluding basic block: %pre as it is too big
; EXCLUDE-PRE: EntryBB: %for.body

; INCLUDE-PRE: EntryBB: %pre


define void @foo(ptr %p) {
entry:
  %priv = alloca [4 x i32], align 4
  br label %pre

pre:
  %gep = getelementptr inbounds i32, ptr %p, i64 1
  %ld1 = load i32, ptr %gep 
  %ld2 = load i32, ptr %gep 
  %ld3 = load i32, ptr %gep 
  %ld4 = load i32, ptr %gep 
  %ld5 = load i32, ptr %gep 
  %ld6 = load i32, ptr %gep 
  %ld7 = load i32, ptr %gep 
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %pre ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [4 x i32], ptr %priv, i64 0, i64 %indvars.iv
  store i32 1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.body, label %exit

exit:
  ret void
}

