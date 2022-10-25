; RUN: opt -S -opaque-pointers -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s
; RUN: opt -S -opaque-pointers -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s
; RUN: opt -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that DTrans can detect the opaque pointers are present in the IR without
; requiring the termporary -opaque-pointer flag.
; When -whole-program-assume -intel-libirc-allowed is enabled, the opaque pointers are detected during
; the PointerTypeAnalyzer execution. Without whole-program-assume, the passes that
; use the base class are responsible for determining whether to configure the
; base class for opaque pointers.

%struct.test01a = type { i32, i32, i32 }
; CHECK: %__DTT_struct.test01a = type { i32, i32, i32 }

define void @test01() {
  %a = alloca ptr
  ret void
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!2}
