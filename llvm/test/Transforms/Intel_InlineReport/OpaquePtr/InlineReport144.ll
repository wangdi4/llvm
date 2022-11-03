; RUN: opt -opaque-pointers -passes='require<anders-aa>,module(indirectcallconv),print<inline-report>' -disable-output -inline-report=0xe807 -intel-ind-call-force-andersen < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xea86 < %s -S | opt -passes='require<anders-aa>,module(indirectcallconv)' -inline-report=0xea86 -intel-ind-call-force-andersen -S | opt -passes='inlinereportemitter' -inline-report=0xea86 -S 2>&1 | FileCheck %s

; Check that an indirect call is deleted by indirect call conversion.

; CHECK-LABEL: COMPILE FUNC: func
; CHECK: DELETE:  {{.*}}Indirect call conversion

@glob = external global i32, align 4
@fptr = internal global ptr null, align 8

define void @add_fun(ptr nocapture %val) {
entry:
  %i = load i32, ptr %val, align 4
  %inc = add nsw i32 %i, 1
  store i32 %inc, ptr %val, align 4
  ret void
}

define void @sub_fun(ptr nocapture %val) {
entry:
  %i = load i32, ptr %val, align 4
  %dec = add nsw i32 %i, -1
  store i32 %dec, ptr %val, align 4
  ret void
}

define i32 @func(ptr %result) {
entry:
  %result.addr = alloca ptr, align 8
  store ptr %result, ptr %result.addr, align 8
  %i = load i32, ptr @glob, align 4
  %tobool = icmp ne i32 %i, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store ptr @add_fun, ptr @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  store ptr @sub_fun, ptr @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %i1 = load ptr, ptr @fptr, align 8
  %i2 = load ptr, ptr %result.addr, align 8
  call void %i1(ptr %i2)
  ret i32 0
}
