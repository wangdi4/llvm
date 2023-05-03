; This test verifies that the indirect call is converted to direct calls
; without fallback case.

; RUN: opt -S -intel-ind-call-force-andersen -intel-ind-call-conv-max-target=2 -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A = type { i64, i32 }
%struct.B = type { i64, i64, i32 }

@glob = external global i32, align 4
@fptr = internal global ptr null, align 8

define i32 @add_fun(ptr %val) {
entry:
  %i = getelementptr inbounds %struct.A, ptr %val, i64 0, i32 1
  %l = load i32, ptr %i, align 4
  ret i32 %l
}

define i32 @sub_fun(ptr %val) {
entry:
  %i = getelementptr inbounds %struct.B, ptr %val, i64 0, i32 2
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
  store ptr @sub_fun, ptr @fptr, align 8
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

; CHECK: .indconv.cmp.add_fun:
; CHECK:   %.indconv.c = icmp eq ptr %1, @add_fun
; CHECK:  br i1 %.indconv.c, label %.indconv.call.add_fun, label %.indconv.call.sub_fun

; CHECK:.indconv.call.add_fun:
; CHECK:  %call.indconv = call i32 @add_fun(ptr null)
; CHECK:  br label %.indconv.sink.

; CHECK:.indconv.call.sub_fun:
; CHECK:  %call.indconv1 = call i32 @sub_fun(ptr null)
; CHECK:  br label %.indconv.sink.

; CHECK:.indconv.sink.:
; CHECK:  %.indconv.ret = phi i32 [ %call.indconv, %.indconv.call.add_fun ], [ %call.indconv1, %.indconv.call.sub_fun ]
