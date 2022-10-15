; RUN: opt < %s -S -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the transformation occurs when there is a safe use of the
; @llvm.assume intrinsic call for checking that a value is non-null.

; The following pattern should be considered safe:
;  %cmp = icmp eq i32* %x, null
;  %xor = xor i1 %cmp, true
;  call void @llvm.assume(i1 %xor)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { ptr, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], ptr, i32, [4 x i8] }
%__SOADT_EL_class.F = type { ptr, ptr }

@gVar = internal global i32 zeroinitializer

define internal  "intel_dtrans_func_index"="1" ptr @getAddr() !intel.dtrans.func.type !9 {
  ret ptr @gVar
}

define internal void @test01() !dtrans-soatoaos !0 {
  %x = call ptr @getAddr()
  %cmp = icmp eq ptr %x, null
  %xor =   xor i1 %cmp, true
  call void @llvm.assume(i1 %xor)

  ; Test with reversed operands
  %cmp2 = icmp eq ptr null, %x
  %xor2 =   xor i1 true, %cmp2
  call void @llvm.assume(i1 %xor2)
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

declare void @llvm.assume(i1)

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
!9 = distinct !{!7}
!10 = !{!"S", %__SOADT_class.F zeroinitializer, i32 2, !1, !2} ; { %__SOADT_AR_struct.Arr*, i64 }
!11 = !{!"S", %__SOADT_AR_struct.Arr zeroinitializer, i32 5, !3, !4, !6, !3, !4} ; { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
!12 = !{!"S", %__SOADT_EL_class.F zeroinitializer, i32 2, !7, !8} ; { i32*, float* }

!intel.dtrans.types = !{!10, !11, !12}

