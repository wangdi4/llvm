; RUN: opt -codegenprepare -S -mtriple=x86_64 < %s | FileCheck %s

; Test that an invalid CFG is not created by splitIndirectCriticalEdges
; transformation when the 'target' block is a loop to itself.

; CHECK: .split:
; INTEL_CUSTOMIZATION
; Critical edges splitting algorithm is changed, clone BB isn't generated anymore.
; CHECK: br label %.split
; CHECK: if.else1:
; CHECK: indirectbr
; end INTEL_CUSTOMIZATION

define void @test() {
entry:
  br label %if.else

if.else:
  br i1 undef, label %while.body, label %preheader

preheader:
  br label %if.else1

if.then:
  unreachable

while.body:
  %dest.sroa = phi i32 [ %1, %while.body ], [ undef, %if.else1 ], [ undef, %if.else ]
  %0 = inttoptr i32 %dest.sroa to ptr
  %incdec.ptr = getelementptr inbounds i8, ptr %0, i32 -1
  %1 = ptrtoint ptr %incdec.ptr to i32
  store i8 undef, ptr %incdec.ptr, align 1
  br label %while.body

if.else1:
  indirectbr ptr undef, [label %if.then, label %while.body, label %if.else, label %if.else1]
}

