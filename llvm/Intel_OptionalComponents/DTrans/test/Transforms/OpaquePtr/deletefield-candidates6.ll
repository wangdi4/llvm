; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -debug-only=dtrans-deletefieldop -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks the delete fields candidate selection for nested
; structures.
;
; In this case, the fields 0 and 2 in %struct.B can be deleted even though it
; is enclosed in %struct.A.

%struct.A = type { i32, %struct.B, i32 }
%struct.B = type { i8, i16, i32 }

define i32 @foo(%struct.A* "intel_dtrans_func_index"="1" %a, i32** "intel_dtrans_func_index"="2" %q) !intel.dtrans.func.type !7 {
entry:
  %p = bitcast %struct.A* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 16, i1 false)

  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 2
  %0 = load i32, i32* %x, align 4
  %1 = load i32, i32* %y, align 4
  %2 = add i32 %0, %1

  store i32* %x, i32** %q

  ret i32 %2
}

define i16 @bar(%struct.B* "intel_dtrans_func_index"="1" %b) !intel.dtrans.func.type !9 {
entry:
  %y = getelementptr inbounds %struct.B, %struct.B* %b, i64 0, i32 1
  %0 = load i16, i16* %y, align 4
  ret i16 %0
}

declare !intel.dtrans.func.type !11 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

; CHECK: Delete field for opaque pointers: looking for candidate structures
; CHECK: Selected for deletion: %struct.B

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.B zeroinitializer, i32 0}  ; %struct.B
!3 = !{i8 0, i32 0}  ; i8
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.A zeroinitializer, i32 1}  ; %struct.A*
!6 = !{i32 0, i32 2}  ; i32**
!7 = distinct !{!5, !6}
!8 = !{%struct.B zeroinitializer, i32 1}  ; %struct.B*
!9 = distinct !{!8}
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10}
!12 = !{!"S", %struct.A zeroinitializer, i32 3, !1, !2, !1} ; { i32, %struct.B, i32 }
!13 = !{!"S", %struct.B zeroinitializer, i32 3, !3, !4, !1} ; { i8, i16, i32 }

!intel.dtrans.types = !{!12, !13}
