; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that after complete unrolling the If, switch and labels are removed.

; BEGIN REGION { }
; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   if (%0 != 0)
; |   {
; |      switch(i1)
; |      {
; |      case 67:
; |         break;
; |      case 69:
; |         break;
; |      default:
; |         goto sw.default;
; |      }
; |      %1 = %1  ^  1;
; |      (@x)[0] = %1;
; |      goto for.inc;
; |   }
; |   sw.default:
; |   %3 = (@a)[0][0];
; |   %xor5 = %3  ^  1;
; |   (@a)[0][0] = %xor5;
; |   for.inc:
; + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }

; CHECK-NOT: DO i1
; CHECK-NOT: if
; CHECK-NOT: switch
; CHECK-NOT: :

; CHECK: %3 = (@a)[0][0];
; CHECK: %xor5 = %3  ^  1;
; CHECK: (@a)[0][0] = %xor5;

; CHECK-NOT: if
; CHECK-NOT: switch
; CHECK-NOT: :

; CHECK: %3 = (@a)[0][0];
; CHECK: %xor5 = %3  ^  1;
; CHECK: (@a)[0][0] = %xor5;

; CHECK: END REGION

;Module Before HIR; ModuleID = '_tmp.ll'
source_filename = "atg_CMPLRS-47431.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = local_unnamed_addr global i8 0, align 1
@a = common local_unnamed_addr global [5 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readnone uwtable
define i64 @foo() local_unnamed_addr #0 {
entry:
  ret i64 -67
}

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #1 {
entry:
  %0 = load i8, ptr @x, align 1
  %tobool = icmp eq i8 %0, 0
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %1 = phi i8 [ %0, %entry ], [ %4, %for.inc ]
  %i.012 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 %tobool, label %sw.default, label %cond.true

cond.true:                                        ; preds = %for.body
  %trunc = trunc i32 %i.012 to i31
  switch i31 %trunc, label %sw.default [
    i31 67, label %sw.bb
    i31 69, label %sw.bb
  ]

sw.bb:                                            ; preds = %cond.true, %cond.true
  %2 = xor i8 %1, 1
  store i8 %2, ptr @x, align 1
  br label %for.inc

sw.default:                                       ; preds = %cond.true, %for.body
  %3 = load i32, ptr @a, align 16
  %xor5 = xor i32 %3, 1
  store i32 %xor5, ptr @a, align 16
  br label %for.inc

for.inc:                                          ; preds = %sw.default, %sw.bb
  %4 = phi i8 [ %2, %sw.bb ], [ %1, %sw.default ]
  %inc = add nuw nsw i32 %i.012, 1
  %exitcond = icmp eq i32 %inc, 2
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret i32 0
}

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


