; This test verifies that all references of empty function ??__FgArrayM@@YAXXZ
; are removed (including atexit call). instcombine and inline passes are
; needed to prove that ??__FgArrayM@@YAXXZ is empty function.

; RUN: opt < %s -S -passes='function(instcombine),cgscc(inline),globalopt' | FileCheck %s

; CHECK: @"?gArrayM@@3VMArrayImpl@1@A" = dso_local global %MArrayImpl
; CHECK-NOT: "??__FgArrayM@@YAXXZ"
; CHECK-NOT: call i32 @atexit

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%"MArrayImpl" = type { %"M" }
%"M" = type { i32 (...)** }

$"??_7MArrayImpl@@6B@" = comdat largest
@"??_7MArrayImpl@@6B@" = unnamed_addr alias i8*, getelementptr inbounds ({ [4 x i8*] }, { [4 x i8*] }* null, i32 0, i32 0, i32 1)

@"?gArrayM@@3VMArrayImpl@1@A" = dso_local global %"MArrayImpl" zeroinitializer, align 8
@"?fgArrayM@XMLPlatformUtils@@2PEAVM@2@EA" = dso_local global %"M"* getelementptr inbounds (%"MArrayImpl", %"MArrayImpl"* @"?gArrayM@@3VMArrayImpl@1@A", i32 0, i32 0), align 8
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_P.cpp, i8* null }]

define internal void @"??__EgArrayM@@YAXXZ"() {
entry:
  %call = call %"MArrayImpl"* @"??0MArrayImpl@@QEAA@XZ"(%"MArrayImpl"* @"?gArrayM@@3VMArrayImpl@1@A")
  %0 = call i32 @atexit(void ()* @"??__FgArrayM@@YAXXZ")
  ret void
}

define internal void @"??__FgArrayM@@YAXXZ"() {
entry:
  call void bitcast (void (%"M"*)* @"??1M@@UEAA@XZ" to void (%"MArrayImpl"*)*)(%"MArrayImpl"* @"?gArrayM@@3VMArrayImpl@1@A")
  ret void
}

define linkonce_odr dso_local void @"??1M@@UEAA@XZ"(%"M"* %this) unnamed_addr align 2 {
entry:
  ret void
}

define internal void @_GLOBAL__sub_I_P.cpp() {
entry:
  call void @"??__EgArrayM@@YAXXZ"()
  ret void
}

define linkonce_odr dso_local %"MArrayImpl"* @"??0MArrayImpl@@QEAA@XZ"(%"MArrayImpl"* returned %this) unnamed_addr align 2 {
entry:
  %0 = getelementptr %"MArrayImpl", %"MArrayImpl"* %this, i64 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** @"??_7MArrayImpl@@6B@" to i32 (...)**), i32 (...)*** %0, align 8
  ret %"MArrayImpl"* %this
}

declare dso_local i32 @atexit(void ()*)
