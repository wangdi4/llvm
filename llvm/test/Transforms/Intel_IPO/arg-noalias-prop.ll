; RUN: opt -aa-pipeline=basic-aa -passes=arg-noalias-prop -S %s | FileCheck %s

; CHECK: define internal void @callback(i8* nocapture %ID, i32* noalias nocapture %A, i32* noalias nocapture %B)
define internal void @callback(i8* nocapture %ID, i32* nocapture %A, i32* nocapture %B) {
  %X = load i32, i32* %B
  store i32 %X, i32* %A
  ret void
}

define void @func1(i32* noalias %A, i32* noalias %B) {
  tail call void (i32, void (i8*, ...)*, ...) @broker(i32 2, void (i8*, ...)* bitcast (void (i8*, i32*, i32*)* @callback to void (i8*, ...)*), i32* %A, i32* %B)
  ret void
}

define void @func2(i32* noalias %A, i32* noalias %B) {
  tail call void @callback(i8* null, i32* %A, i32* %B)
  ret void
}

define i32 @func3() {
  %A = alloca i32
  %B = alloca i32
  store i32 2, i32* %B
  call void @callback(i8* null, i32* %A, i32* %B)
  %X = load i32, i32* %A
  ret i32 %X
}

define i32 @func4() {
  %A = alloca i32
  %P = tail call noalias i8* @malloc(i64 4)
  %B = bitcast i8* %P to i32*
  store i32 2, i32* %B
  call void @callback(i8* null, i32* %A, i32* %B)
  tail call void @free(i8* %P)
  %X = load i32, i32* %A
  ret i32 %X
}

define void @func5(i32* noalias %A, i32* noalias %B, i1 %cond) {
entry:
  br i1 %cond, label %if, label %else

if:
  tail call void (i32, void (i8*, ...)*, ...) @broker(i32 2, void (i8*, ...)* bitcast (void (i8*, i32*, i32*)* @callback to void (i8*, ...)*), i32* %A, i32* %B)
  br label %done

else:
  tail call void (i32, void (i8*, ...)*, ...) @broker(i32 2, void (i8*, ...)* bitcast (void (i8*, i32*, i32*)* @callback to void (i8*, ...)*), i32* %B, i32* %A)
  br label %done

done:
  ret void
}

define i32 @func6() {
entry:
  %P1 = tail call noalias i8* @malloc(i64 4)
  %P2 = tail call noalias i8* @malloc(i64 4)
  %A = bitcast i8* %P1 to i32*
  %B = bitcast i8* %P2 to i32*
  store i32 2, i32* %B
  store i32 3, i32* %B
  br label %body

body:
  %X = phi i32* [ %A, %entry ], [ %Y, %body ]
  %Y = phi i32* [ %B, %entry ], [ %X, %body ]
  call void @callback(i8* null, i32* %X, i32* %Y)
  %val = load i32, i32* %X
  %cond = icmp ne i32 %val, 0
  br i1 %cond, label %exit, label %body

exit:
  tail call void @free(i8* %P1)
  tail call void @free(i8* %P2)
  %fval = load i32, i32* %A
  ret i32 %fval
}

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

declare !callback !0 void @broker(i32, void (i8*, ...)*, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}
