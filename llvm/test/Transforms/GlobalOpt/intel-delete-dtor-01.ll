; This test verifies that all references of empty function ??__FgArrayM@@YAXXZ
; are removed (including atexit call).

; RUN: opt < %s -S -passes='globalopt' | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

; CHECK: @"?gArrayM@@3VMArrayImpl@1@A" = dso_local global %MArrayImpl
; CHECK-NOT: "??__FgArrayM@@YAXXZ"
; CHECK-NOT: call i32 @atexit

%"MArrayImpl" = type { %"M" }
%"M" = type { ptr }

$"??_7MArrayImpl@@6B@" = comdat largest
@"??_7MArrayImpl@@6B@" = unnamed_addr alias ptr, getelementptr inbounds ({ [4 x ptr] }, ptr null, i32 0, i32 0, i32 1)

@"?gArrayM@@3VMArrayImpl@1@A" = dso_local global %"MArrayImpl" zeroinitializer, align 8
@"?fgArrayM@XMLPlatformUtils@@2PEAVM@2@EA" = dso_local global ptr getelementptr inbounds (%"MArrayImpl", ptr @"?gArrayM@@3VMArrayImpl@1@A", i32 0, i32 0), align 8
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_P.cpp, ptr null }]

define internal void @"??__FgArrayM@@YAXXZ"() {
entry:
  ret void
}

define internal void @_GLOBAL__sub_I_P.cpp() {
entry:
  store ptr bitcast (ptr @"??_7MArrayImpl@@6B@" to ptr), ptr getelementptr inbounds (%MArrayImpl, ptr @"?gArrayM@@3VMArrayImpl@1@A", i64 0, i32 0, i32 0), align 8
  %0 = call i32 @atexit(ptr @"??__FgArrayM@@YAXXZ")
  ret void
}

define linkonce_odr dso_local ptr @"??0MArrayImpl@@QEAA@XZ"(ptr returned %this) unnamed_addr align 2 {
entry:
  %0 = getelementptr %"MArrayImpl", ptr %this, i64 0, i32 0, i32 0
  store ptr bitcast (ptr @"??_7MArrayImpl@@6B@" to ptr), ptr %0, align 8
  ret ptr %this
}

declare dso_local i32 @atexit(ptr)
