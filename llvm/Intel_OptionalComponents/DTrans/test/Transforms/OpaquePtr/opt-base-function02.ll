; RUN: opt -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01a < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the remapping process for a function also causes
; the metadata type information to be updated with the remapped
; types.

%struct.test01a = type { i32, i32, i32 }

define void @test01() {
  %local_ptr1a = alloca ptr, align 8, !intel_dtrans_type !2
  %local_ptrptr1a = alloca ptr, align 8, !intel_dtrans_type !3
  %local_array_ptr1a = alloca [4 x ptr], align 8, !intel_dtrans_type !4
  ret void
}

; CHECK-LABEL: define void @test01()


; These checks can replace the above checks when opaque pointers are in use.
; CHECK: %local_ptr1a = alloca ptr, align 8, !intel_dtrans_type ![[PTR_S01A:[0-9]+]]
; CHECK: %local_ptrptr1a = alloca ptr, align 8, !intel_dtrans_type ![[PTRPTR_S01A:[0-9]+]]
; CHECK: %local_array_ptr1a = alloca [4 x ptr], align 8, !intel_dtrans_type ![[ARRAY_4xPTRPTRPTR_S01A:[0-9]+]]

; The metadata should be the same with or without opaque pointers.
; CHECK: ![[PTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 1}
; CHECK: ![[PTRPTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 2}
; CHECK: ![[ARRAY_4xPTRPTRPTR_S01A]] = !{!"A", i32 4, ![[PTRPTRPTR_S01A:[0-9]+]]}
; CHECK: ![[PTRPTRPTR_S01A]] = !{%__DTT_struct.test01a zeroinitializer, i32 3}

!intel.dtrans.types = !{!6}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!3 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!4 = !{!"A", i32 4, !5}  ; [4 x %struct.test01a***]
!5 = !{%struct.test01a zeroinitializer, i32 3}  ; %struct.test01a***
!6 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
