; RUN: opt < %s -passes="hir-ssa-deconstruction,print,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that we put a live.range metadata on the non-header phi %Res of the
; SCC (%Res -> %Tmp) to suppress parsing %Res as min(%Tmp, %Val).


; CHECK: %Res = phi i32 [ %Val, %if.then ], [ %Tmp, %for.body ], !live.range.de.ssa

; CHECK: SCC1: %Res -> %Tmp

; CHECK: + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK: |   %Val = (%ip)[i1];
; CHECK: |   if (%Tmp > %Val)
; CHECK: |   {
; CHECK: |      %Tmp = %Val;
; CHECK: |   }
; CHECK: |   %Tmp.out = %Tmp;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(ptr nocapture readonly %ip) {
entry:
  %min = alloca i32, align 4
  store i32 2147483647, ptr %min, align 4
  br label %for.body.pre

for.body.pre:                              ; preds = %entry
  %.pre = load i32, ptr %min, align 4
  br label %for.body

for.body:                                      ; preds = %6, %for.body.pre
  %Tmp = phi i32 [ %.pre, %for.body.pre ], [ %Res, %for.inc ]
  %indvars.iv = phi i64 [ 0, %for.body.pre ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %ip, i64 %indvars.iv
  %Val = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp sgt i32 %Tmp, %Val
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                      ; preds = %2
  br label %for.inc

for.inc:                                      ; preds = %5, %2
  %Res = phi i32 [ %Val, %if.then ], [ %Tmp, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                      ; preds = %6
  %.lcssa = phi i32 [ %Res, %for.inc ]
  store i32 %.lcssa, ptr %min, align 4
  br label %exit

exit:                              ; preds = %for.end
  ret i32 %.lcssa
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

