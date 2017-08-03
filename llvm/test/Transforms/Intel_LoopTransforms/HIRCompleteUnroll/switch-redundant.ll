; RUN: opt -hir-ssa-deconstruction -disable-output -hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll < %s 2>&1 | FileCheck %s

; HIR:
;  BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;       |   + DO i2 = 0, 9, 1   <DO_LOOP>
;       |   |   switch(i2)
;       |   |   {
;       |   |   case 1:
;       |   |      goto sw.bb;
;       |   |   case 2:
;       |   |      goto sw.bb;
;       |   |   case 3:
;       |   |      break;
;       |   |   case 4:
;       |   |      break;
;       |   |   default:
;       |   |      %2 = (%q)[i2];
;       |   |      (%q)[i2] = %2 + 3;
;       |   |      goto exit;
;       |   |   }
;       |   |   %1 = (%p)[i2];
;       |   |   (%p)[i2] = %1 + 2;
;       |   |   goto exit;
;       |   |   sw.bb:
;       |   |   %0 = (%p)[i2];
;       |   |   (%p)[i2] = %0 + 1;
;       |   |   exit:
;       |   + END LOOP
;       + END LOOP
;  END REGION

; CHECK:   BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, %n + -1, 1
; CHECK-NEXT:  |   %2 = (%q)[0];
; CHECK-NEXT:  |   (%q)[0] = %2 + 3;
; CHECK-NEXT:  |   %0 = (%p)[1];
; CHECK-NEXT:  |   (%p)[1] = %0 + 1;
; CHECK-NEXT:  |   %0 = (%p)[2];
; CHECK-NEXT:  |   (%p)[2] = %0 + 1;
; CHECK-NEXT:  |   %1 = (%p)[3];
; CHECK-NEXT:  |   (%p)[3] = %1 + 2;
; CHECK-NEXT:  |   %1 = (%p)[4];
; CHECK-NEXT:  |   (%p)[4] = %1 + 2;
; CHECK-NEXT:  |   %2 = (%q)[5];
; CHECK-NEXT:  |   (%q)[5] = %2 + 3;
; CHECK-NEXT:  |   %2 = (%q)[6];
; CHECK-NEXT:  |   (%q)[6] = %2 + 3;
; CHECK-NEXT:  |   %2 = (%q)[7];
; CHECK-NEXT:  |   (%q)[7] = %2 + 3;
; CHECK-NEXT:  |   %2 = (%q)[8];
; CHECK-NEXT:  |   (%q)[8] = %2 + 3;
; CHECK-NEXT:  |   %2 = (%q)[9];
; CHECK-NEXT:  |   (%q)[9] = %2 + 3;
; CHECK-NEXT: + END LOOP
; CHECK:   END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* %p, i32* %q, i32* %r, i32 %n, i32 %k) #0 {
entry:
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %for.body.lr.ph, label %for.end13

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc11
  %j.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc12, %for.inc11 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc
  %i.01 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  switch i32 %i.01, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb
    i32 3, label %sw.bb4
    i32 4, label %sw.bb4
  ]

sw.bb:                                            ; preds = %for.body3, %for.body3
  br label %L1

sw.bb4:                                           ; preds = %for.body3, %for.body3
  br label %L2

sw.default:                                       ; preds = %for.body3
  br label %L3

L1:                                               ; preds = %sw.bb
  %idxprom = zext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* %arrayidx, align 4
  br label %exit

L2:                                               ; preds = %sw.bb4
  %idxprom5 = zext i32 %i.01 to i64
  %arrayidx6 = getelementptr inbounds i32, i32* %p, i64 %idxprom5
  %1 = load i32, i32* %arrayidx6, align 4
  %add7 = add nsw i32 %1, 2
  store i32 %add7, i32* %arrayidx6, align 4
  br label %exit

L3:                                               ; preds = %sw.default
  %idxprom8 = zext i32 %i.01 to i64
  %arrayidx9 = getelementptr inbounds i32, i32* %q, i64 %idxprom8
  %2 = load i32, i32* %arrayidx9, align 4
  %add10 = add nsw i32 %2, 3
  store i32 %add10, i32* %arrayidx9, align 4
  br label %exit

exit:                                             ; preds = %L3, %L2, %L1
  %i.02 = phi i32 [ %i.01, %L3 ], [ %i.01, %L2 ], [ %i.01, %L1 ]
  br label %for.inc

for.inc:                                          ; preds = %exit
  %inc = add i32 %i.02, 1
  %cmp2 = icmp ult i32 %inc, 10
  br i1 %cmp2, label %for.body3, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc11

for.inc11:                                        ; preds = %for.end
  %inc12 = add nsw i32 %j.04, 1
  %cmp = icmp slt i32 %inc12, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end13_crit_edge

for.cond.for.end13_crit_edge:                     ; preds = %for.inc11
  br label %for.end13

for.end13:                                        ; preds = %for.cond.for.end13_crit_edge, %entry
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


