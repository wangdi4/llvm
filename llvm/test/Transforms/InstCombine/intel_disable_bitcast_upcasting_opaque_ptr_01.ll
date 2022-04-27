; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test case checks that load instruction simplification run if two
; instructions are loading the same address using different types and
; one of the instructions is loading an opaque pointer.

; RUN: opt -opaque-pointers -instcombine -disable-combine-upcasting=true < %s -S 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -instcombine -disable-combine-upcasting=false < %s -S 2>&1 | FileCheck %s

; ModuleID = 'intel_disable_bitcast_upcasting_01.ll'
source_filename = "intel_disable_bitcast_upcasting_01.ll"

%class.Inner1 = type <{ [ 4 x i32 ], i32 }>
%class.Inner2 = type <{ [ 8 x i32 ], i32 }>
%class.Outer2 = type { %class.Inner1, %class.Inner2 }
%class.Outer1 = type { ptr }

declare void @bar(ptr)

declare void @baz(i64)

define void @foo(ptr %0, i32 %1) {
  %tmp0 = getelementptr inbounds %class.Outer1, ptr %0, i64 0, i32 0
  %tmp1 = load ptr, ptr %0, align 8
  %tmp2 = load i64, ptr %tmp0, align 8

  call void @bar(ptr %tmp1)
  call void @baz(i64 %tmp2)
  ret void
}

; Make sure that the load in %tmp2 is converted as a ptrtoint instruction.
; CHECK: %tmp1 = load ptr, ptr %0, align 8
; CHECK: %tmp2.cast = ptrtoint ptr %tmp1 to i64

; end INTEL_FEATURE_SW_DTRANS