; It checks indirectcallconv converts indirect call fptr() to direct
; calls add_fun() and sub_fun().
; RUN: opt -S -intel-ind-call-force-andersen -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

; CHECK: define i32 @func
; CHECK: call void @add_fun(
; CHECK:  call void @sub_fun(

@glob = external global i32, align 4
@fptr = internal global ptr null, align 8

define void @add_fun(ptr nocapture %val) {
entry:
  %0 = load i32, ptr %val, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr %val, align 4
  ret void
}

define void @sub_fun(ptr nocapture %val) {
entry:
  %0 = load i32, ptr %val, align 4
  %dec = add nsw i32 %0, -1
  store i32 %dec, ptr %val, align 4
  ret void
}

define i32 @func(ptr %result) {
entry:
  %result.addr = alloca ptr, align 8
  store ptr %result, ptr %result.addr, align 8
  %0 = load i32, ptr @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store ptr @add_fun, ptr @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  store ptr @sub_fun, ptr @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load ptr, ptr @fptr, align 8
  %2 = load ptr, ptr %result.addr, align 8
  call void %1(ptr %2)
  ret i32 0
}
