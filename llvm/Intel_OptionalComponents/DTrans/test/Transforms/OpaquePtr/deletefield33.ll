; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that memset size is adjusted and fields are removed from both types
%struct.test01t = type { i16, i32 }
%struct.test01 = type { i32, i16, %struct.test01t }
; CHECK: %__DFT_struct.test01 = type { i32, %__DFT_struct.test01t }
; CHECK: %__DFT_struct.test01t = type { i32 }

define i32 @test01(%struct.test01* "intel_dtrans_func_index"="1" %s1, %struct.test01* "intel_dtrans_func_index"="2" %s2) !intel.dtrans.func.type !5 {
  %p1 = bitcast %struct.test01* %s1 to i8*
  %p2 = bitcast %struct.test01* %s2 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p1, i8 0, i64 16, i1 false)
  %p3 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 0
  %r1 = load i32, i32* %p3
  %p4 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 1
  %r2 = load i32, i32* %p4
  %r = add i32 %r1, %r2
  ret i32 %r
}

; CHECK-LABEL: define internal i32 @test01
; CHECK: call void @llvm.memset
; CHECK-SAME: i64 8

; CHECK: getelementptr %__DFT_struct.test01, {{.*}} %s1, i64 0, i32 0
; CHECK: getelementptr %__DFT_struct.test01, {{.*}} %s1, i64 0, i32 1, i32 0

declare !intel.dtrans.func.type !7 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)


!1 = !{i16 0, i32 0}  ; i16
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01t zeroinitializer, i32 0}  ; %struct.test01t
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4, !4}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01t zeroinitializer, i32 2, !1, !2} ; { i16, i32 }
!9 = !{!"S", %struct.test01 zeroinitializer, i32 3, !2, !1, !3} ; { i32, i16, %struct.test01t }

!intel.dtrans.types = !{!8, !9}
