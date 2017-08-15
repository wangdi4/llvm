; Check that label is remapped after OptPredicate pass

; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S -disable-output -debug < %s 2>&1 | FileCheck %s

; Source:
; int a[1000];
; int b[1000];
; 
; int foo(char *p, char *q, int x, int y) {
;   int i,j;
;   for (i=0;i<1000;++i) {
;    if (x <= 10) {
;      a[i] = 1;
;      if (y < x) {
;        p[i] = 0;
;        goto L1;
;      } else {
;        goto L2;
;      }
;    }
;    q[i] = 1;
;    L1:
;    p[i] = 1;
;    L2:
;   }
;   return p[0];
; }

; Input HIR
;           BEGIN REGION { }
; <32>            + DO i1 = 0, 999, 1   <DO_LOOP>
; <2>             |   if (%x < 11)
; <2>             |   {
; <16>            |      (@a)[0][i1] = 1;
; <17>            |      if (%y < %x)
; <17>            |      {
; <22>            |         (%p)[i1] = 0;
; <23>            |         %arrayidx9.pre-phi = &((%p)[i1]);
; <17>            |      }
; <17>            |      else
; <17>            |      {
; <19>            |         goto for.inc;
; <17>            |      }
; <2>             |   }
; <2>             |   else
; <2>             |   {
; <7>             |      (%q)[i1] = 1;
; <9>             |      %arrayidx9.pre-phi = &((%p)[i1]);
; <2>             |   }
; <12>            |   (%arrayidx9.pre-phi)[0] = 1;
; <25>            |   for.inc:
; <32>            + END LOOP
;           END REGION

; REQUIRES: asserts

; Skip one iteration
; CHECK: Hoisting:

; Capture second iteration
; CHECK: Hoisting:
; CHECK-SAME: if (%y < %x)

; CHECK: BEGIN REGION
; CHECK: goto for.inc.[[NUM:[0-9]+]];
; CHECK-NOT: DO
; CHECK-NOT: END LOOP 
; CHECK: for.inc.[[NUM]]:

; Verify that the region is modified
; CHECK: BEGIN REGION { modified }

;Module Before HIR; ModuleID = 'goto.c'
source_filename = "goto.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i8* nocapture %p, i8* nocapture %q, i32 %x, i32 %y) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp slt i32 %x, 11
  %cmp2 = icmp slt i32 %y, %x
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %indvars.iv
  store i32 1, i32* %arrayidx, align 4
  br i1 %cmp2, label %if.then3, label %for.inc

if.then3:                                         ; preds = %if.then
  %arrayidx5 = getelementptr inbounds i8, i8* %p, i64 %indvars.iv
  store i8 0, i8* %arrayidx5, align 1
  br label %L1

if.end:                                           ; preds = %for.body
  %arrayidx7 = getelementptr inbounds i8, i8* %q, i64 %indvars.iv
  store i8 1, i8* %arrayidx7, align 1
  %.pre = getelementptr inbounds i8, i8* %p, i64 %indvars.iv
  br label %L1

L1:                                               ; preds = %if.end, %if.then3
  %arrayidx9.pre-phi = phi i8* [ %.pre, %if.end ], [ %arrayidx5, %if.then3 ]
  store i8 1, i8* %arrayidx9.pre-phi, align 1
  br label %for.inc

for.inc:                                          ; preds = %L1, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  %0 = load i8, i8* %p, align 1
  %conv = sext i8 %0 to i32
  ret i32 %conv
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

