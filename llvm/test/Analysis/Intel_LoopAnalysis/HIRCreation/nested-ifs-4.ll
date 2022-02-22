; RUN: opt -enable-new-pm=0 -analyze -xmain-opt-level=2 -hir-ssa-deconstruction -hir-framework -debug-only=hir-region-identification < %s 2>&1 | FileCheck %s -check-prefix=CHECK2
; RUN: opt %s -xmain-opt-level=2 -passes="hir-ssa-deconstruction,print<hir-framework>" -debug-only=hir-region-identification -disable-output 2>&1 | FileCheck %s -check-prefix=CHECK2
; RUN: opt -enable-new-pm=0 -analyze -xmain-opt-level=3 -hir-ssa-deconstruction -hir-framework -debug-only=hir-region-identification < %s 2>&1 | FileCheck %s -check-prefix=CHECK3
; RUN: opt %s -xmain-opt-level=3 -passes="hir-ssa-deconstruction,print<hir-framework>" -debug-only=hir-region-identification -disable-output 2>&1 | FileCheck %s -check-prefix=CHECK3

; Verify that 4 level of IF nesting is allowed for inner loops @ O3.

; void foo(int n, int a, int b, int c, int d, int *p, int *q) {
;  for (int i=0;i<n;++i) {
;    if (a) {
;      if (b) {
;        if (c) {
;          if (d) {
;            p[j] = q[j];
;          } else {
;            d++;
;          }
;        } else {
;          c++;
;        }
;      } else {
;        b++;
;      }
;    } else {
;      a++;
;    }
;  }
;}

; REQUIRES: asserts

; CHECK2: Loop throttled due to presence of too many nested ifs
; CHECK3: BEGIN

;Module Before HIR; ModuleID = 'nested-ifs-4.c'
source_filename = "nested-ifs-4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32 %n, i32 %a, i32 %b, i32 %c, i32 %d, i32* nocapture %p, i32* nocapture readonly %q) local_unnamed_addr #0 {
entry:
  %cmp27 = icmp sgt i32 %n, 0
  br i1 %cmp27, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %a.addr.031 = phi i32 [ %a, %for.body.lr.ph ], [ %a.addr.1, %for.inc ]
  %b.addr.030 = phi i32 [ %b, %for.body.lr.ph ], [ %b.addr.1, %for.inc ]
  %d.addr.029 = phi i32 [ %d, %for.body.lr.ph ], [ %d.addr.1, %for.inc ]
  %c.addr.028 = phi i32 [ %c, %for.body.lr.ph ], [ %c.addr.1, %for.inc ]
  %tobool = icmp eq i32 %a.addr.031, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %tobool1 = icmp eq i32 %b.addr.030, 0
  br i1 %tobool1, label %for.inc, label %if.then2

if.then2:                                         ; preds = %if.then
  %tobool3 = icmp eq i32 %c.addr.028, 0
  br i1 %tobool3, label %for.inc, label %if.then4

if.then4:                                         ; preds = %if.then2
  %tobool5 = icmp eq i32 %d.addr.029, 0
  br i1 %tobool5, label %for.inc, label %if.then6

if.then6:                                         ; preds = %if.then4
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx8 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 %0, i32* %arrayidx8, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then, %if.then2, %if.then4, %if.then6
  %c.addr.1 = phi i32 [ %c.addr.028, %if.then6 ], [ %c.addr.028, %if.then4 ], [ 1, %if.then2 ], [ %c.addr.028, %if.then ], [ %c.addr.028, %for.body ]
  %d.addr.1 = phi i32 [ %d.addr.029, %if.then6 ], [ 1, %if.then4 ], [ %d.addr.029, %if.then2 ], [ %d.addr.029, %if.then ], [ %d.addr.029, %for.body ]
  %b.addr.1 = phi i32 [ %b.addr.030, %if.then6 ], [ %b.addr.030, %if.then4 ], [ %b.addr.030, %if.then2 ], [ 1, %if.then ], [ %b.addr.030, %for.body ]
  %a.addr.1 = phi i32 [ %a.addr.031, %if.then6 ], [ %a.addr.031, %if.then4 ], [ %a.addr.031, %if.then2 ], [ %a.addr.031, %if.then ], [ 1, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


