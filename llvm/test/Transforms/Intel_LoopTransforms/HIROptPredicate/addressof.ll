; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Check that the following switch is optimized.

; BEGIN REGION { }
;       + DO i1 = 0, -1 * sext.i32.i64(%1) + smax(sext.i32.i64((%0 + %1)), (1 + sext.i32.i64(%1))) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;       |   %r.1 = %r.031;
;       |   if (&((%2)[0]) == null)
;       |   {
;       |      %5 = (%lpears)[i1 + sext.i32.i64(%1)];
;       |      %r.1 = %5;
;       |   }
;       |   %6 = (%2)[i1 + sext.i32.i64(%1)];
;       |   %add14 = 3 * %6  +  %r.1;
;       |   %r.031 = 3 * %6 + %r.1;
;       + END LOOP
; END REGION

; CHECK:  BEGIN REGION { modified }
; CHECK:        if (&((%2)[0]) == null)
; CHECK:        {
; CHECK:           + DO i1
; CHECK:           + END LOOP
; CHECK:        }
; CHECK:        else
; CHECK:        {
; CHECK:           + DO i1
; CHECK:           + END LOOP
; CHECK:        }
; CHECK:  END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define hidden fastcc i32 @foo(i32 %i, ptr nocapture readonly %lpears, ptr nocapture readonly %upears, ptr nocapture readonly %pearlist) {
entry:
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32, ptr %upears, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, ptr %lpears, i64 %idxprom
  %1 = load i32, ptr %arrayidx2, align 4
  %cmp30 = icmp sgt i32 %0, 0
  br i1 %cmp30, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %add = add nsw i32 %1, %0
  %arrayidx6 = getelementptr inbounds ptr, ptr %pearlist, i64 %idxprom
  %2 = load ptr, ptr %arrayidx6, align 8
  %cmp7 = icmp eq ptr %2, null
  %3 = sext i32 %1 to i64
  %4 = sext i32 %add to i64
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.lr.ph
  %indvars.iv = phi i64 [ %3, %for.body.lr.ph ], [ %indvars.iv.next, %if.end ]
  %r.031 = phi i32 [ 0, %for.body.lr.ph ], [ %add14, %if.end ]
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx9 = getelementptr inbounds i32, ptr %lpears, i64 %indvars.iv
  %5 = load i32, ptr %arrayidx9, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %r.1 = phi i32 [ %5, %if.then ], [ %r.031, %for.body ]
  %arrayidx13 = getelementptr inbounds i32, ptr %2, i64 %indvars.iv
  %6 = load i32, ptr %arrayidx13, align 4
  %mul = mul nsw i32 %6, 3
  %add14 = add nsw i32 %mul, %r.1
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %4
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %if.end
  %add14.lcssa = phi i32 [ %add14, %if.end ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %r.0.lcssa = phi i32 [ 0, %entry ], [ %add14.lcssa, %for.end.loopexit ]
  ret i32 %r.0.lcssa
}

