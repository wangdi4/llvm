; This test verifies that @fptr is converted to direct call with same type
; target function (i.e @add_fun) without fallback case.

; RUN: opt -S -intel-ind-call-force-andersen -intel-ind-call-conv-max-target=2 -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A = type { i64, i32 }

@glob = external global i32, align 4
@fptr = internal global ptr null, align 8

define i32 @add_fun(ptr %val) {
entry:
  %i = getelementptr inbounds %struct.A, ptr %val, i64 0, i32 1
  %l = load i32, ptr %i, align 4
  ret i32 %l
}

define ptr @mult_fun(i32 %val) {
entry:
  ret ptr null
}

define i32 @func(i32 %in_val) {
entry:
  %0 = load i32, ptr @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  %tobool2 = icmp ne i32 %0, 1
  br i1 %tobool2, label %if.else.then, label %if.else.end

if.else.then:                                     ; preds = %if.else
  store ptr @add_fun, ptr @fptr, align 8
  br label %if.end

if.else.end:                                      ; preds = %if.else
  store ptr @mult_fun, ptr @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else.end, %if.else.then, %if.then
  %1 = load ptr, ptr @fptr, align 8
  %call = call i32 %1(ptr null)
  ret i32 %call
}

; CHECK:  %call = call i32 @add_fun(ptr null)
; CHECK:  ret i32 %call
