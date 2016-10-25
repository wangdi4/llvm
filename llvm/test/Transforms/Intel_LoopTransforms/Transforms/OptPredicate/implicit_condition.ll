; The test checks hoisting of def@level predicate (%x > 10) and proper linear (%m > 100) predicate.
; Hoisting of the "if (%x > 10)" outside of the j-loop will generate two j-loops with the same If statement (%m > 100).
; (%m > 100) should not be duplicated after all.

; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s

; Source:
; int foo(int m, char *a, char *b) {
; 
; int i,j;
; 
; for(i=0;i<100;i++) {
;   int x = a[i];
;   for(j=0;j<100;j++) {
;     if (x > 10) {
;       a[i] = i;
;     }
;     if (m > 100) {
;       b[j] = a[i] + i;
;     } else {
;       a[i] = b[j];
;     }
;   }
; }
;
; }

; CHECK:      IR Dump After
; CHECK:      BEGIN REGION { modified }
; CHECK:                 if (%m > 100)
; CHECK-NEXT:            {
; CHECK-NEXT:               + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:               |   %0 = (%a)[i1];
; CHECK-NEXT:               |   if (%0 > 10)
; CHECK-NEXT:               |   {
; CHECK-NEXT:               |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:               |      |   (%a)[i1] = i1;
; CHECK-NEXT:               |      |   %2 = (%a)[i1];
; CHECK-NEXT:               |      |   (%b)[i2] = i1 + %2;
; CHECK-NEXT:               |      + END LOOP
; CHECK-NEXT:               |   }
; CHECK-NEXT:               |   else
; CHECK-NEXT:               |   {
; CHECK-NEXT:               |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:               |      |   %2 = (%a)[i1];
; CHECK-NEXT:               |      |   (%b)[i2] = i1 + %2;
; CHECK-NEXT:               |      + END LOOP
; CHECK-NEXT:               |   }
; CHECK-NEXT:               + END LOOP
; CHECK-NEXT:            }
; CHECK-NEXT:            else
; CHECK-NEXT:            {
; CHECK-NEXT:               + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:               |   %0 = (%a)[i1];
; CHECK-NEXT:               |   if (%0 > 10)
; CHECK-NEXT:               |   {
; CHECK-NEXT:               |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:               |      |   (%a)[i1] = i1;
; CHECK-NEXT:               |      |   %3 = (%b)[i2];
; CHECK-NEXT:               |      |   (%a)[i1] = %3;
; CHECK-NEXT:               |      + END LOOP
; CHECK-NEXT:               |   }
; CHECK-NEXT:               |   else
; CHECK-NEXT:               |   {
; CHECK-NEXT:               |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:               |      |   %3 = (%b)[i2];
; CHECK-NEXT:               |      |   (%a)[i1] = %3;
; CHECK-NEXT:               |      + END LOOP
; CHECK-NEXT:               |   }
; CHECK-NEXT:               + END LOOP
; CHECK-NEXT:            }
; CHECK-NEXT: END REGION

;Module Before HIR; ModuleID = '1.c'
source_filename = "1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32 %m, i8* nocapture %a, i8* nocapture %b) local_unnamed_addr #0 {
entry:
  %cmp10 = icmp sgt i32 %m, 100
  br label %for.body

for.body:                                         ; preds = %for.end, %entry
  %indvars.iv47 = phi i64 [ 0, %entry ], [ %indvars.iv.next48, %for.end ]
  %i.046 = phi i32 [ 0, %entry ], [ %inc25, %for.end ]
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %indvars.iv47
  %0 = load i8, i8* %arrayidx, align 1
  %cmp5 = icmp sgt i8 %0, 10
  %conv7 = trunc i32 %i.046 to i8
  %1 = trunc i64 %indvars.iv47 to i8
  br label %for.body4

for.body4:                                        ; preds = %for.inc, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp5, label %if.then, label %if.end

if.then:                                          ; preds = %for.body4
  store i8 %conv7, i8* %arrayidx, align 1
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body4
  br i1 %cmp10, label %if.then12, label %if.else

if.then12:                                        ; preds = %if.end
  %2 = load i8, i8* %arrayidx, align 1
  %add = add i8 %2, %1
  %arrayidx18 = getelementptr inbounds i8, i8* %b, i64 %indvars.iv
  store i8 %add, i8* %arrayidx18, align 1
  br label %for.inc

if.else:                                          ; preds = %if.end
  %arrayidx20 = getelementptr inbounds i8, i8* %b, i64 %indvars.iv
  %3 = load i8, i8* %arrayidx20, align 1
  store i8 %3, i8* %arrayidx, align 1
  br label %for.inc

for.inc:                                          ; preds = %if.then12, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body4

for.end:                                          ; preds = %for.inc
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %inc25 = add nuw nsw i32 %i.046, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next48, 100
  br i1 %exitcond49, label %for.end26, label %for.body

for.end26:                                        ; preds = %for.end
  ret i32 undef
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

