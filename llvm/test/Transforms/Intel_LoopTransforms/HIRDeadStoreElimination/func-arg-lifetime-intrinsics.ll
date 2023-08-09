; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that stores to %A (which is a function argument) are eliminated by
; treating fake refs in lifetime.end intrinsics as post-dominating stores.


; CHECK: Dump Before

; CHECK:      + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK-NEXT: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   (%A)[%n] = %t2;
; CHECK-NEXT: |   %ldn = (%A)[%n];
; CHECK-NEXT: |   @llvm.lifetime.end.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.start.p0(-1,  &((%A)[0]));
; CHECK-NEXT: |   (%A)[3] = %t2;
; CHECK-NEXT: |   %ld3 = (%A)[3];
; CHECK-NEXT: |   @llvm.lifetime.end.p0(-1,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   (%A)[4] = %t1;
; CHECK-NEXT: |   (%A)[0] = %t3;
; CHECK-NEXT: |   %ld4 = (%A)[4];
; CHECK-NEXT: |   %ld0 = (%A)[0];
; CHECK-NEXT: |   (%A)[5] = %t3;
; CHECK-NEXT: |   @llvm.lifetime.end.p0(20,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   (%A)[5] = %t2;
; CHECK-NEXT: |   %ld5 = (%A)[5];
; CHECK-NEXT: |   (%A)[6] = %t2;
; CHECK-NEXT: |   %ld6 = (%A)[6];
; CHECK-NEXT: |   @llvm.lifetime.end.p0(20,  &((%A)[0]));
; CHECK-NEXT: |   %add5 = %ldn + %ld3 + %ld4 + %ld0 + %ld5  +  %ld6;
; CHECK-NEXT: + END LOOP


; CHECK: Dump After

; CHECK:      + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK-NEXT: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   (%A)[%n] = %t2;
; CHECK-NEXT: |   %ldn = (%A)[%n];
; CHECK-NEXT: |   @llvm.lifetime.end.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.start.p0(-1,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.end.p0(-1,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.end.p0(20,  &((%A)[0]));
; CHECK-NEXT: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK-NEXT: |   (%A)[5] = %t2;
; CHECK-NEXT: |   %ld5 = (%A)[5];
; CHECK-NEXT: |   (%A)[6] = %t2;
; CHECK-NEXT: |   %ld6 = (%A)[6];
; CHECK-NEXT: |   @llvm.lifetime.end.p0(20,  &((%A)[0]));
; CHECK-NEXT: |   %add5 = %t2 + %ldn + %t1 + %t3 + %ld5  +  %ld6;
; CHECK-NEXT: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(ptr %A, i32 %t1, i32 %t2, i32 %t3, i64 %n) {
entry:
  %bc = bitcast ptr %A to ptr
  %gep0 = getelementptr inbounds i32, ptr %A,  i64 0
  %gep3 = getelementptr inbounds i32, ptr %A,  i64 3
  %gep4 = getelementptr inbounds i32, ptr %A,  i64 4 
  %gep5 = getelementptr inbounds i32, ptr %A,  i64 5
  %gep6 = getelementptr inbounds i32, ptr %A,  i64 6
  %gepn = getelementptr inbounds i32, ptr %A,  i64 %n
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry], [ %iv.inc, %loop]
  call void @llvm.lifetime.start.p0(i64 40, ptr nonnull %bc)
  store i32 %t2, ptr %gepn, align 4
  %ldn = load i32, ptr %gepn, align 4
  call void @llvm.lifetime.end.p0(i64 40, ptr nonnull %bc)
  call void @llvm.lifetime.start.p0(i64 -1, ptr nonnull %bc)
  store i32 %t2, ptr %gep3, align 4
  %ld3 = load i32, ptr %gep3, align 4
  call void @llvm.lifetime.end.p0(i64 -1, ptr nonnull %bc)
  call void @llvm.lifetime.start.p0(i64 40, ptr nonnull %bc)
  store i32 %t1, ptr %gep4, align 4
  store i32 %t3, ptr %gep0, align 4
  %ld4 = load i32, ptr %gep4, align 4
  %ld0 = load i32, ptr %gep0, align 4
  store i32 %t3, ptr %gep5, align 4
  call void @llvm.lifetime.end.p0(i64 20, ptr nonnull %bc)
  call void @llvm.lifetime.start.p0(i64 40, ptr nonnull %bc)
  store i32 %t2, ptr %gep5, align 4
  %ld5 = load i32, ptr %gep5, align 4
  store i32 %t2, ptr %gep6, align 4
  %ld6 = load i32, ptr %gep6, align 4
  call void @llvm.lifetime.end.p0(i64 20, ptr nonnull %bc)
  %add1 = add i32 %ldn, %ld3
  %add2 = add i32 %add1, %ld4
  %add3 = add i32 %add2, %ld0
  %add4 = add i32 %add3, %ld5
  %add5 = add i32 %add4, %ld6
  %iv.inc = add i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  %ld.lcssa = phi i32 [ %add5, %loop ]
  ret i32 %ld.lcssa
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #1 = { argmemonly nounwind }
