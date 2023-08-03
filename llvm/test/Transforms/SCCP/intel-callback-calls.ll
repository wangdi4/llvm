; RUN: opt -passes=ipsccp -S %s | FileCheck %s
;
; Check that B = 5 is propagated to the callback function from bar().
;
; static void callback(void *ID, int *A, int B, unsigned N) {
;   for (int I = 0; I < N; ++I)
;     A[I] = B * 3;
; }
;
; void broker(int N, void (*)(void*, ...), ...);
;
; static void foo(int *A, int B, unsigned N) {
;   broker(3, (void (*)(void*, ...)) callback, A, B, N);
; }
;
; void bar(int *A, unsigned N) {
;   foo(A, 5, N);
; }
;
; CHECK: define internal void @callback(ptr %ID, ptr [[ABASE:%.+]], i32 %B, i32 %N)
; CHECK:   [[AADR:%.+]] = getelementptr inbounds i32, ptr [[ABASE]], i64 %{{.+}}
; CHECK:   store i32 15, ptr [[AADR]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal void @callback(ptr %ID, ptr %A, i32 %B, i32 %N) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %I.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i32 %I.0, %N
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %I.0 to i64
  %mul = mul nsw i32 %B, 3
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %idxprom
  store i32 %mul, ptr %arrayidx2, align 4
  %inc = add nuw nsw i32 %I.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

define internal void @foo(ptr %A, i32 %B, i32 %N) {
entry:
  call void (i32, ptr, ...) @broker(i32 3, ptr @callback, ptr %A, i32 %B, i32 %N)
  ret void
}

define void @bar(ptr %A, i32 %N) {
entry:
  call void @foo(ptr %A, i32 5, i32 %N)
  ret void
}

declare !callback !0 void @broker(i32, ptr, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}
