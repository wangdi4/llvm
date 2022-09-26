; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that we correctly handle bitcasts involving vtables.

%A = type { i32 (...)**, i32, i32 }
%B = type { %A, i32, i32 }
%C = type { %B }

define void @testA(%A* %a) {
  %vtable = bitcast %A* %a to void (%A*)***
  ret void
}

define void @testB(%B* %b) {
  %vtable = bitcast %B* %b to void (%B*)***
  ret void
}

define void @testC(%C* %c) {
  ; This happens if C doesn't override anything in the vtable.
  %vtable = bitcast %C* %c to void (%B*)***
  ret void
}

define void @testAllocatedC() {
  %p = call i8* @malloc(i64 24)
  %c = bitcast i8* %p to %C*
  %vtableB = bitcast i8* %p to void (%B*)***
  %vtableA = bitcast i8* %p to void (%A*)***
  ret void
}

; CHECK:  LLVMType: %A = type { i32 (...)**, i32, i32 }
; CHECK:  Safety data: Nested structure | Has vtable

; CHECK:  LLVMType: %B = type { %A, i32, i32 }
; CHECK:  Safety data: Nested structure | Contains nested structure

; CHECK:  LLVMType: %C = type { %B }
; CHECK:  Safety data: Contains nested structure

declare i8* @malloc(i64)
