; REQUIRES: asserts

; RUN: opt -dtrans-typemetadatareader -dtrans-typemetadatareader-values -dtrans-typemetadatareader-strict-check=false -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-values -dtrans-typemetadatareader-strict-check=false -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the DTrans type metadata reader passes when there is
; missing metadata but the check isn't strict.


%struct.test01 = type { i32, i32 }
%struct.test02 = type { i32 }
%struct.test03 = type { i8* }

define internal i32 @foo(%struct.test01* %ptr) {
  %val = getelementptr %struct.test01, %struct.test01* %ptr, i32 0, i32 0
  %ld = load i32, i32* %val
  ret i32 %ld
}

define internal i32 @bar(%struct.test02* %ptr) {
  %val = getelementptr %struct.test02, %struct.test02* %ptr, i32 0, i32 0
  %ld = load i32, i32* %val
  ret i32 %ld
}

define internal i8* @bas(%struct.test03* %ptr) {
  %val = getelementptr %struct.test03, %struct.test03* %ptr, i32 0, i32 0
  %ld = load i8*, i8** %val
  ret i8* %ld
}

; CHECK: StructTypes
; CHECK: --------------------------------------------------------
; CHECK: StructType: %struct.test01 = type { i32, i32 }

; CHECK: dtrans-typemetadatareader: All structures types  NOT populated
; CHECK: dtrans-typemetadatareader: Errors while checking IR annotations


!intel.dtrans.types = !{ !10 }

!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !11, !11 } ; %struct.test01
!11 = !{i32 0, i32 0 }  ; i32
