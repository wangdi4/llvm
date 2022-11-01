; REQUIRES: asserts

; This test checks that the Andersen's analysis identifies that %struct.A and
; %struct.A.01 are similar structures, but the indirect call convention runs
; when the limit of targets is reached. The goal of this test is to make sure
; that the trace is printed correctly.

; RUN: opt < %s -intel-ind-call-force-andersen -debug-only=intel-ind-call-conv -passes='require<anders-aa>,indirectcallconv' 2>&1 | FileCheck %s

%struct.A = type { %struct.A* ()*, i32 }
%struct.A.01 = type { {}*, i32 }

@glob = external global i32, align 4
@fptr = internal global %struct.A* (i32)* null, align 8

define %struct.A.01* @add_fun(i32 %val) {
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

define %struct.A* @mult_fun(i32 %val) {
entry:
  %val.addr = alloca i32, align 4
  %ret = alloca %struct.A, align 4
  store i32 %val, i32* %val.addr, align 4
  %0 = load i32, i32* %val.addr, align 4
  %mul = mul nsw i32 %0, -1
  %b = getelementptr inbounds %struct.A, %struct.A* %ret, i32 0, i32 1
  store i32 %mul, i32* %b, align 4
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
  br i1 %tobool2, label %if.else.then, label %if.else.end

if.else.then:
  store %struct.A* (i32)* @sub_fun, %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.else.end:
  store %struct.A* (i32)* @mult_fun, %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load %struct.A* (i32)*, %struct.A* (i32)** @fptr, align 8
  %call = call %struct.A* %1(i32 %in_val)
  %out = alloca %struct.A*, align 8
  store %struct.A* %call, %struct.A** %out, align 8
  %2 = load %struct.A*, %struct.A** %out, align 8
  %b = getelementptr inbounds %struct.A, %struct.A* %2, i32 0, i32 1
  %3 = load i32, i32* %b, align 4
  ret i32 %3
}


; CHECK: Call-Site:    %call = call %struct.A* %1(i32 %in_val)

; Check that @add_fun was indentified as a similar type
; CHECK:     Types might be similar: Ignoring add_fun

; Check that the set is partially complete
; CHECK:     (Partially complete set)

; Check that @sub_fun and @mult_fun where identified as targets
; CHECK:     sub_fun
; CHECK:     mult_fun
