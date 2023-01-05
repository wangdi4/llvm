; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; In this test, llvm.assume is used in a pattern similar to the expected form
; for checking pointer alignment, but the mask value is not a pointer
; alignment mask.


; CHECK: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { ptr, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], ptr, i32, [4 x i8] }
%__SOADT_EL_class.F = type { ptr, ptr }

%struct.other.outer = type { %struct.other.inner, i8 }
%struct.other.inner = type { ptr, i64 }
%struct.other = type { i64, i64 }

define internal void @test01() !dtrans-soatoaos !0 {
  ret void
}

define internal void @test01b(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !12 {
  %a = getelementptr %struct.other.outer, ptr %p, i64 0, i32 0
  %pti = ptrtoint ptr %a to i64
  ; The bitmask pattern here is not an allowed value for the transformation.
  %masked = and i64 %pti, 11
  %aligned = icmp eq i64 %masked, 0
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
declare !intel.dtrans.func.type !14  "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

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
!10 = !{%struct.other zeroinitializer, i32 1}  ; %struct.other*
!11 = !{%struct.other.outer zeroinitializer, i32 1}  ; %struct.other.outer*
!12 = distinct !{!11}
!13 = !{i8 0, i32 1}  ; i8*
!14 = distinct !{!13}
!15 = !{!"S", %__SOADT_class.F zeroinitializer, i32 2, !1, !2} ; { %__SOADT_AR_struct.Arr*, i64 }
!16 = !{!"S", %__SOADT_AR_struct.Arr zeroinitializer, i32 5, !3, !4, !6, !3, !4} ; { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
!17 = !{!"S", %__SOADT_EL_class.F zeroinitializer, i32 2, !7, !8} ; { i32*, float* }
!18 = !{!"S", %struct.other.outer zeroinitializer, i32 2, !9, !5} ; { %struct.other.inner, i8 }
!19 = !{!"S", %struct.other.inner zeroinitializer, i32 2, !10, !2} ; { %struct.other*, i64 }
!20 = !{!"S", %struct.other zeroinitializer, i32 2, !2, !2} ; { i64, i64 }

!intel.dtrans.types = !{!15, !16, !17, !18, !19, !20}

