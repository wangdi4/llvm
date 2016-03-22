; RUN: opt -loop-simplify -hir-cg -force-hir-cg -S < %s | FileCheck %s

; Verify that CG is able to code generate the implicit call correctly. There is no explicit declaration of bar() in the test case-

; void foo(long n, int *A) {
;   long i;

;   for(i=0; i<n; i++) {
;     if (bar(i)) {
;       A[i]++;
;     }
;   }
; }

; CHECK: region:
; CHECK: loop.{{[0-9]+}}
; CHECK: call i32 (i64, ...) bitcast (i32 (...)* @bar to i32 (i64, ...)*)

; ModuleID = 'implicit-call.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i64 %n, i32* nocapture %A) {
entry:
  %cmp.6 = icmp sgt i64 %n, 0
  br i1 %cmp.6, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.inc
  %i.07 = phi i64 [ %inc1, %for.inc ], [ 0, %entry ]
  %call = tail call i32 (i64, ...) bitcast (i32 (...)* @bar to i32 (i64, ...)*)(i64 %i.07) 
  %tobool = icmp eq i32 %call, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %i.07
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc1 = add nuw nsw i64 %i.07, 1
  %exitcond = icmp eq i64 %inc1, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc, %entry
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) 

declare i32 @bar(...) 

