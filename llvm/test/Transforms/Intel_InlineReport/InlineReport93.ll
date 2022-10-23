; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that a presplit coroutine does not get inlined.

; CHECK-CL-LABEL: define void @foo(i32 %n)
; CHECK-CL: call void @no_suspends(i32 %n)
; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: no_suspends{{.*}}Unsplit coroutine call
; CHECK-MD-LABEL: define void @foo(i32 %n)
; CHECK-MD: call void @no_suspends(i32 %n)

declare i8* @malloc(i32)
declare void @free(i8*) willreturn
declare void @print(i32)
declare token @llvm.coro.id(i32, i8*, i8*, i8*)
declare i1 @llvm.coro.alloc(token)
declare i32 @llvm.coro.size.i32()
declare i8* @llvm.coro.begin(token, i8*)
declare i8* @llvm.coro.free(token, i8*)
declare i1 @llvm.coro.end(i8*, i1)

define void @foo(i32 %n) {
  call void @no_suspends(i32 %n)
  ret void
}

define void @no_suspends(i32 %n) presplitcoroutine {
entry:
  %id = call token @llvm.coro.id(i32 0, i8* null, i8* null, i8* null)
  %need.dyn.alloc = call i1 @llvm.coro.alloc(token %id)
  br i1 %need.dyn.alloc, label %dyn.alloc, label %coro.begin
dyn.alloc:
  %size = call i32 @llvm.coro.size.i32()
  %alloc = call i8* @malloc(i32 %size)
  br label %coro.begin
coro.begin:
  %phi = phi i8* [ null, %entry ], [ %alloc, %dyn.alloc ]
  %hdl = call noalias i8* @llvm.coro.begin(token %id, i8* %phi)
  br label %body
body:
  call void @print(i32 %n)
  br label %cleanup
cleanup:
  %mem = call i8* @llvm.coro.free(token %id, i8* %hdl)
  %need.dyn.free = icmp ne i8* %mem, null
  br i1 %need.dyn.free, label %dyn.free, label %suspend
dyn.free:
  call void @free(i8* %mem)
  br label %suspend
suspend:
  call i1 @llvm.coro.end(i8* %hdl, i1 false)
  ret void
}
