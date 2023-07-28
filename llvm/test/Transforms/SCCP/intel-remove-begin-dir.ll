; RUN: opt < %s -passes=sccp -S | FileCheck %s

; Verify that @llvm.directive.region.entry() is also removed when we fold known
; true/false conditional branches making 'exit' block with
; @llvm.directive.region.exit() unreachable.

; CHECK: pre:
; CHECK-NEXT: br label %loop

; CHECK: store i32 0, ptr %gep
; CHECK-NEXT: br label %loop

; CHECK-NOT: exit:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal global [10 x i32] zeroinitializer, align 16

define void @foo_() {
entry:
  br label %pre

pre:                                              ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  br i1 false, label %exit, label %loop

loop:                                              ; preds = %loop, %pre
  %iv = phi i64 [ 1, %pre ], [ %add.1, %loop ]
  %add.1 = add nsw i64 %iv, 1
  %gep = getelementptr inbounds [10 x i32], ptr @A, i32 0, i64 %iv
  store i32 0, ptr %gep, align 4
  br i1 true, label %loop, label %exit

exit:                   ; preds = %pre, %loop
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

