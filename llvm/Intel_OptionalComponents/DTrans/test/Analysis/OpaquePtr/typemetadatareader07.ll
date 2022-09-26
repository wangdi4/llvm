; REQUIRES: asserts

; RUN: opt -opaque-pointers -dtrans-typemetadatareader -dtrans-typemetadatareader-values -dtrans-typemetadatareader-strict-check=false -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-values -dtrans-typemetadatareader-strict-check=false -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the DTrans type metadata reader can reconstruct
; missing types when metadata is missing, and there are interdependencies
; between the types.
;
; This case intentionally omit the metadata for %struct.test02 and
; %struct.test03 because these are simple types that can be recreated
; without metadata.

%struct.test01 = type { i32, ptr }
%struct.test02 = type { %struct.test03, i32 }
%struct.test03 = type { i32 }
!intel.dtrans.types = !{ !10 }

!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !11, !12 } ; %struct.test01
!11 = !{i32 0, i32 0 }  ; i32
!12 = !{%struct.test02 zeroinitializer, i32 1}

; CHECK: StructTypes
; CHECK: --------------------------------------------------------
; CHECK: StructType: %struct.test01 = type { i32, %struct.test02* }
; CHECK: StructType: %struct.test02 = type { %struct.test03, i32 }
; CHECK: StructType: %struct.test03 = type { i32 }

; CHECK: dtrans-typemetadatareader: All structures types populated
; CHECK: dtrans-typemetadatareader: NO Errors while checking IR annotations
