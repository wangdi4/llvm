; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; Had a bug when multiple functions were predicated. This tests the bug.

%struct.STACK = type { i32, [10000 x i32] }
; CHECK: @push
define void @push(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}

define void @push1(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push2(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push3(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push4(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push5(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push6(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push7(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push8(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push9(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}

define void @push10(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}
define void @push11(%struct.STACK* nocapture %ps, i32 %x) nounwind {
entry:
  %0 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 0 ; <i32*> %[#uses=2]
  %1 = load i32* %0, align 4                      ; <i32> [#uses=3]
  %2 = icmp eq i32 %1, 10000                      ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb

bb:                                               ; preds = %entry
  %3 = sext i32 %1 to i64                         ; <i64> [#uses=1]
  %4 = getelementptr inbounds %struct.STACK* %ps, i64 0, i32 1, i64 %3 ; <i32*> [#uses=1]
  store i32 %x, i32* %4, align 4
  %5 = add nsw i32 %1, 1                          ; <i32> [#uses=1]
  store i32 %5, i32* %0, align 4
  ret void

return:                                           ; preds = %entry
  ret void
}

