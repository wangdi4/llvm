; This regression happend due to pre-conversion of constants to blobs before the multiplication of CE by Blob.

; Take a look at the expression: LINEAR sext.i32.i64(%n * i1 + %n * i2)
; The Runtime DD tries to replace i2 by the const UB 3.
;
; Please note that const 3 has a i2 IV type - i64, but the expr src type is i32.
; While replacing i2 with 3 the UB src type is set to i32 making the UB CE: sext.i32.i64(3)
; Then it tries to multiply UB CE by blob %n.
; Because we can not put mul %b under the extension, the sext.i32.i64(3) is converted to a standalone blob.
; The SCEV folds constant expression to just i64(3) blob and the call SCEV.getMul(i32(%n), i64(3))
; raises an assertion because of a type conflict.

; HIR:
; + DO i32 i1 = 0, %n + -1, 1   <DO_LOOP>
; | <RVAL-REG> LINEAR i32 %n + -1 {sb:2}
; |    <BLOB> LINEAR i32 %n {sb:4}
; |
; |   + DO i64 i2 = 0, 3, 1   <DO_LOOP>
; |   |   %0 = (%q)[%n * i1 + %n * i2];
; |   |   <LVAL-REG> NON-LINEAR i8 %0 {sb:11}
; |   |   <RVAL-REG> {al:1}(LINEAR i8* %q)[LINEAR sext.i32.i64(%n * i1 + %n * i2)] {sb:21}
; |   |      <BLOB> LINEAR i32 %n {sb:4}
; |   |      <BLOB> LINEAR i8* %q {sb:12}
; |   |
; |   |   (%p)[i1 + i2] = %0;
; |   |   <LVAL-REG> {al:1}(LINEAR i8* %p)[LINEAR sext.i32.i64(i1 + i2)] {sb:21}
; |   |      <BLOB> LINEAR i8* %p {sb:16}
; |   |   <RVAL-REG> NON-LINEAR i8 %0 {sb:11}
; |   |
; |   + END LOOP
; + END LOOP

; RUN: opt -disable-output -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -disable-hir-runtime-dd-cost-model -S < %s 2>&1 | FileCheck %s

; CHECK: if (%n >=u 16 && %mv.and == 0)

; ModuleID = '1.ll'
source_filename = "1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i8* %p, i8* %q, i32 %n) #0 {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.body.lr.ph, label %for.end9

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc7
  %i.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc8, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc
  %j.01 = phi i64 [ 0, %for.body ], [ %inc, %for.inc ]
  %j = trunc i64 %j.01 to i32
  %add = add nsw i32 %i.04, %j
  %mul = mul nsw i32 %add, %n
  %idxprom = sext i32 %mul to i64
  %arrayidx = getelementptr inbounds i8, i8* %q, i64 %idxprom
  %0 = load i8, i8* %arrayidx, align 1
  %add4 = add nsw i32 %i.04, %j
  %idxprom5 = sext i32 %add4 to i64
  %arrayidx6 = getelementptr inbounds i8, i8* %p, i64 %idxprom5
  store i8 %0, i8* %arrayidx6, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add nsw i64 %j.01, 1
  %cmp2 = icmp slt i64 %inc, 4
  br i1 %cmp2, label %for.body3, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc7

for.inc7:                                         ; preds = %for.end
  %inc8 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc8, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end9_crit_edge

for.cond.for.end9_crit_edge:                      ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.cond.for.end9_crit_edge, %entry
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20729)"}
