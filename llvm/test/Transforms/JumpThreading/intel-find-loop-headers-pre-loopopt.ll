; CMPLRLLVM-37804, recognize loop headers with freeze|sext|zext|trunc and prevent the threading in pre-loopopt.
; RUN: opt -passes=jump-threading -S < %s | FileCheck %s

declare void @f1()

define void @test1() #0 {
; CHECK-LABEL: @test1(
; CHECK-next:  entry:
; CHECK-next:    br label [[LOOP:%.*]]
; CHECK:       loop:
; CHECK-next:    [[IV:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ [[IV_NEXT:%.*]], [[INC:%.*]] ]
; CHECK-next:    [[IV_SEXT:%.*]] = sext i32 [[IV:%.*]] to i64
; CHECK-next:    switch i64 [[IV_SEXT:%.*]], label [[INC:%.*]] [
; CHECK-next:      i64 0, label [[THEN:%.*]]
; CHECK-next:      i64 199, label [[THEN:%.*]]
; CHECK:       then:
; CHECK-next:    call void @f1()
; CHECK-next:    br label [[INC:%.*]]
; CHECK:       inc:
; CHECK-next:    [[CMP:%.*]] = icmp slt i64 [[IV_SEXT:%.*]], 200
; CHECK-next:    [[IV_NEXT:%.*]] = add nsw i32 [[IV:%.*]], 1
; CHECK-next:    br i1 [[CMP:%.*]], label [[LOOP:%.*]], label [[EXIT:%.*]]
; CHECK:       exit:
; CHECK-next:    ret void
entry:
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %inc]
  %iv.sext = sext i32 %iv to i64
  switch i64 %iv.sext, label %inc [
    i64 0, label %then
    i64 199, label %then
  ]

then:
  call void @f1()
  br label %inc

inc:
  %cmp = icmp slt i64 %iv.sext, 200
  %iv.next = add nsw i32 %iv, 1
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

define void @test2() #0 {
; CHECK-LABEL: @test2(
; CHECK-next:  entry:
; CHECK-next:    br label [[LOOP:%.*]]
; CHECK:       loop:
; CHECK-next:    [[IV:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ [[IV_NEXT:%.*]], [[INC:%.*]] ]
; CHECK-next:    [[IV_ZEXT:%.*]] = zext i32 [[IV:%.*]] to i64
; CHECK-next:    switch i64 [[IV_ZEXT:%.*]], label [[INC:%.*]] [
; CHECK-next:      i64 0, label [[THEN:%.*]]
; CHECK-next:      i64 199, label [[THEN:%.*]]
; CHECK:       then:
; CHECK-next:    call void @f1()
; CHECK-next:    br label [[INC:%.*]]
; CHECK:       inc:
; CHECK-next:    [[CMP:%.*]] = icmp ult i64 [[IV_ZEXT:%.*]], 200
; CHECK-next:    [[IV_NEXT:%.*]] = add nuw i32 [[IV:%.*]], 1
; CHECK-next:    br i1 [[CMP:%.*]], label [[LOOP:%.*]], label [[EXIT:%.*]]
; CHECK:       exit:
; CHECK-next:    ret void
entry:
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %inc]
  %iv.zext = zext i32 %iv to i64
  switch i64 %iv.zext, label %inc [
    i64 0, label %then
    i64 199, label %then
  ]

then:
  call void @f1()
  br label %inc

inc:
  %cmp = icmp ult i64 %iv.zext, 200
  %iv.next = add nsw i32 %iv, 1
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

define void @test3() #0 {
; CHECK-LABEL: @test3(
; CHECK-next:  entry:
; CHECK-next:    br label [[LOOP:%.*]]
; CHECK:       loop:
; CHECK-next:    [[IV:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ [[IV_NEXT:%.*]], [[INC:%.*]] ]
; CHECK-next:    [[IV_FR:%.*]] = freeze i64 [[IV:%.*]]
; CHECK-next:    [[IV_TRUNC:%.*]] = trunc i64 [[IV_FR:%.*]] to i32
; CHECK-next:    switch i32 [[IV_TRUNC:%.*]], label [[INC:%.*]] [
; CHECK-next:      i32 0, label [[THEN:%.*]]
; CHECK-next:      i32 199, label [[THEN:%.*]]
; CHECK:       then:
; CHECK-next:    call void @f1()
; CHECK-next:    br label [[INC:%.*]]
; CHECK:       inc:
; CHECK-next:    [[CMP:%.*]] = icmp ult i32 [[IV_TRUNC:%.*]], 200
; CHECK-next:    [[IV_NEXT:%.*]] = add nuw i64 [[IV_FR:%.*]], 1
; CHECK-next:    br i1 [[CMP:%.*]], label [[LOOP:%.*]], label [[EXIT:%.*]]
; CHECK:       exit:
; CHECK-next:    ret void
entry: 
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %inc]
  %iv.fr = freeze i64 %iv
  %iv.trunc = trunc i64 %iv.fr to i32
  switch i32 %iv.trunc, label %inc [
    i32 0, label %then
    i32 199, label %then
  ]

then:
  call void @f1()
  br label %inc

inc:
  %cmp = icmp ult i32 %iv.trunc, 200
  %iv.next = add nuw i64 %iv.fr, 1
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}

attributes #0 = { "pre_loopopt" }
