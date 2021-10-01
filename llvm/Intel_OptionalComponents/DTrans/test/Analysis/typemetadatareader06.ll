; REQUIRES: asserts

; RUN: not --crash opt -dtrans-typemetadatareader -dtrans-typemetadatareader-strict-check=true -disable-output < %s 2>&1 | FileCheck %s --allow-empty
; RUN: not --crash opt -passes=dtrans-typemetadatareader -dtrans-typemetadatareader-strict-check=true -disable-output < %s 2>&1 | FileCheck %s --allow-empty

; This test checks that the DTrans type metadata reader didn't pass since
; the check is strict. Test case should crash.

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

; CHECK-NOT: StructType: %struct.test01 = type { i32, i32 }


!intel.dtrans.types = !{ !10 }

!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !11, !11 } ; %struct.test01
!11 = !{i32 0, i32 0 }  ; i32
