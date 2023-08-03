; This test checks that indirect call is converted to direct calls (@add_fun
; and @sub_fun) since types of both target functions match with type of the
; call.
; Typed pointers vs Opaque pointers: This is the case where generated code
; is different between Opaque pointers and typed pointers. With typed pointers,
; @sub_fun is considered as possible target because type of @sub_fun is same
; as call and @add_fun is NOT considered as possible target but @add_fun is
; treated as similar function and generates fallback case.

; RUN: opt -S -intel-ind-call-force-andersen -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A.01 = type { ptr, i32 }
%struct.A = type { ptr, i32 }

@glob = external global i32, align 4
@fptr = internal global ptr null, align 8

define ptr @add_fun(i32 %val) {
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
  store ptr @sub_fun, ptr @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
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

; Check that the compare instruction was generated correctly
; CHECK: .indconv.cmp.add_fun:
; CHECK:   %.indconv.c = icmp eq ptr %1, @add_fun
; CHECK:   br i1 %.indconv.c, label %.indconv.call.add_fun, label %.indconv.call.sub_fun

; Check that @fptr was converted into @add_fun correctly
; CHECK: .indconv.call.add_fun:
; CHECK:   %call.indconv = call ptr @add_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that @fptr was converted into @sub_fun correctly
; CHECK: .indconv.call.sub_fun:
; CHECK:   %call.indconv1 = call ptr @sub_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the PHI node was constructed correctly
; CHECK: .indconv.sink.:
; CHECK:   %.indconv.ret = phi ptr [ %call.indconv, %.indconv.call.add_fun ], [ %call.indconv1, %.indconv.call.sub_fun ]
