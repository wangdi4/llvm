; This test checks that the Andersen's analysis identifies that %struct.A and
; %struct.A.01 are similar structures, but the indirect call convention runs
; when the limit of targets is reached. The goal of this test is to make sure
; that the targets with the same type as @fptr (@sub_fun and @mult_fun) are
; converted into a direct call, and the fallback case is generated since
; @add_fun is a similar type. The limit for direct calls is set to 2, therefore
; there will be 2 direct calls.

; RUN: opt -S -intel-ind-call-force-andersen -intel-ind-call-conv-max-target=2 -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A = type { %struct.A* ()*, i32 }
%struct.A.01 = type { {}*, i32 }

@glob = external global i32, align 4
@fptr = internal global %struct.A* (i32)* null, align 8

define %struct.A.01* @add_fun(i32 %val)  {
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

define %struct.A* @sub_fun(i32 %val)  {
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

define %struct.A* @mult_fun(i32 %val)  {
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

define i32 @func(i32 %in_val)  {
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

; CHECK: define i32 @func

; Check that the compare with @sub_fun was generated
; CHECK: .indconv.cmp.sub_fun:
; CHECK:   %.indconv.c = icmp eq %struct.A* (i32)* %1, @sub_fun
; CHECK:   br i1 %.indconv.c, label %.indconv.call.sub_fun, label %.indconv.cmp.mult_fun

; Check that the call to @sub_fun was generated
; CHECK: .indconv.call.sub_fun:
; CHECK:   %call.indconv = call %struct.A* @sub_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the compare to @mult_fun was generated
; CHECK: .indconv.cmp.mult_fun:
; CHECK:   %.indconv.c1 = icmp eq %struct.A* (i32)* %1, @mult_fun
; CHECK:   br i1 %.indconv.c1, label %.indconv.call.mult_fun, label %.indconv.icall.call

; Check that the call to @mult_fun was generated
; CHECK: .indconv.call.mult_fun:
; CHECK:   %call.indconv2 = call %struct.A* @mult_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the indirect call is preserved
; CHECK: .indconv.icall.call:
; CHECK:   %call.indconv3 = call %struct.A* %1(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the PHI node was generated correctly
; CHECK: .indconv.sink.:
; CHECK:   %.indconv.ret = phi %struct.A* [ %call.indconv, %.indconv.call.sub_fun ], [ %call.indconv2, %.indconv.call.mult_fun ], [ %call.indconv3, %.indconv.icall.call ]
