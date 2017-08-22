; RUN: opt -hir-cg -force-hir-cg -hir-cost-model-throttling=0 -S < %s | FileCheck %s

; Verify that CG is able to code generate the implicit call correctly. There is no explicit declaration of bar() in the test case-

; void foo(long n, int *A) {
;   long i;

;   for(i=0; i<n; i++) {
;     if (bar(i)) {
;       A[i]++;
;     }
;   }
; }

; CHECK: region.0:
; CHECK: loop.{{[0-9]+}}
; CHECK: call i32 (i64, ...) bitcast (i32 (...)* @bar to i32 (i64, ...)*)

; ModuleID = 'implicit-call.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64 %n, i32* nocapture %A) {
entry:
  %cmp.6 = icmp sgt i64 %n, 0
  br i1 %cmp.6, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %i.07 = phi i64 [ %inc1, %for.inc ], [ 0, %for.body.preheader ]
  %call = tail call i32 (i64, ...) bitcast (i32 (...)* @bar to i32 (i64, ...)*)(i64 %i.07)
  %tobool = icmp eq i32 %call, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %i.07
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %inc1 = add nuw nsw i64 %i.07, 1
  %exitcond = icmp eq i64 %inc1, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #0

declare i32 @bar(...)

attributes #0 = { argmemonly nounwind }
