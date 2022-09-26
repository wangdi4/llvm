; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-callinfo -disable-output 2>&1 | FileCheck %s

; This test verifies memory function call info collection for the transforms.

; --------------------------------------------------------------------------
; Tests for memset callinfo
; --------------------------------------------------------------------------

; This test checks when an entire structure is filled by memset.
%struct.test01 = type { i32, i16, i8 }
define void @test01(%struct.test01* %in1) {
  %ptr = bitcast %struct.test01* %in1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)
  ret void
}
; CHECK: Function: test01
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: true
; CHECK:     Type: %struct.test01 = type { i32, i16, i8 }


; This test checks when a multiple of the structure size is used by
; memset, such as for an array of structures.
%struct.test02 = type { i32, i16, i8 }
define void @test02(%struct.test02* %in1) {
  %ptr = bitcast %struct.test02* %in1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 32, i1 false)
  ret void
}
; CHECK: Function: test02
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: true
; CHECK:     Type: %struct.test02 = type { i32, i16, i8 }


; This test checks using memset to clear part of the structure, starting
; from the address of the start of the structure
%struct.test03 = type { i32, i16, i16, i8 }
define void @test03(%struct.test03* %in1) {
  %ptr = bitcast %struct.test03* %in1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 6, i1 false)
  ret void
}
; CHECK: Function: test03
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: false
; CHECK:     FirstField: 0
; CHECK:     LastField:  1
; CHECK:     Type: %struct.test03 = type { i32, i16, i16, i8 }


; This test checks using memset to clear part of the structure, starting
; from the address of a field within the structure, such as done by the
; memcpy optimizer.
%struct.test04 = type { i64, i64, i64, i64, i64 }
define void @test04(%struct.test04* %in1) {
  %faddr = getelementptr inbounds %struct.test04, %struct.test04* %in1, i64 0, i32 0
  %ptr = bitcast i64* %faddr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 24, i1 false)
  ret void
}
; CHECK: Function: test04
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: false
; CHECK:     FirstField: 0
; CHECK:     LastField:  2
; CHECK:     Type: %struct.test04 = type { i64, i64, i64, i64, i64 }


; This test checks using memset to write a subset of fields in the middle of a
; structure such as may have been created by the memcpy optimizer.
%struct.test05 = type { i64, i64, i64, i64, i64 }
define void @test05(%struct.test05* %in1) {
  %faddr = getelementptr inbounds %struct.test05, %struct.test05* %in1, i64 0, i32 1
  %ptr = bitcast i64* %faddr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 24, i1 false)
  ret void
}
; CHECK: Function: test05
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: false
; CHECK:     FirstField: 1
; CHECK:     LastField:  3
; CHECK:     Type: %struct.test05 = type { i64, i64, i64, i64, i64 }


; This test checks using memset to clear the entire the structure, starting
; from the address of the first field within the structure.
%struct.test06 = type { i64, i64, i64, i64, i64 }
define void @test06(%struct.test06* %in1) {
  %faddr = getelementptr inbounds %struct.test06, %struct.test06* %in1, i64 0, i32 0
  %ptr = bitcast i64* %faddr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 40, i1 false)
  ret void
}
; CHECK: Function: test06
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: true
; CHECK:     Type: %struct.test06 = type { i64, i64, i64, i64, i64 }


; This test checks using memset with a pointer to a structure that is a
; structure member field of a parent structure.
; Note, currently for the purpose of tracking the structures impacted, only
; the outer structure is tracked for the callinfo. If transforms in the
; future support operations on nested structures, may need to extend the
; modelling, or internally note that the operation on the outer structure
; is impacting an inner structure type.
%struct.test07.a = type { i32, i32, i32, i32, i32 }
%struct.test07.b = type { i32, %struct.test07.a }
define void @test07(%struct.test07.b* %in1) {
  %faddr = getelementptr inbounds %struct.test07.b, %struct.test07.b* %in1, i64 0, i32 1
  %ptr = bitcast %struct.test07.a* %faddr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 20, i1 false)
  ret void
}
; CHECK: Function: test07
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: false
; CHECK:     FirstField: 1
; CHECK:     LastField:  1
; CHECK:     Type: %struct.test07.b = type { i32, %struct.test07.a }


; This test checks using memset with an array type.
%array.test08 = type [ 25 x i32 ]
define void @test08(%array.test08* %in1) {
  %arr_addr = bitcast %array.test08* %in1 to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %arr_addr, i8 0, i64 100, i1 false)
  ret void
}
; CHECK: Function: test08
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: true
; CHECK:     Type: [25 x i32]{{ *$}}


; --------------------------------------------------------------------------
; Tests for memcpy callinfo
; --------------------------------------------------------------------------

