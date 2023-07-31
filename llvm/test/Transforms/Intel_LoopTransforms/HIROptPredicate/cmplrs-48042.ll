; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   %x.promoted = (@x)[0];
; |
; |   + DO i2 = 0, -1 * i1 + 1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2>
; |   |   %inc = i2 + %x.promoted  +  1;
; |   |   %inc13 = i1 + i2  +  1;
; |   |   if (%0 != 0)
; |   |   {
; |   |      goto if.then;
; |   |   }
; |   + END LOOP
; |
; |   (@x)[0] = %inc;
; |   %inc16.sink2425 = %inc13;
; |   goto for.inc15;
; |   if.then:
; |   (@x)[0] = %inc;
; |   (@k)[0] = 0;
; |   %2 = %inc;
; |
; |   + DO i2 = 0, 1, 1   <DO_LOOP>
; |   |   if (%1 != 0)
; |   |   {
; |   |      %2 = %2  +  1;
; |   |      (@x)[0] = %2;
; |   |   }
; |   + END LOOP
; |
; |   (@k)[0] = 2;
; |   %inc16.sink2425 = i1;
; |   for.inc15:
; + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }

; CHECK: if (%0 != 0)
; CHECK: {
; CHECK:   if (%1 != 0)
; CHECK:   {
; CHECK:     + DO i1 = 0, 1, 1
; CHECK:     | + DO i2 = 0, 1, 1
; CHECK-NOT: if
; CHECK:     | + END LOOP
; CHECK:     + END LOOP
; CHECK:   }
; CHECK:   else
; CHECK:   {
; CHECK:     + DO i1 = 0, 1, 1
; CHECK-NOT: if
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:   if (%1 != 0)
; CHECK:   {
; CHECK:     + DO i1 = 0, 1, 1
; CHECK:     |   + DO i2 = 0, -1 * i1 + 1, 1
; CHECK-NOT: if
; CHECK:     |   + END LOOP
; CHECK:     + END LOOP
; CHECK:   }
; CHECK:   else
; CHECK:   {
; CHECK:     + DO i1 = 0, 1,
; CHECK:     |   + DO i2 = 0, -1 * i1 + 1, 1
; CHECK-NOT: if
; CHECK:     |   + END LOOP
; CHECK:     + END LOOP
; CHECK:   }
; CHECK: }

;Module Before HIR; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = common local_unnamed_addr global i32 0, align 4
@j = common local_unnamed_addr global i32 0, align 4
@x = common local_unnamed_addr global i32 0, align 4
@c1 = common local_unnamed_addr global i32 0, align 4
@k = common local_unnamed_addr global i32 0, align 4
@c2 = common local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  store i32 0, ptr @i, align 4
  %0 = load i32, ptr @c1, align 4
  %tobool = icmp eq i32 %0, 0
  %1 = load i32, ptr @c2, align 4
  %tobool7 = icmp eq i32 %1, 0
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %entry, %for.inc15
  %inc16.sink24 = phi i32 [ 0, %entry ], [ %inc16, %for.inc15 ]
  %x.promoted = load i32, ptr @x, align 4
  br label %for.body3

for.cond1:                                        ; preds = %for.body3
  %cmp2 = icmp ult i32 %inc13, 2
  br i1 %cmp2, label %for.body3, label %for.cond1.for.inc15.loopexit18_crit_edge

for.body3:                                        ; preds = %for.body3.lr.ph, %for.cond1
  %inc21 = phi i32 [ %x.promoted, %for.body3.lr.ph ], [ %inc, %for.cond1 ]
  %inc13.sink20 = phi i32 [ %inc16.sink24, %for.body3.lr.ph ], [ %inc13, %for.cond1 ]
  %inc = add nsw i32 %inc21, 1
  %inc13 = add nuw nsw i32 %inc13.sink20, 1
  br i1 %tobool, label %for.cond1, label %if.then

if.then:                                          ; preds = %for.body3
  %inc.lcssa = phi i32 [ %inc, %for.body3 ]
  store i32 %inc.lcssa, ptr @x, align 4
  store i32 0, ptr @k, align 4
  br label %for.body6

for.body6:                                        ; preds = %for.inc, %if.then
  %2 = phi i32 [ %inc.lcssa, %if.then ], [ %3, %for.inc ]
  %inc10.sink23 = phi i32 [ 0, %if.then ], [ %inc10, %for.inc ]
  br i1 %tobool7, label %for.inc, label %if.then8

if.then8:                                         ; preds = %for.body6
  %inc9 = add nsw i32 %2, 1
  store i32 %inc9, ptr @x, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6, %if.then8
  %3 = phi i32 [ %2, %for.body6 ], [ %inc9, %if.then8 ]
  %inc10 = add nuw nsw i32 %inc10.sink23, 1
  %exitcond = icmp eq i32 %inc10, 2
  br i1 %exitcond, label %for.inc15.loopexit, label %for.body6

for.inc15.loopexit:                               ; preds = %for.inc
  store i32 2, ptr @k, align 4
  br label %for.inc15

for.cond1.for.inc15.loopexit18_crit_edge:         ; preds = %for.cond1
  %inc.lcssa35 = phi i32 [ %inc, %for.cond1 ]
  %inc13.lcssa34 = phi i32 [ %inc13, %for.cond1 ]
  store i32 %inc.lcssa35, ptr @x, align 4
  br label %for.inc15

for.inc15:                                        ; preds = %for.cond1.for.inc15.loopexit18_crit_edge, %for.inc15.loopexit
  %inc16.sink2425 = phi i32 [ %inc16.sink24, %for.inc15.loopexit ], [ %inc13.lcssa34, %for.cond1.for.inc15.loopexit18_crit_edge ]
  %inc16 = add nuw nsw i32 %inc16.sink24, 1
  %exitcond31 = icmp eq i32 %inc16, 2
  br i1 %exitcond31, label %for.end17, label %for.body3.lr.ph

for.end17:                                        ; preds = %for.inc15
  %inc16.sink2425.lcssa = phi i32 [ %inc16.sink2425, %for.inc15 ]
  store i32 %inc16.sink2425.lcssa, ptr @j, align 4
  store i32 2, ptr @i, align 4
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


