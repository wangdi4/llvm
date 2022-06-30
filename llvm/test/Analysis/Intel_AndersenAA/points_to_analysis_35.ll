; This test verifies that %test1 in "baz" is pointing to %mem in "bar"
; by handling "call void @foo(%"MCELFStreamer"* %i)", which is called
; through alias, correctly.

; RUN: opt < %s -passes='require<anders-aa>' -print-anders-points-to -disable-output 2>&1 | FileCheck %s

; CHECK: [1] baz:test1     --> ({{[0-9]+}}): bar:mem

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"MCStreamer" = type  { i32, i8, i32 }
%"MCELFStreamer" = type { %"MCObjectStreamer" }
%"MCObjectStreamer" = type { i64, %"class.std::unique_ptr" }
%"class.std::unique_ptr" = type { %"MCAssembler"* }
%"MCAssembler" = type { i32, i8, i32 }

@foo = alias void (%"MCELFStreamer"*), bitcast (void (%"MCELFStreamer"*)* @bar to void (%"MCELFStreamer"*)*)

define void @bar(%"MCELFStreamer"* %0) {
exit:
  %head = getelementptr inbounds %"MCELFStreamer", %"MCELFStreamer"* %0, i64 0, i32 0, i32 1, i32 0
  %mem = tail call i8* @_Znwm(i64 10)
  %cp = bitcast i8* %mem to %"MCAssembler"*
  store %"MCAssembler"* %cp, %"MCAssembler"** %head, align 4
  ret void
}

declare i8* @_Znwm(i64)

define %"MCStreamer"* @baz() {
entry:
  %call = tail call i8* @_Znwm(i64 0)
  %i = bitcast i8* %call to %"MCELFStreamer"*
  call void @foo(%"MCELFStreamer"* %i)
  br label %exit2

exit2: ; preds = %entry
  br label %exit0

exit0: ; preds = %exit2
  br label %exit1

exit1: ; preds = %exit0
  br label %if.then

if.then:                                          ; preds = %exit1
  %_M_head = getelementptr inbounds %"MCELFStreamer", %"MCELFStreamer"* %i, i64 0, i32 0, i32 1, i32 0
  %test1 = load %"MCAssembler"*, %"MCAssembler"** %_M_head, align 8
  %test2 = getelementptr inbounds %"MCAssembler", %"MCAssembler"* %test1, i64 0, i32 1
  store i8 0, i8* %test2, align 4
  ret %"MCStreamer"* null
}
