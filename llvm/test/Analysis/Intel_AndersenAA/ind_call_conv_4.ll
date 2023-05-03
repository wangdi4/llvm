; This test verifies that the indirect call is converted into @sub_fun
; direct call since type of @sub_fun is same as the call and a fallback
; case is generated as one of the possible targets @add_fun is marked as
; VarArg.

; RUN: opt -S -intel-ind-call-force-andersen -intel-ind-call-conv-max-target=2 -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A.01 = type { ptr, i32 }
%struct.A = type { ptr, i32 }

@glob = external global i32, align 4
@fptr = internal global ptr null, align 8

define ptr @add_fun(i32 %val, ...) {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A.01, align 4
  store i32 %val, ptr %val.addr, align 4
  %0 = load i32, ptr %val.addr, align 4
  %add = add nsw i32 %0, 1
  %b = getelementptr inbounds %struct.A.01, ptr %ret, i32 0, i32 1
  store i32 %add, ptr %b, align 4
  ret ptr %ret
}

define ptr @sub_fun(i32 %val) {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A, align 4
  store i32 %val, ptr %val.addr, align 4
  %0 = load i32, ptr %val.addr, align 4
  %sub = add nsw i32 %0, -1
  %b = getelementptr inbounds %struct.A, ptr %ret, i32 0, i32 1
  store i32 %sub, ptr %b, align 4
  ret ptr %ret
}

define i32 @func(i32 %in_val) {
entry:
  %0 = load i32, ptr @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store ptr @add_fun, ptr @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %tobool2 = icmp ne i32 %0, 1
  br i1 %tobool2, label %if.else.then, label %if.end

if.else.then:                                     ; preds = %if.else
  store ptr @sub_fun, ptr @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else.then, %if.then
  %1 = load ptr, ptr @fptr, align 8
  %call = call ptr %1(i32 %in_val)
  %out = alloca ptr, align 8
  store ptr %call, ptr %out, align 8
  %2 = load ptr, ptr %out, align 8
  %b = getelementptr inbounds %struct.A, ptr %2, i32 0, i32 1
  %3 = load i32, ptr %b, align 4
  ret i32 %3
}

; CHECK: define i32 @func

; CHECK: .indconv.cmp.sub_fun:
; CHECK:   %.indconv.c = icmp eq ptr %1, @sub_fun
; CHECK:   br i1 %.indconv.c, label %.indconv.call.sub_fun, label %.indconv.icall.call

; CHECK: .indconv.call.sub_fun:
; CHECK:   %call.indconv = call ptr @sub_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; CHECK: .indconv.icall.call:
; CHECK:   %call.indconv1 = call ptr %1(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; CHECK: .indconv.sink.:
; CHECK:   %.indconv.ret = phi ptr [ %call.indconv, %.indconv.call.sub_fun ], [ %call.indconv1, %.indconv.icall.call ]

