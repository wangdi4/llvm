; This test verifies that anders-aa should be able to disambiguate stdout
; (__acrt_iob_func(i32 1)) and local allocated pointer (i.e %p1) when
; whole_program_safe is true.

; RUN: opt < %s -whole-program-assume -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27035"

%struct._iobuf = type { ptr }

; TODO: the store in @bar is not getting analyzed.
; Removing it allows a noalias result.
; CHECK: MayAlias:        ptr* %gep, ptr* %p1

@plist = internal unnamed_addr global ptr null, align 8

define dso_local void @bar() {
  %call1 = tail call noalias ptr @malloc(i64 40)
  %1 = bitcast ptr %call1 to ptr
  store ptr %1, ptr @plist
  ret void
}

define dso_local i32 @main() {
entry:
  %p0 = tail call ptr @__acrt_iob_func(i32 1)
  %gep = getelementptr inbounds %struct._iobuf, ptr %p0, i32 0, i32 0
  %ld.gep = load ptr, ptr %gep
  %call0 = tail call i32 @fflush(ptr %p0)
  tail call void @bar()
  %p1 = load ptr, ptr @plist
  %ld.p1 = load ptr, ptr %p1
  ret i32 0
}

declare dso_local i32 @fflush(ptr)
declare noalias ptr @malloc(i64)
declare dso_local ptr @__acrt_iob_func(i32)
