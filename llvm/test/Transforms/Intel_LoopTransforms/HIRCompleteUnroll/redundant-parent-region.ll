; Check that the region should be empty after unroll. All redundant nodes should be removed

; RUN: opt -hir-ssa-deconstruction -disable-output -hir-post-vec-complete-unroll -print-after-all -stats < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -disable-output -hir-post-vec-complete-unroll -print-after-all -stats < %s 2>&1 | FileCheck %s --check-prefix=DEBUG

; Input HIR:
; <0>       BEGIN REGION { }
; <65>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; <66>            |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; <67>            |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
; <68>            |   |   |   + DO i4 = 0, i3 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9>
; <20>            |   |   |   |   if (i3 + i4 > 25)
; <20>            |   |   |   |   {
; <27>            |   |   |   |      (%a)[i3] = i4;
; <20>            |   |   |   |   }
; <68>            |   |   |   + END LOOP
; <67>            |   |   + END LOOP
; <66>            |   + END LOOP
; <65>            + END LOOP
; <0>       END REGION

; CHECK: IR Dump After

; CHECK: BEGIN REGION
; CHECK-NEXT: END REGION

; DEBUG: IR Dump After

; DEBUG: BEGIN REGION
; DEBUG-NEXT: END REGION

; DEBUG-REQUIRES: asserts

; DEBUG: Statistics Collected
; DEBUG:  1 hir-hlnode-utils          - Number of regions invalidated by utility
; DEBUG:  2 hir-hlnode-utils          - Number of empty Loops removed by utility
; DEBUG: 45 hir-hlnode-utils          - Number of redundant predicates removed by utility
; DEBUG-NOT: hir-hlnode-utils

; ModuleID = '1.ll'
source_filename = "1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i8* %a, i32 %n) #0 {
entry:
  %cmp6 = icmp slt i32 0, %n
  br i1 %cmp6, label %for.body.lr.ph, label %for.end19

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc17
  %k1.07 = phi i32 [ 0, %for.body.lr.ph ], [ %inc18, %for.inc17 ]
  %cmp24 = icmp slt i32 0, %n
  br i1 %cmp24, label %for.body3.lr.ph, label %for.end16

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc14
  %k.05 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc15, %for.inc14 ]
  br label %for.body6

for.body6:                                        ; preds = %for.body3, %for.inc11
  %i.03 = phi i32 [ 0, %for.body3 ], [ %inc12, %for.inc11 ]
  %cmp81 = icmp slt i32 0, %i.03
  br i1 %cmp81, label %for.body9.lr.ph, label %for.end

for.body9.lr.ph:                                  ; preds = %for.body6
  br label %for.body9

for.body9:                                        ; preds = %for.body9.lr.ph, %for.inc
  %j.02 = phi i32 [ 0, %for.body9.lr.ph ], [ %inc, %for.inc ]
  %add = add nsw i32 %i.03, %j.02
  %cmp10 = icmp sgt i32 %add, 25
  br i1 %cmp10, label %if.then, label %if.end

if.then:                                          ; preds = %for.body9
  %conv = trunc i32 %j.02 to i8
  %idxprom = sext i32 %i.03 to i64
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %idxprom
  store i8 %conv, i8* %arrayidx, align 1
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body9
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %j.02, 1
  %cmp8 = icmp slt i32 %inc, %i.03
  br i1 %cmp8, label %for.body9, label %for.cond7.for.end_crit_edge

for.cond7.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond7.for.end_crit_edge, %for.body6
  br label %for.inc11

for.inc11:                                        ; preds = %for.end
  %inc12 = add nsw i32 %i.03, 1
  %cmp5 = icmp slt i32 %inc12, 10
  br i1 %cmp5, label %for.body6, label %for.end13

for.end13:                                        ; preds = %for.inc11
  br label %for.inc14

for.inc14:                                        ; preds = %for.end13
  %inc15 = add nsw i32 %k.05, 1
  %cmp2 = icmp slt i32 %inc15, %n
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end16_crit_edge

for.cond1.for.end16_crit_edge:                    ; preds = %for.inc14
  br label %for.end16

for.end16:                                        ; preds = %for.cond1.for.end16_crit_edge, %for.body
  br label %for.inc17

for.inc17:                                        ; preds = %for.end16
  %inc18 = add nsw i32 %k1.07, 1
  %cmp = icmp slt i32 %inc18, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end19_crit_edge

for.cond.for.end19_crit_edge:                     ; preds = %for.inc17
  br label %for.end19

for.end19:                                        ; preds = %for.cond.for.end19_crit_edge, %entry
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 21010) (llvm/branches/loopopt 21057)"}
