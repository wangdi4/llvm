; RUN: opt < %s -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=false 2>&1 | FileCheck %s

; Test that the mallopt call gets inserted at the start of main when
; all the safety checks and heuristics pass.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { ptr, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], ptr, i32, [4 x i8] }
%__SOADT_EL_class.F = type { ptr, ptr }

define internal void @test01() !dtrans-soatoaos !0 {
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

; CHECK-LABEL: define i32 @main()
; CHECK-NEXT: call i32 @mallopt(i32 3225, i32 0)
; CHECK-NEXT: call void @test01()

!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
!1 = !{%__SOADT_AR_struct.Arr zeroinitializer, i32 1}  ; %__SOADT_AR_struct.Arr*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"A", i32 4, !5}  ; [4 x i8]
!5 = !{i8 0, i32 0}  ; i8
!6 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}  ; %__SOADT_EL_class.F*
!7 = !{i32 0, i32 1}  ; i32*
!8 = !{float 0.0e+00, i32 1}  ; float*
!9 = !{!"S", %__SOADT_class.F zeroinitializer, i32 2, !1, !2} ; { %__SOADT_AR_struct.Arr*, i64 }
!10 = !{!"S", %__SOADT_AR_struct.Arr zeroinitializer, i32 5, !3, !4, !6, !3, !4} ; { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
!11 = !{!"S", %__SOADT_EL_class.F zeroinitializer, i32 2, !7, !8} ; { i32*, float* }

!intel.dtrans.types = !{!9, !10, !11}

