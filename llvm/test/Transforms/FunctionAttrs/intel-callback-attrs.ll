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
; CHECK: define void @callback(ptr nocapture readnone %ID, ptr nocapture writeonly %A, ptr nocapture readonly %B, i32 %N)
; CHECK: define void @foo(ptr nocapture writeonly %A, ptr nocapture readonly %B, i32 %N)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @callback(ptr %ID, ptr %A, ptr %B, i32 %N) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %I.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i32 %I.0, %N
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %I.0 to i64
  %arrayidx = getelementptr inbounds i32, ptr %B, i64 %idxprom
  %0 = load i32, ptr %arrayidx, align 4
  %mul = mul nsw i32 %0, 3
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %idxprom
  store i32 %mul, ptr %arrayidx2, align 4
  %inc = add nuw nsw i32 %I.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define void @foo(ptr %A, ptr %B, i32 %N) {
entry:
  call void (i32, ptr, ...) @broker(i32 3, ptr @callback, ptr %A, ptr %B, i32 %N)
  ret void
}

declare !callback !0 void @broker(i32, ptr, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}
