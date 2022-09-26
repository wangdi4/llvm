; REQUIRES: asserts

; RUN: opt -opaque-pointers -dtrans-typemetadatareader -dtrans-typemetadatareader-values -dtrans-typemetadatareader-strict-check=false -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-values -dtrans-typemetadatareader-strict-check=false -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the DTrans type metadata reader passes when there is
; missing metadata but the check isn't strict.


%struct.test01 = type { i32, i32 }
%struct.test02 = type { i32 }
%struct.test03 = type { ptr }

define internal i32 @foo(ptr %ptr) {
  %val = getelementptr %struct.test01, ptr %ptr, i32 0, i32 0
  %ld = load i32, ptr %val
  ret i32 %ld
}

define internal i32 @bar(ptr %ptr) {
  %val = getelementptr %struct.test02, ptr %ptr, i32 0, i32 0
  %ld = load i32, ptr %val
  ret i32 %ld
}

define internal ptr @bas(ptr %ptr) {
  %val = getelementptr %struct.test03, ptr %ptr, i32 0, i32 0
  %ld = load ptr, ptr %val
  ret ptr %ld
}

; CHECK: StructTypes
; CHECK: --------------------------------------------------------
; CHECK: StructType: %struct.test01 = type { i32, i32 }

; CHECK: dtrans-typemetadatareader: All structures types  NOT populated
; CHECK: dtrans-typemetadatareader: Errors while checking IR annotations


!intel.dtrans.types = !{ !10 }

!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !11, !11 } ; %struct.test01
!11 = !{i32 0, i32 0 }  ; i32
