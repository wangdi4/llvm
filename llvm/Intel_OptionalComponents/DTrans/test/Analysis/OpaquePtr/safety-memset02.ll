; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test the special case where a structure begins with an array of i8 elements,
; whose address is passed to memset. This case is special because the address of
; the array is the same as the address of the entire structure and the type of a
; GEP to the start of the array element matches the type expected for parameter to
; memset. When opaque pointers are introduced this pattern may be seen for other
; types of array elements.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with memset that writes the entire structure, starting from the GEP of
; array element 0.
%struct.test01 = type { [200 x i8], i32, i32, i32, i32 }
@var01 = internal global %struct.test01 zeroinitializer
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !5 {
  call void @llvm.memset.p0i8.i64(i8* getelementptr (%struct.test01, %struct.test01* @var01, i64 0, i32 0, i32 0),
                                  i8 1, i64 216, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: [200 x i8]
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Test with memset that just writes the array elements.
%struct.test02 = type { [200 x i8], i32, i32, i32, i32 }
@var02 = internal global %struct.test02 zeroinitializer
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  call void @llvm.memset.p0i8.i64(i8* getelementptr (%struct.test02, %struct.test02* @var02, i64 0, i32 0, i32 0),
                                  i8 1, i64 200, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: [200 x i8]
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Global instance | Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test02


; Test with memset that only writes some of the array elements. This is not
; supported by DTrans because the analysis only supports complete field writes.
%struct.test03 = type { [200 x i8], i32, i32, i32, i32 }
@var03 = internal global %struct.test03 zeroinitializer
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !9 {
  call void @llvm.memset.p0i8.i64(i8* getelementptr (%struct.test03, %struct.test03* @var03, i64 0, i32 0, i32 0),
                                  i8 1, i64 100, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Global instance | Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test03


declare !intel.dtrans.func.type !11 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{!"A", i32 200, !2}  ; [200 x i8]
!2 = !{i8 0, i32 0}  ; i8
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10}
!12 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }
!13 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }
!14 = !{!"S", %struct.test03 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }

!intel.dtrans.types = !{!12, !13, !14}
