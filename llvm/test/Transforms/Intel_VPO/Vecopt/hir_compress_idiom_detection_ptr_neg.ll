; RUN: opt %s -mattr=+avx512f,+avx512vl -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert' -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   if ((%c)[i1] != 0)
;       |   {
;       |      (%p.addr.05)[0] = %v;
;       |      %p.addr.05 = &((%p.addr.05)[1]);
;       |   }
;       |   (%p.addr.05)[-2] = %v2;
;       + END LOOP
; END REGION

; CHECK:       [Compress/Expand Idiom] Increment {sb:3}+1 detected: {{.*}} [[P_ADDR_050:%.*]] = &(([[P_ADDR_050]])[1])
; CHECK:       [Compress/Expand Idiom] Inconsistent parent of dependency: {{.*}} ([[P_ADDR_050]])[-2] = [[V20:%.*]]
; CHECK-NEXT:  [Compress/Expand Idiom] Increment rejected: {{.*}} [[P_ADDR_050]] = &(([[P_ADDR_050]])[1])
; CHECK-NEXT:  Idiom List
; CHECK-NEXT:    No idioms detected.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture noundef writeonly %p, ptr noalias nocapture noundef readonly %c, i32 noundef %N, i32 noundef %v, i32 noundef %v2) {
entry:
  %cmp4 = icmp sgt i32 %N, 0
  br i1 %cmp4, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %if.end
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %if.end ]
  %p.addr.05 = phi ptr [ %p, %for.body.preheader ], [ %p.addr.1, %if.end ]
  %arrayidx = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %incdec.ptr = getelementptr inbounds i32, ptr %p.addr.05, i64 1
  store i32 %v, ptr %p.addr.05
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %p.addr.1 = phi ptr [ %incdec.ptr, %if.then ], [ %p.addr.05, %for.body ]
  %add.ptr = getelementptr inbounds i32, ptr %p.addr.1, i64 -2
  store i32 %v2, ptr %add.ptr
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
