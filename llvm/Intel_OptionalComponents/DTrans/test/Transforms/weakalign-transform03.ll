; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=false 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=false 2>&1 | FileCheck %s

; Test that the mallopt call gets inserted at the start of main when
; all the safety checks and heuristics pass. In particular, this
; test is for a safe use of llvm.assume.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
%__SOADT_EL_class.F = type { i32*, float* }

%struct.other.outer = type { %struct.other.inner, i8 }
%struct.other.inner = type { %struct.other*, i64 }
%struct.other = type { i64, i64 }

define internal void @test01() !dtrans-soatoaos !0 {
  %tt = tail call i1 @llvm.type.test(i8* null, metadata !"typeId")
  tail call void @llvm.assume(i1 %tt)
  ret void
}

define internal void @test01b(%struct.other.outer* %p) {
  %a = getelementptr %struct.other.outer, %struct.other.outer* %p, i64 0, i32 0
  %pti = ptrtoint %struct.other.inner* %a to i64
  %masked = and i64 %pti, 7
  %aligned = icmp eq i64 %masked, 0
  tail call void @llvm.assume(i1 %aligned)
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

declare i1 @llvm.type.test(i8* , metadata)
declare void @llvm.assume(i1)

; CHECK-LABEL: define i32 @main()
; CHECK-NEXT: call i32 @mallopt(i32 3225, i32 0)
; CHECK-NEXT: call void @test01()

!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
