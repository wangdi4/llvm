; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that a presplit coroutine does not get inlined.

; CHECK-CL-LABEL: define void @foo(i32 %n)
; CHECK-CL: call void @no_suspends(i32 %n)
; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: no_suspends{{.*}}Unsplit coroutine call
; CHECK-MD-LABEL: define void @foo(i32 %n)
; CHECK-MD: call void @no_suspends(i32 %n)

declare ptr @malloc(i32)
declare void @free(ptr) willreturn
declare void @print(i32)
declare token @llvm.coro.id(i32, ptr, ptr, ptr)
declare i1 @llvm.coro.alloc(token)
declare i32 @llvm.coro.size.i32()
declare ptr @llvm.coro.begin(token, ptr)
declare ptr @llvm.coro.free(token, ptr)
declare i1 @llvm.coro.end(ptr, i1)

define void @foo(i32 %n) {
  call void @no_suspends(i32 %n)
  ret void
}

define void @no_suspends(i32 %n) presplitcoroutine {
entry:
  %id = call token @llvm.coro.id(i32 0, ptr null, ptr null, ptr null)
  %need.dyn.alloc = call i1 @llvm.coro.alloc(token %id)
  br i1 %need.dyn.alloc, label %dyn.alloc, label %coro.begin
dyn.alloc:
  %size = call i32 @llvm.coro.size.i32()
  %alloc = call ptr @malloc(i32 %size)
  br label %coro.begin
coro.begin:
  %phi = phi ptr [ null, %entry ], [ %alloc, %dyn.alloc ]
  %hdl = call noalias ptr @llvm.coro.begin(token %id, ptr %phi)
  br label %body
body:
  call void @print(i32 %n)
  br label %cleanup
cleanup:
  %mem = call ptr @llvm.coro.free(token %id, ptr %hdl)
  %need.dyn.free = icmp ne ptr %mem, null
  br i1 %need.dyn.free, label %dyn.free, label %suspend
dyn.free:
  call void @free(ptr %mem)
  br label %suspend
suspend:
  call i1 @llvm.coro.end(ptr %hdl, i1 false)
  ret void
}
