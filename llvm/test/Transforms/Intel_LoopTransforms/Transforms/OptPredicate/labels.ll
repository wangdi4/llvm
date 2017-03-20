; Check that OptPredicate is disabled when there is a label inside the if statement.
; HIR structure has changed, test need to be modified.
; XFAIL: *
; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -hir-cost-model-throttling=0 -S < %s 2>&1 | FileCheck %s

; Source
;
; int foo(int m, char *a, char *b) {
;
; int j;
;
;   for(j=0;j<100;j++) {
;     if (m < 100 && a[m] > 100) {
;       a[j] = j;
;     } else {
;       b[j] = a[j];
;     }
;   }
;
; }
;
; HIR:
;
;           BEGIN REGION { }
; <31>            + DO i1 = 0, 99, 1   <DO_LOOP>
; <2>             |   if (%m < 100)
; <2>             |   {
; <12>            |      %0 = (%a)[%m];
; <14>            |      if (%0 > 100)
; <14>            |      {
; <20>            |         (%a)[i1] = i1;
; <14>            |      }
; <14>            |      else
; <14>            |      {
; <16>            |         goto if.else;
; <14>            |      }
; <2>             |   }
; <2>             |   else
; <2>             |   {
; <5>             |      if.else:
; <7>             |      %1 = (%a)[i1];
; <9>             |      (%b)[i1] = %1;
; <2>             |   }
; <31>            + END LOOP
;           END REGION

; CHECK: IR Dump After
; CHECK: BEGIN REGION { }

;Module Before HIR; ModuleID = '2.c'
source_filename = "2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32 %m, i8* nocapture %a, i8* nocapture %b) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp slt i32 %m, 100
  %idxprom = sext i32 %m to i64
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %idxprom
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %j.020 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 %cmp1, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %for.body
  %0 = load i8, i8* %arrayidx, align 1
  %cmp2 = icmp sgt i8 %0, 100
  br i1 %cmp2, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %conv4 = trunc i32 %j.020 to i8
  %arrayidx6 = getelementptr inbounds i8, i8* %a, i64 %indvars.iv
  store i8 %conv4, i8* %arrayidx6, align 1
  br label %for.inc

if.else:                                          ; preds = %land.lhs.true, %for.body
  %arrayidx8 = getelementptr inbounds i8, i8* %a, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx8, align 1
  %arrayidx10 = getelementptr inbounds i8, i8* %b, i64 %indvars.iv
  store i8 %1, i8* %arrayidx10, align 1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %j.020, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret i32 undef
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

