; This test verifies that the target with the same type as @fptr (@add_fun
; is converted into a direct call, and the fallback case is generated since
; type of @sub_fun is same as @fptr if pointers are treated
; as opaque pointers. @sub_fun is considered as potential target function
; but is not added as possible target since the type of @sub_fun is
; not same as @fptr. @mult_fun is not considered as target as type of
; @mult_fun doesn't match with the call.

; RUN: opt -opaque-pointers=0 -S -intel-ind-call-force-andersen -intel-ind-call-conv-max-target=2 -passes='require<anders-aa>,indirectcallconv' %s | FileCheck %s

%struct.A = type { i64, i32 }
%struct.B = type { i64, i64, i32 }

@glob = external global i32, align 4
@fptr = internal global i32 (%struct.A*)* null, align 8

define i32 @add_fun(%struct.A* %val)  {
entry:
  ret i32 0
}

define i32 @sub_fun(%struct.B* %val)  {
entry:
  ret i32 1
}

define %struct.A* @mult_fun(i32 %val)  {
entry:
  ret %struct.A* null
}

define i32 @func(i32 %in_val)  {
entry:
  %0 = load i32, i32* @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 (%struct.A*)* bitcast(i32 (%struct.B*)* @sub_fun to i32 (%struct.A*)*), i32 (%struct.A*)** @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  %tobool2 = icmp ne i32 %0, 1
  br i1 %tobool2, label %if.else.then, label %if.else.end

if.else.then:
  store i32 (%struct.A*)* @add_fun, i32 (%struct.A*)** @fptr, align 8
  br label %if.end

if.else.end:
  store i32 (%struct.A*)* bitcast(%struct.A* (i32)* @mult_fun to i32 (%struct.A*)*), i32 (%struct.A*)** @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load i32 (%struct.A*)*, i32 (%struct.A*)** @fptr, align 8
  %call = call i32 %1(%struct.A* null)
  ret i32 %call
}

; CHECK: define i32 @func

; Check that the compare with @add_fun was generated
; CHECK: .indconv.cmp.add_fun:
; CHECK:   %.indconv.c = icmp eq i32 (%struct.A*)* %1, @add_fun
; CHECK:   br i1 %.indconv.c, label %.indconv.call.add_fun, label %.indconv.icall.call

; Check that the call to @add_fun was generated
; CHECK: .indconv.call.add_fun:
; CHECK:   %call.indconv = call i32 @add_fun(%struct.A* null)
; CHECK:   br label %.indconv.sink.

; Check that the indirect call is preserved
; CHECK: .indconv.icall.call:
; CHECK:   %call.indconv1 = call i32 %1(%struct.A* null)
; CHECK:   br label %.indconv.sink.

; Check that the PHI node was generated correctly
; CHECK: .indconv.sink.:
; CHECK:   %.indconv.ret = phi i32 [ %call.indconv, %.indconv.call.add_fun ], [ %call.indconv1, %.indconv.icall.call ]
