; REQUIRES: asserts

; RUN: opt -opaque-pointers -dtrans-typemetadatareader -dtrans-typemetadatareader-values -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-values -disable-output < %s 2>&1 | FileCheck %s

; This test is to check that structure types can be reconstructed based on metadata.

; Test simple structure
%struct.test01 = type { i32, i32 }
; CHECK: StructType: %struct.test01 = type { i32, i32 }

; Test structure with pointers and pointers to pointers
; The metadata will be able to produce the typed pointer form.
%struct.test02 = type { ptr, ptr }
; CHECK: StructType: %struct.test02 = type { i64**, i8* }

; Test structure which also contains pointers to structures
%struct.test03 = type { ptr, ptr, ptr }
; CHECK: StructType: %struct.test03  = type { %struct.test01*, %struct.test02*, %struct.test03* }

; Test structure with nested structures. For this structure, it is not
; required that metadata is present, but for now we will include it for
; completeness.
%struct.test04 = type { %struct.test01, %struct.test02 }
; CHECK: StructType: %struct.test04 = type { %struct.test01, %struct.test02 }

; Test structure with function pointer containing pointer to structure type, and void type.
%struct.test05 = type { i32, ptr }
; CHECK: StructType: %struct.test05 = type { i32, void (%struct.test05*)* }

; Test structure with arrays, and arrays of pointers
%struct.test06 = type {[ 256 x i8], ptr, [64 x ptr] }
; CHECK: StructType: %struct.test06 = type { [256 x i8], [128 x i16]*, [64 x i8**] }

; Test structure with literal struct
%struct.test07 = type { i32, { double, i16 } }
; CHECK: StructType: %struct.test07 = type { i32, { double, i16 } }

; Test with opaque structure definition
%struct.test08 = type opaque
; CHECK: StructType: %struct.test08 = type opaque

; Test with empty structure definition
%struct.test09 = type {}
; CHECK: StructType: %struct.test09 = type {}

; Test with base/derived class with vtable
%struct.test10base = type { ptr }
%struct.test10derived = type { %struct.test10base }
; CHECK: StructType: %struct.test10base = type { i32 (...)** }
; CHECK: StructType: %struct.test10derived = type { %struct.test10base }

; Test with vector type
%struct.test11 = type { <4 x i32> }
; CHECK: StructType: %struct.test11 = type { <4 x i32> }

!intel.dtrans.types = !{ !10, !20, !30, !40, !50, !60, !70, !80, !90, !100, !105, !110 }

!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !11, !11 } ; %struct.test01
!11 = !{i32 0, i32 0 }  ; i32

!20 = !{!"S", %struct.test02 zeroinitializer, i32 2, !21, !22 } ; %struct.test02
!21 = !{i64 0, i32 2}  ; i64**
!22 = !{i8 0, i32 1}   ; i8*

!30 = !{!"S", %struct.test03 zeroinitializer, i32 3, !31, !32, !33 } ; %struct.test03
!31 = !{%struct.test01 zeroinitializer, i32 1 } ; %struct.test01*
!32 = !{%struct.test02 zeroinitializer, i32 1 } ; %struct.test02*
!33 = !{%struct.test03 zeroinitializer, i32 1 } ; %struct.test03*

!40 = !{!"S", %struct.test04 zeroinitializer, i32 2, !41, !42 } ; %struct.test04
!41 = !{%struct.test01 zeroinitializer, i32 0 } ; %struct.test01
!42 = !{%struct.test02 zeroinitializer, i32 0 } ; %struct.test02

!50 = !{!"S", %struct.test05 zeroinitializer, i32 2, !51, !52 } ; %struct.test05
!51 = !{i32 0, i32 0 }                                ; i32
!52 = !{!53, i32 1 }                                  ; void (%struct.test05*)*
!53 = !{!"F", i1 false, i32 1, !54, !55 }             ; void (%struct.test05*)
!54 = !{!"void", i32 0 }                              ; void
!55 = !{%struct.test05 zeroinitializer, i32 1 } ; %struct.test05*

!60 = !{!"S", %struct.test06 zeroinitializer, i32 3, !61, !63, !66} ; %struct.test06
!61 = !{!"A", i32 256, !62} ; [256 x i8]
!62 = !{i8 0, i32 0}        ; i8
!63 = !{!64, i32 1 }        ; [128 x i16]*
!64 = !{!"A", i32 128, !65} ; [128 x i16]
!65 = !{i16 0, i32 0}       ; i16
!66 = !{!"A", i32 64, !67}  ; [64 x i8**]
!67 = !{i8 0, i32 2}        ; i8**

!70 = !{!"S", %struct.test07 zeroinitializer, i32 2, !71, !72} ; %struct.test07
!71 = !{i32 0, i32 0}      ; i32
!72 = !{!"L", i32 2, !73, !74}  ; { double, i16}
!73 = !{double 0.0, i32 0}  ; double
!74 = !{i16 0, i32 0}       ; i16

!80 = !{!"S", %struct.test08 zeroinitializer, i32 -1 } ; %struct.test08

!90 = !{!"S", %struct.test09 zeroinitializer, i32 0 } ; %struct.test09

!100 = !{!"S", %struct.test10base zeroinitializer, i32 1, !101} ; %struct.test10base
!101 = !{!102, i32 2 }                                           ; i32 (...)**
!102 = !{!"F", i1 true, i32 0, !11}                              ; i32 (...)
!105 = !{!"S", %struct.test10derived zeroinitializer, i32 1, !106} ; %struct.test10derived
!106 = !{%struct.test10base zeroinitializer, i32 0 }       ; %struct.test10base

!110 = !{!"S", %struct.test11 zeroinitializer, i32 1, !111 }     ; %struct.test11
!111 = !{!"V", i32 4, !11}    ; <4 x i32>
