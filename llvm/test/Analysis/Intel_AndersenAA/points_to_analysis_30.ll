; This test verifies that anders-aa should be able to disambiguate stdout
; (__acrt_iob_func(i32 1)) and local allocated pointer (i.e %p1) when
; whole_program_safe is true.

; RUN: opt < %s -whole-program-assume -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27035"

%struct._iobuf = type { i8* }

; TODO: the store in @bar is not getting analyzed.
; Removing it allows a noalias result.
; CHECK: MayAlias:        i8** %gep, i32** %p1

@plist = internal unnamed_addr global i32** null, align 8

define dso_local void @bar() {
  %call1 = tail call noalias i8* @malloc(i64 40)
  %1 = bitcast i8* %call1 to i32**
  store i32** %1, i32*** @plist
  ret void
}

define dso_local i32 @main() {
entry:
  %p0 = tail call %struct._iobuf* @__acrt_iob_func(i32 1)
  %gep = getelementptr inbounds %struct._iobuf, %struct._iobuf* %p0, i32 0, i32 0
  %ld.gep = load i8*, i8** %gep
  %call0 = tail call i32 @fflush(%struct._iobuf* %p0)
  tail call void @bar()
  %p1 = load i32**, i32*** @plist
  %ld.p1 = load i32*, i32** %p1
  ret i32 0
}

declare dso_local i32 @fflush(%struct._iobuf*)
declare noalias i8* @malloc(i64)
declare dso_local %struct._iobuf* @__acrt_iob_func(i32)
