; REQUIRES: asserts

; This is same as ind_call_conv_4.ll except this test checks for
; debug dump.

; This test verifies that the indirect call is converted into @sub_fun
; direct call since type of @sub_fun is same as the call and a fallback
; case is generated as one of the possible targets @add_fun is marked as
; VarArg.

; RUN: opt -opaque-pointers < %s -disable-output -intel-ind-call-force-andersen -debug-only=intel-ind-call-conv -passes='require<anders-aa>,indirectcallconv' 2>&1 | FileCheck %s

%struct.A = type { %struct.A* ()*, i32 }
%struct.A.01 = type { {}*, i32 }

@glob = external global i32, align 4
@fptr = internal global %struct.A* (i32)* null, align 8

define %struct.A.01* @add_fun(i32 %val, ...) {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A.01, align 4
  store i32 %val, i32* %val.addr, align 4
  %0 = load i32, i32* %val.addr, align 4
  %add = add nsw i32 %0, 1
  %b = getelementptr inbounds %struct.A.01, %struct.A.01* %ret, i32 0, i32 1
  store i32 %add, i32* %b, align 4
  ret %struct.A.01* %ret
}

define %struct.A* @sub_fun(i32 %val) {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A, align 4
  store i32 %val, i32* %val.addr, align 4
  %0 = load i32, i32* %val.addr, align 4
  %sub = add nsw i32 %0, -1
  %b = getelementptr inbounds %struct.A, %struct.A* %ret, i32 0, i32 1
  store i32 %sub, i32* %b, align 4
  ret %struct.A* %ret
}

define i32 @func(i32 %in_val) {
entry:
  %0 = load i32, i32* @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store %struct.A* (i32)* bitcast(%struct.A.01* (i32)* @add_fun to %struct.A* (i32)*), %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %tobool2 = icmp ne i32 %0, 1
  br i1 %tobool2, label %if.else.then, label %if.end

if.else.then:
  store %struct.A* (i32)* @sub_fun, %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.else.then
  %1 = load %struct.A* (i32)*, %struct.A* (i32)** @fptr, align 8
  %call = call %struct.A* %1(i32 %in_val)
  %out = alloca %struct.A*, align 8
  store %struct.A* %call, %struct.A** %out, align 8
  %2 = load %struct.A*, %struct.A** %out, align 8
  %b = getelementptr inbounds %struct.A, %struct.A* %2, i32 0, i32 1
  %3 = load i32, i32* %b, align 4
  ret i32 %3
}

; CHECK: Call-Site:    %call = call ptr %1(i32 %in_val)
; CHECK:    Unsafe target: Skipping  add_fun
; CHECK:    (Incomplete set)
; CHECK:    sub_fun
