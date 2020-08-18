; RUN: opt -basic-aa -loop-idiom -lir-disable-mem-calls -S %s | FileCheck %s
;
; This test checks that loop idiom recognition pass does not convert loops
; to memcpy/memset calls when -lir-disable-mem-calls is specified.

target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test1(i8* %Base, i64 %Size) {
entry:                                           ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %for.body ]
  %I.0.014 = getelementptr i8, i8* %Base, i64 %indvar
  store i8 0, i8* %I.0.014, align 1
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %Size
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
; CHECK-LABEL: @test1(
; CHECK-NOT: memset
}

define void @test2(i64 %Size) {
entry:
  %Base = alloca i8, i32 10000
  %Dest = alloca i8, i32 10000
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvar = phi i64 [ 0, %entry ], [ %indvar.next, %for.body ]
  %I.0.014 = getelementptr i8, i8* %Base, i64 %indvar
  %DestI = getelementptr i8, i8* %Dest, i64 %indvar
  %V = load i8, i8* %I.0.014, align 1
  store i8 %V, i8* %DestI, align 1
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %Size
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
; CHECK-LABEL: @test2(
; CHECK-NOT: memcpy
}
