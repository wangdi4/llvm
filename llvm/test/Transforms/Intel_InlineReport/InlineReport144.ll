; RUN: opt -passes='require<anders-aa>,module(indirectcallconv),print<inline-report>' -disable-output -inline-report=0xe807 -intel-ind-call-force-andersen < %s 2>&1 | FileCheck %s 
; RUN: opt -passes='inlinereportsetup' -inline-report=0xea86 < %s -S | opt -passes='require<anders-aa>,module(indirectcallconv)' -inline-report=0xea86 -intel-ind-call-force-andersen -S | opt -passes='inlinereportemitter' -inline-report=0xea86 -S 2>&1 | FileCheck %s

; Check that an indirect call is deleted by indirect call conversion.

; CHECK-LABEL: COMPILE FUNC: func
; CHECK: DELETE:  {{.*}}Indirect call conversion

@glob = external global i32, align 4
@fptr = internal global void (i32*)* null, align 8

; Function Attrs: norecurse nounwind uwtable
define void @add_fun(i32* nocapture %val)  {
entry:
  %0 = load i32, i32* %val, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %val, align 4
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define void @sub_fun(i32* nocapture %val)  {
entry:
  %0 = load i32, i32* %val, align 4
  %dec = add nsw i32 %0, -1
  store i32 %dec, i32* %val, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define i32 @func(i32* %result)  {
entry:
  %result.addr = alloca i32*, align 8
  store i32* %result, i32** %result.addr, align 8
  %0 = load i32, i32* @glob, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store void (i32*)* @add_fun, void (i32*)** @fptr, align 8
  br label %if.end

if.else:                                          ; preds = %entry
  store void (i32*)* @sub_fun, void (i32*)** @fptr, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %1 = load void (i32*)*, void (i32*)** @fptr, align 8
  %2 = load i32*, i32** %result.addr, align 8
  call void %1(i32* %2)
  ret i32 0
}

