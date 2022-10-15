; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; In this test, llvm.assume is used in a pattern similar to the expected form
; for checking pointer alignment, but the structure is 4 bytes long so
; qkmalloc may not keep it aligned on an 8-byte boundary.

; CHECK: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { ptr, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], ptr, i32, [4 x i8] }
%__SOADT_EL_class.F = type { ptr, ptr }

%struct.other.outer = type { %struct.other.inner, i8 }

; Alignment is checked on this structure, which may not have 8-byte alignment
%struct.other.inner = type { i32 }

define internal void @test01() !dtrans-soatoaos !0 {
  ret void
}

define internal void @test01b(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !11 {
  %a = getelementptr %struct.other.outer, ptr %p, i64 0, i32 0
  %pti = ptrtoint ptr %a to i64
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
  %mem = call ptr @malloc(i64 24)
  %st = bitcast ptr %mem to ptr
  call void @test01b(ptr %st)
  ret i32 0
}

declare void @llvm.assume(i1)
declare !intel.dtrans.func.type !13  "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
!1 = !{%__SOADT_AR_struct.Arr zeroinitializer, i32 1}  ; %__SOADT_AR_struct.Arr*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"A", i32 4, !5}  ; [4 x i8]
!5 = !{i8 0, i32 0}  ; i8
!6 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}  ; %__SOADT_EL_class.F*
!7 = !{i32 0, i32 1}  ; i32*
!8 = !{float 0.0e+00, i32 1}  ; float*
!9 = !{%struct.other.inner zeroinitializer, i32 0}  ; %struct.other.inner
!10 = !{%struct.other.outer zeroinitializer, i32 1}  ; %struct.other.outer*
!11 = distinct !{!10}
!12 = !{i8 0, i32 1}  ; i8*
!13 = distinct !{!12}
!14 = !{!"S", %__SOADT_class.F zeroinitializer, i32 2, !1, !2} ; { %__SOADT_AR_struct.Arr*, i64 }
!15 = !{!"S", %__SOADT_AR_struct.Arr zeroinitializer, i32 5, !3, !4, !6, !3, !4} ; { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
!16 = !{!"S", %__SOADT_EL_class.F zeroinitializer, i32 2, !7, !8} ; { i32*, float* }
!17 = !{!"S", %struct.other.outer zeroinitializer, i32 2, !9, !5} ; { %struct.other.inner, i8 }
!18 = !{!"S", %struct.other.inner zeroinitializer, i32 1, !3} ; { i32 }

!intel.dtrans.types = !{!14, !15, !16, !17, !18}

