; RUN: opt < %s -hir-cost-model-throttling=0 -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

; Verify the test case compiles successfully. It was incorrectly accessing upper
; of i2 unknown loops while trying to delinearize (%ptr)[%n * i1 + i2].

; HIR-
; + UNKNOWN LOOP i1
; |   <i1 = 0>
; |   for.cond8.preheader.i:
; |
; |   + UNKNOWN LOOP i2
; |   |   <i2 = 0>
; |   |   for.body10.i:
; |   |   (%ptr)[%n * i1 + i2] = %n * i1 + i2;
; |   |   %ub = (%ub_ptr)[0];
; |   |   if (%ub >u i2 + 1)
; |   |   {
; |   |      <i2 = i2 + 1>
; |   |      goto for.body10.i;
; |   |   }
; |   + END LOOP
; |
; |   if (i1 + 1 <u %ub)
; |   {
; |      <i1 = i1 + 1>
; |      goto for.cond8.preheader.i;
; |   }
; + END LOOP

; CHECK: (%ub_ptr)[0] --> (%ptr)[%n * i1 + i2]


define void @foo(i32* %ptr, i32* %ub_ptr, i32 %n) {
entry:
  br label %for.cond8.preheader.i

for.cond8.preheader.i:                            ; preds = %for.inc32.i, %entry
  %i = phi i32 [ 0, %entry ], [ %i.inc, %for.inc32.i ]
  %i.inc = add nuw i32 %i, 1
  br label %for.body10.i.preheader

for.body10.i.preheader:                           ; preds = %for.cond8.preheader.i
  br label %for.body10.i

for.body10.i:                                     ; preds = %for.body10.i, %for.body10.i.preheader
  %j = phi i32 [ %inc.i, %for.body10.i ], [ 0, %for.body10.i.preheader ]
  %mul11.i = mul i32 %n, %i
  %add12.i = add i32 %mul11.i, %j
  %add.ptr.i = getelementptr inbounds i32, i32* %ptr, i32 %add12.i
  store i32 %add12.i, i32* %add.ptr.i, align 4
  %ub = load i32, i32* %ub_ptr, align 4
  %inc.i = add nuw i32 %j, 1
  %cmp9.i = icmp ugt i32 %ub, %inc.i
  br i1 %cmp9.i, label %for.body10.i, label %for.inc32.loopexit.i

for.inc32.loopexit.i:                             ; preds = %for.body10.i
  %.lcssa = phi i32 [ %ub, %for.body10.i ]
  br label %for.inc32.i

for.inc32.i:                                      ; preds = %for.inc32.loopexit.i
  %t21 = phi i32 [ %.lcssa, %for.inc32.loopexit.i ]
  %cmp.i = icmp ult i32 %i.inc, %t21
  br i1 %cmp.i, label %for.cond8.preheader.i, label %for.cond35.preheader.i

for.cond35.preheader.i:                           ; preds = %for.inc32.i
  ret void
}
