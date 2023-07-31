; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert' -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <vectorize>
;       |   %0 = (%in)[i1];
;       |   if (%0 != 0)
;       |   {
;       |      (%out1)[%k.022].0 = %0;
;       |      (%out1)[%k.022].1 = %0;
;       |      %k.022 = %k.022  +  1;
;       |   }
;       + END LOOP
; END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+1 detected: {{.*}} %k.022 = %k.022  +  1;
; CHECK-NEXT: [Compress/Expand Idiom] Structures fields access is not supported: (%out1)[%k.022].0
; CHECK-NEXT: [Compress/Expand Idiom] Increment rejected: {{.*}} %k.022 = %k.022  +  1;
; CHECK-NEXT: Idiom List
; CHECK-NEXT:   No idioms detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32, i32 }

define void @_Z3fooP1SPii(ptr noalias %out1, ptr %in, i32 %n) {
entry:
  %cmp21 = icmp sgt i32 %n, 0
  br i1 %cmp21, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %k.022 = phi i32 [ 0, %for.body.preheader ], [ %k.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %in, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %idxprom3 = sext i32 %k.022 to i64
  %a = getelementptr inbounds %struct.S, ptr %out1, i64 %idxprom3, i32 0
  store i32 %0, ptr %a, align 4
  %b = getelementptr inbounds %struct.S, ptr %out1, i64 %idxprom3, i32 1
  store i32 %0, ptr %b, align 4
  %inc = add nsw i32 %k.022, 1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %k.1 = phi i32 [ %inc, %if.then ], [ %k.022, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
