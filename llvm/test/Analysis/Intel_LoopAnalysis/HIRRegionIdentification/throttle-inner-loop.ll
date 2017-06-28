; REQUIRES: asserts
; RUN: opt < %s -analyze -hir-region-identification -debug-only=hir-region-identification  2>&1 | FileCheck %s

; Check that we suppress the inner loop which turns into a non-countable loop when the outer loop is suppressed.

; CHECK: Cannot handle inner loopnest.
; CHECK-NOT: Region 1


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-12002e2.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct.(anonymous namespace)::Int" = type { i32 }

; Function Attrs: norecurse uwtable
define void @main() local_unnamed_addr personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  invoke void @_Znwm()
          to label %for.body unwind label %lpad252

for.body:                              ; preds = %entry, %exit
  %incdec.ptr = phi i32* [ %incdec.ptr.i.i, %exit ], [ undef, %entry ]
  br label %while.body

while.body:                         ; preds = %while.body, %for.body
  %0 = phi %"struct.(anonymous namespace)::Int"* [ %incdec.ptr.i, %while.body ], [ undef, %for.body ]
  %1 = phi i32* [ %incdec.ptr.i.i.i, %while.body ], [ %incdec.ptr, %for.body ]
  %incdec.ptr.i.i.i = getelementptr inbounds i32, i32* %1, i64 -1
  %incdec.ptr.i = getelementptr inbounds %"struct.(anonymous namespace)::Int", %"struct.(anonymous namespace)::Int"* %0, i64 -1
  %cmp = icmp ne i32* %incdec.ptr.i.i.i, undef
  %cmp.i = icmp ne %"struct.(anonymous namespace)::Int"* %incdec.ptr.i, undef
  %lnot = and i1 %cmp.i, %cmp
  br i1 %lnot, label %while.body, label %exit

exit: ; preds = %while.body
  %incdec.ptr.i.i = getelementptr inbounds i32, i32* %incdec.ptr, i64 1
  br i1 undef, label %for.body, label %invoke.cont364.loopexit

invoke.cont364.loopexit:                          ; preds = %exit
  ret void

lpad252:                                          ; preds = %entry
  %2 = landingpad { i8*, i32 }
          cleanup
  resume { i8*, i32 } undef
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nobuiltin
declare void @_Znwm() local_unnamed_addr 


!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
