; RUN: opt -function-attrs -S %s | FileCheck %s
; RUN: opt -passes='function-attrs' -S %s | FileCheck %s
;
; Check that PostOrderFunctionAttrs pass propagates argument attributes from
; the callback to the broker function caller for callback calls.
;
; Test IR is based on the following sample with small additions like callback
; metadata annotation for 'broker' function which tells that 'broker' propagates
; A, B and N arguments to the callback function.
;
; void callback(void *ID, int *A, int *B, unsigned N) {
;   for (int I = 0; I < N; ++I)
;     A[I] = B[I] * 3;
; }
;
; void broker(int N, void (*)(void*, ...), ...);
;
; void foo(int *A, int *B, unsigned N) {
;   broker(3, (void (*)(void*, ...)) callback, A, B, N);
; }
;
; CHECK: define void @callback(i8* nocapture readnone %ID, i32* nocapture writeonly %A, i32* nocapture readonly %B, i32 %N)
; CHECK: define void @foo(i32* nocapture writeonly %A, i32* nocapture readonly %B, i32 %N)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @callback(i8* %ID, i32* %A, i32* %B, i32 %N) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %I.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i32 %I.0, %N
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %I.0 to i64
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %0, 3
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %mul, i32* %arrayidx2, align 4
  %inc = add nuw nsw i32 %I.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define void @foo(i32* %A, i32* %B, i32 %N) {
entry:
  call void (i32, void (i8*, ...)*, ...) @broker(i32 3, void (i8*, ...)* bitcast (void (i8*, i32*, i32*, i32)* @callback to void (i8*, ...)*), i32* %A, i32* %B, i32 %N)
  ret void
}

declare !callback !0 void @broker(i32, void (i8*, ...)*, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}