; The test checks calls to memcpy with matched struct pointers that copy
; the entire structure.
%struct.test09 = type { i32, i32 }
define void @test09(%struct.test09* %in1, %struct.test09* %in2) {
  %ptr1 = bitcast %struct.test09* %in1 to i8*
  %ptr2 = bitcast %struct.test09* %in2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 8, i1 false)
  ret void
}
; CHECK: Function: test09
; CHECK: MemfuncInfo:
; CHECK:   Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test09 = type { i32, i32 }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test09 = type { i32, i32 }


; The test checks calls to memcpy with matched struct pointers that copies
; the entire structure starting with starting address of the first field
; within the structure.
%struct.test10 = type { i32, i32, i32, i32 }
define void @test10(%struct.test10* %in1, %struct.test10* %in2) {
  %faddr1 = getelementptr inbounds %struct.test10, %struct.test10* %in1, i64 0, i32 0
  %ptr1 = bitcast i32* %faddr1 to i8*
  %faddr2 = getelementptr inbounds %struct.test10, %struct.test10* %in2, i64 0, i32 0
  %ptr2 = bitcast i32* %faddr2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 16, i1 false)
  ret void
}
; CHECK: Function: test10
; CHECK: MemfuncInfo:
; CHECK:   Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test10 = type { i32, i32, i32, i32 }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test10 = type { i32, i32, i32, i32 }


; The test checks calls to memcpy with matched struct pointers that copy
; a subset of fields of the structure starting with starting address of the
; structure.
%struct.test11 = type { i32, i32, i32, i32 }
define void @test11(%struct.test11* %in1, %struct.test11* %in2) {
  %ptr1 = bitcast %struct.test11* %in1 to i8*
  %ptr2 = bitcast %struct.test11* %in2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 8, i1 false)
  ret void
}
; CHECK: Function: test11
; CHECK: MemfuncInfo:
; CHECK:   Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: false
; CHECK:     FirstField: 0
; CHECK:     LastField:  1
; CHECK:     Type: %struct.test11 = type { i32, i32, i32, i32 }
; CHECK:   Region 1:
; CHECK:     Complete: false
; CHECK:     FirstField: 0
; CHECK:     LastField:  1
; CHECK:     Type: %struct.test11 = type { i32, i32, i32, i32 }


; This test checks using memcpy to copy a subset of fields in the middle of a
; structure
%struct.test12 = type { i64, i64, i64, i64, i64 }
define void @test12(%struct.test12* %in1, %struct.test12* %in2) {
  %faddr1 = getelementptr inbounds %struct.test12, %struct.test12* %in1, i64 0, i32 1
  %ptr1 = bitcast i64* %faddr1 to i8*
  %faddr2 = getelementptr inbounds %struct.test12, %struct.test12* %in2, i64 0, i32 1
  %ptr2 = bitcast i64* %faddr2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 24, i1 false)
  ret void
}
; CHECK: Function: test12
; CHECK: MemfuncInfo:
; CHECK:   Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: false
; CHECK:     FirstField: 1
; CHECK:     LastField:  3
; CHECK:     Type: %struct.test12 = type { i64, i64, i64, i64, i64 }
; CHECK:   Region 1:
; CHECK:     Complete: false
; CHECK:     FirstField: 1
; CHECK:     LastField:  3
; CHECK:     Type: %struct.test12 = type { i64, i64, i64, i64, i64 }


; This test checks using memcpy with an array type.
%array.test13 = type [ 25 x i64 ]
define void @test13(%array.test13* %in1, %array.test13* %in2) {
  %arr_addr1 = bitcast %array.test13* %in1 to i8*
  %arr_addr2 = bitcast %array.test13* %in2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %arr_addr1, i8* %arr_addr2, i64 200, i1 false)
  ret void
}
; CHECK: Function: test13
; CHECK: MemfuncInfo:
; CHECK:   Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: [25 x i64]{{ *$}}
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: [25 x i64]{{ *$}}

; --------------------------------------------------------------------------
; Tests for memmove callinfo
; Memmove analysis shares the same code as the memcpy analysis, so testing
; is limited to basic identification that the callinfo indicates the operation
; is a memmove.

; The test checks calls to memmove with matched struct pointers that copy
; the entire structure.
%struct.test14 = type { i32, i32 }
define void @test14(%struct.test14* %in1, %struct.test14* %in2) {
  %ptr1 = bitcast %struct.test14* %in1 to i8*
  %ptr2 = bitcast %struct.test14* %in2 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 8, i1 false)
  ret void
}
; CHECK: Function: test14
; CHECK: MemfuncInfo:
; CHECK:   Kind: memmove
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test14 = type { i32, i32 }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test14 = type { i32, i32 }

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8* , i8*, i64, i1)
