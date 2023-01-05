; This test checks that the Andersen analysis identifies that %struct.A and
; %struct.A.01 are similar structures. Then, run the indirect call conversion.
; The result should be a branch calling the function @sub_fun because the type
; is the same as @fptr, and the fallback case because the type of @add_fun is
; similar to @fptr.

; RUN: opt -S -intel-ind-call-force-andersen -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A = type { %struct.A* ()*, i32 }
%struct.A.01 = type { {}*, i32 }

@glob = external global i32, align 4
@fptr = internal global %struct.A* (i32)* null, align 8

; Function Attrs: norecurse nounwind uwtable
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

; Function Attrs: norecurse nounwind uwtable
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

; Function Attrs: nounwind uwtable
define i32 @func(i32 %in_val)  {
entry:
  %0 = load i32, i32* @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store %struct.A* (i32)* bitcast(%struct.A.01* (i32)* @add_fun to %struct.A* (i32)*), %struct.A* (i32)** @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  store %struct.A* (i32)* @sub_fun, %struct.A* (i32)** @fptr, align 8
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

; Check that the compare instruction was generated correctly
; CHECK: .indconv.cmp.sub_fun:
; CHECK:   %.indconv.c = icmp eq %struct.A* (i32)* %1, @sub_fun
; CHECK:   br i1 %.indconv.c, label %.indconv.call.sub_fun, label %.indconv.icall.call


; Check that @fptr was converted into @sub_fun correctly
; CHECK: .indconv.call.sub_fun:
; CHECK:   %call.indconv = call %struct.A* @sub_fun(i32 %in_val)
; CHECK:   br label %.indconv.sink.

; Check that the fallback case was generated since %struct.A.01
; is similar to %struct.A
; CHECK: .indconv.icall.call:
; CHECK:   %call.indconv1 = call %struct.A* %1(i32 %in_val)
; CHECK:   br label %.indconv.sink

; Check that the PHI node was constructed correctly
; CHECK: .indconv.sink.:
; CHECK:   %.indconv.ret = phi %struct.A* [ %call.indconv, %.indconv.call.sub_fun ], [ %call.indconv1, %.indconv.icall.call ]
