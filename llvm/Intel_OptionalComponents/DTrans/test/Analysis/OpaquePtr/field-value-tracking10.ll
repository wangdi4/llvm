; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that a call to memcpy does not change the field value info
; for calls that are safe. For cases that are not safe, the value
; info should be changed to be 'incomplete'.

; Copy an entire structure.
%struct.test01 = type { i32, i32, i32 }
@glob01 = internal global %struct.test01 { i32 1, i32 2, i32 3 }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !3 {
  %pDst = bitcast ptr %pStructA to ptr
  %pSrc = bitcast ptr %pStructB to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %pDst, ptr %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK:     Single Value: i32 1
; CHECK: 1)Field LLVM Type: i32
; CHECK:     Single Value: i32 2
; CHECK: 2)Field LLVM Type: i32
; CHECK:     Single Value: i32 3
; CHECK: Safety data: Global instance | Has initializer list{{ *}}
; CHECK: End LLVMType: %struct.test01


; Copy a subset of structure fields, starting from a GEP of field 0.
%struct.test02 = type { i32, i32, i32 }
@glob02 = internal global %struct.test02 { i32 1, i32 2, i32 3 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !5 {
  %pFieldA = getelementptr %struct.test02, ptr %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test02, ptr %pStructB, i64 0, i32 0
  %pDst = bitcast ptr %pFieldA to ptr
  %pSrc = bitcast ptr %pFieldB to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %pDst, ptr %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK:     Single Value: i32 1
; CHECK: 1)Field LLVM Type: i32
; CHECK:     Single Value: i32 2
; CHECK: 2)Field LLVM Type: i32
; CHECK:     Single Value: i32 3
; CHECK: Safety data: Global instance | Has initializer list | Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test02


; Copy a subset of the structure, with a size that does not end on a field
; boundary.
%struct.test03 = type { i32, i32, i32, i32, i32 }
@glob03 = internal global %struct.test03 { i32 1, i32 2, i32 3, i32 4, i32 5 }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !7 {
  %pFieldA = getelementptr %struct.test03, ptr %pStructA, i64 0, i32 2
  %pFieldB = getelementptr %struct.test03, ptr %pStructB, i64 0, i32 2
  %pDst = bitcast ptr %pFieldA to ptr
  %pSrc = bitcast ptr %pFieldB to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %pDst, ptr %pSrc, i64 6, i1 false)
  ret void
}
; Currently, the analysis conservatively marks all fields as 'incomplete' for
; this case when 'bad memfunc size' is set. This can be improved in the future
; to only apply to selected fields.

; CHECK-LABEL: LLVMType: %struct.test03
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 1 ] <incomplete>
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 2 ] <incomplete>
; CHECK:   2)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 3 ] <incomplete>
; CHECK:   3)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 4 ] <incomplete>
; CHECK:   4)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 5 ] <incomplete>
; CHECK: Safety data: Global instance | Has initializer list | Bad memfunc size{{ *}}
; CHECK: End LLVMType: %struct.test03

; Copy from one structure type to another. This is not supported by DTrans as a
; simplification for what the transformations need to handle. This should result
; in the field value info of the destination structure being marked as
; 'incomplete'. The source structure should still track the values from the
; initializer because the memcpy is not writing them.
%struct.test04a = type { i32, i32, i32 }
%struct.test04b = type { i32, i32, i32 }
@glob04a = internal global %struct.test04a { i32 1, i32 2, i32 3 }
@glob04b = internal global %struct.test04b { i32 4, i32 5, i32 6 }
define void @test04(ptr "intel_dtrans_func_index"="1" %pStructA, ptr "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !10 {
  %pFieldA = getelementptr %struct.test04a, ptr %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test04b, ptr %pStructB, i64 0, i32 0
  %pDst = bitcast ptr %pFieldA to ptr
  %pSrc = bitcast ptr %pFieldB to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %pDst, ptr %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04a
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 1 ] <incomplete>
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 2 ] <incomplete>
; CHECK:   2)Field LLVM Type: i32
; CHECK:     Multiple Value: [ 3 ] <incomplete>
; CHECK:   Safety data: Global instance | Has initializer list | Bad memfunc manipulation{{ *}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: LLVMType: %struct.test04b
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 4
; CHECK:   1)Field LLVM Type: i32
; CHECK:     Single Value: i32 5
; CHECK:   2)Field LLVM Type: i32
; CHECK:     Single Value: i32 6
; CHECK:   Safety data: Global instance | Has initializer list | Bad memfunc manipulation{{ *}}
; CHECK: End LLVMType: %struct.test04b

declare !intel.dtrans.func.type !12 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4, !4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6, !6}
!8 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!9 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!10 = distinct !{!8, !9}
!11 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!11, !11}
!13 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!14 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!15 = !{!"S", %struct.test03 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!16 = !{!"S", %struct.test04a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!17 = !{!"S", %struct.test04b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!13, !14, !15, !16, !17}
