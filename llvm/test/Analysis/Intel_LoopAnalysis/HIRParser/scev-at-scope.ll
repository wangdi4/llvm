; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Src code-
; while (count-- > 0) {
;   for (todo = len; todo > 0; todo--) {
;     *to++ = *from++;
;   }
;   from = frombase;
; }

; TODO: change SCC formation to identifiy the bigger cycle- to.addr.130 -> to.addr.227 -> incdec.ptr8 -> to.addr.130 instead of the smaller cycle to.addr.130 -> to.addr.130.

; Check parsing output for the loop verifying that the copy stmt after i2 loop is parsed properly using getSCEVAtScope() information.

; CHECK:      + DO i1 = 0, %count + smax(-2, (-1 + (-1 * %count))) + 1, 1   <DO_LOOP>
; CHECK-NEXT: |   %to.addr.130.out = &((%to.addr.130)[0]);
; CHECK-NEXT: |   if (%len > 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      + DO i2 = 0, %len + smax(-2, (-1 + (-1 * %len))) + 1, 1   <DO_LOOP>
; CHECK-NEXT: |      |   %0 = {al:1}(%from)[i2];
; CHECK-NEXT: |      |   {al:1}(%to.addr.130.out)[i2] = %0;
; CHECK-NEXT: |      + END LOOP
; CHECK-NEXT: |      %to.addr.130 = &((%to.addr.130.out)[%len + smax(-2, (-1 + (-1 * %len))) + 2]);
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP


; ModuleID = 'test.ll'

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture, i8, i32, i32, i1) #0

define void @Perl_repeatcpy(i8* nocapture %to, i8* nocapture readonly %from, i32 %len, i32 %count) {
entry:
  %cmp = icmp eq i32 %len, 1
  %cmp121 = icmp sgt i32 %count, 0
  br i1 %cmp, label %if.then, label %while.cond2.preheader

while.cond2.preheader:                            ; preds = %entry
  br i1 %cmp121, label %for.cond.preheader.lr.ph, label %cleanup

for.cond.preheader.lr.ph:                         ; preds = %while.cond2.preheader
  %cmp624 = icmp sgt i32 %len, 0
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %while.cond2.loopexit, %for.cond.preheader.lr.ph
  %dec331.in = phi i32 [ %count, %for.cond.preheader.lr.ph ], [ %dec331, %while.cond2.loopexit ]
  %to.addr.130 = phi i8* [ %to, %for.cond.preheader.lr.ph ], [ %to.addr.2.lcssa, %while.cond2.loopexit ]
  %dec331 = add nsw i32 %dec331.in, -1
  br i1 %cmp624, label %for.body.preheader, label %while.cond2.loopexit

for.body.preheader:                               ; preds = %for.cond.preheader
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %to.addr.227 = phi i8* [ %incdec.ptr8, %for.body ], [ %to.addr.130, %for.body.preheader ]
  %from.addr.126 = phi i8* [ %incdec.ptr7, %for.body ], [ %from, %for.body.preheader ]
  %todo.025 = phi i32 [ %dec9, %for.body ], [ %len, %for.body.preheader ]
  %incdec.ptr7 = getelementptr inbounds i8, i8* %from.addr.126, i32 1
  %0 = load i8, i8* %from.addr.126, align 1
  %incdec.ptr8 = getelementptr inbounds i8, i8* %to.addr.227, i32 1
  store i8 %0, i8* %to.addr.227, align 1
  %dec9 = add nsw i32 %todo.025, -1
  %cmp6 = icmp sgt i32 %todo.025, 1
  br i1 %cmp6, label %for.body, label %while.cond2.loopexit.loopexit

while.cond2.loopexit.loopexit:                    ; preds = %for.body
  br label %while.cond2.loopexit

while.cond2.loopexit:                             ; preds = %for.cond.preheader, %while.cond2.loopexit.loopexit
  %to.addr.2.lcssa = phi i8* [ %to.addr.130, %for.cond.preheader ], [ %incdec.ptr8, %while.cond2.loopexit.loopexit ]
  %cmp4 = icmp sgt i32 %dec331.in, 1
  br i1 %cmp4, label %for.cond.preheader, label %cleanup.loopexit

cleanup.loopexit:                                 ; preds = %while.cond2.loopexit
  br label %cleanup

if.then:                                          ; preds = %entry
  br i1 %cmp121, label %while.body.preheader, label %cleanup

while.body.preheader:                             ; preds = %if.then
  %1 = load i8, i8* %from, align 1
  call void @llvm.memset.p0i8.i32(i8* %to, i8 %1, i32 %count, i32 1, i1 false)
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %while.body.preheader, %if.then, %while.cond2.preheader
  ret void
}

attributes #0 = { argmemonly nounwind }

