; RUN: opt -opaque-pointers=0 -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that the test compiles successfully.
; Compiler was choking during constant array simplification of
; (@operators)[0][0].0 which yields constant GEP expr which are not yet
; supported by the utilities.


; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %ld = (@operators)[0][0].0;
; CHECK: |   %res = @strcmp(&((%ld)[0]),  &((%ref.ptr)[0]));
; CHECK: |   %liveout.cmp = %res == 0;
; CHECK: |   %ld = (@operators)[0][1].0;
; CHECK: |   %res = @strcmp(&((%ld)[0]),  &((%ref.ptr)[0]));
; CHECK: |   %liveout.cmp = %res == 0;
; CHECK: + END LOOP


%struct.anon.1024 = type { i8*, i32, i32, i32 }

@.str.16.1422 = private unnamed_addr constant [3 x i8] c"||\00", align 1
@.str.17.1423 = private unnamed_addr constant [3 x i8] c"&&\00", align 1

@operators = internal unnamed_addr constant [2 x %struct.anon.1024] [%struct.anon.1024 { i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.16.1422, i32 0, i32 0), i32 2, i32 1, i32 8 }, %struct.anon.1024 { i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.17.1423, i32 0, i32 0), i32 2, i32 2, i32 7 }], align 16

define void @foo(i8* %ref.ptr) {
entry:
  br label %outerloop

outerloop:                                              ; preds = %entry
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %outerlatch ]
  br label %loop

loop:                                              ; preds = %outerloop, %loop
  %constgep = phi %struct.anon.1024* [ %constgep.inc, %loop ], [ getelementptr inbounds ([2 x %struct.anon.1024], [2 x %struct.anon.1024]* @operators, i64 0, i64 0), %outerloop ]
  %inner.iv = phi i64 [ %inner.iv.next, %loop ], [ 0, %outerloop ]
  %gep = getelementptr inbounds %struct.anon.1024, %struct.anon.1024* %constgep, i64 0, i32 0
  %ld = load i8*, i8** %gep, align 8
  %res = tail call i32 @strcmp(i8* nonnull %ld, i8* nonnull dereferenceable(1) %ref.ptr) #54
  %liveout.cmp = icmp eq i32 %res, 0
  %inner.iv.next = add nuw nsw i64 %inner.iv, 1
  %constgep.inc = getelementptr inbounds [2 x %struct.anon.1024], [2 x %struct.anon.1024]* @operators, i64 0, i64 %inner.iv.next
  %cmp1 = icmp eq i64 %inner.iv.next, 2
  br i1 %cmp1, label %outerlatch, label %loop

outerlatch:
  %out = phi i1 [ %liveout.cmp, %loop ]
  %iv.next = add nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.next, 100
  br i1 %cmp, label %exit, label %outerloop

exit:
  %out1 = phi i1 [ %out, %outerlatch ]
  ret void
}

declare dso_local i32 @strcmp(i8* nocapture, i8* nocapture) #0

attributes #0 = { argmemonly nofree nounwind readonly }
