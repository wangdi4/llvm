; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; In this test, llvm.assume is used in a pattern similar to the expected form
; for checking pointer alignment, but the structure is 4 bytes long so
; qkmalloc may not keep it aligned on an 8-byte boundary.

; CHECK: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
%__SOADT_EL_class.F = type { i32*, float* }

%struct.other.outer = type { %struct.other.inner, i8 }

; Alignment is checked on this structure, which may not have 8-byte alignment
%struct.other.inner = type { i32 }

define internal void @test01() !dtrans-soatoaos !0 {
  ret void
}

define internal void @test01b(%struct.other.outer* %p) {
  %a = getelementptr %struct.other.outer, %struct.other.outer* %p, i64 0, i32 0
  %pti = ptrtoint %struct.other.inner* %a to i64
  %masked = and i64 %pti, 7
  %aligned = icmp eq i64 %masked, 0

  ; We would not expect to see an assume to exist in compiler generated
  ; IR for this, but make sure that the analysis will treat it as unsafe,
  ; just in case.
  tail call void @llvm.assume(i1 %aligned)
  ret void
}

define i32 @main() {
  call void @test01()
  %mem = call i8* @malloc(i64 24)
  %st = bitcast i8* %mem to %struct.other.outer*
  call void @test01b(%struct.other.outer* %st)
  ret i32 0
}

declare void @llvm.assume(i1)
declare i8* @malloc(i64)

!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
