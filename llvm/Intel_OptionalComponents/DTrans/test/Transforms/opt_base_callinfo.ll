; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -disable-output -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.type01,struct.type02 -debug-only=dtrans-optbase 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -disable-output -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.type01,struct.type02 -debug-only=dtrans-optbase 2>&1 | FileCheck %s

%struct.noclonetype01 = type { i32, i32 }
%struct.type01 = type { i32, i16, i8 }
%struct.type01dep = type { %struct.type01*, %struct.type01* }
%struct.type02 = type { i32, i32, i32, i32 }

; This test verifies the types stored in the callinfo
; get updated after the type remapping occurs.
define void @test01() {
  %call = call i8* @malloc(i64 64)
  %pts = bitcast i8* %call to %struct.type01*
  %ptv = bitcast %struct.type01* %pts to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK: Function: test01
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }
; CHECK: Function: test01
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; This test verifies non-remapped types stored in the callinfo
; are not affected.
define void @test02() {
  %call = call i8* @malloc(i64 64)
  %pts = bitcast i8* %call to %struct.noclonetype01*
  %ptv = bitcast %struct.noclonetype01* %pts to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK: Function: test02
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.noclonetype01 = type { i32, i32 }
; CHECK: Function: test02
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.noclonetype01 = type { i32, i32 }


; This test verifies the types for a memset call info get updated
; to reflect the remapped types.
define void @test03() {
  %local1 = alloca %struct.type01*
  %local1_ptr = load %struct.type01*, %struct.type01** %local1
  %ptr = bitcast %struct.type01* %local1_ptr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)
  ret void
}
; CHECK: Function: test03
; CHECK: Instruction:   call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; This test verifies the types for memfunc calls involving multiple
; regions get updated. (Currently, the compiler only stores a single
; type list since all regions are required to be the same type, but
; this test will allow catching regressions if independent type lists
; were to be stored in the future.)
define void @test04() {
  %local1 = alloca %struct.type02*
  %local2 = alloca %struct.type02*
  %local1_ptr = load %struct.type02*, %struct.type02** %local1
  %local2_ptr = load %struct.type02*, %struct.type02** %local2
  %ptr1 = bitcast %struct.type02* %local1_ptr to i8*
  %ptr2 = bitcast %struct.type02* %local2_ptr to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 8, i1 false)
  ret void
}
; CHECK: Function: test04
; CHECK: MemfuncInfo:
; CHECK:     Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: false
; CHECK:     FirstField: 0
; CHECK:     LastField:  1
; CHECK:     Type: %__DTT_struct.type02 = type { i32, i32, i32, i32 }
; CHECK:   Region 1:
; CHECK:     Complete: false
; CHECK:     FirstField: 0
; CHECK:     LastField:  1
; CHECK:     Type: %__DTT_struct.type02 = type { i32, i32, i32, i32 }


; This test verifies the call info gets moved to the cloned
;function, and the types get updated for the type remapping.
define %struct.type01* @test05() {
  %call = tail call i8* @malloc(i64 64)
  %p = bitcast i8* %call to %struct.type01*
  ret %struct.type01* %p
}
; CHECK: Function: test05.1
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; This test verifies the call info gets moved to the cloned
; function, and the types get updated from the type remapping.
; (Same behavior as test05, but uses call to 'free')
define void @test06(%struct.type01* %in) {
  %ptv = bitcast %struct.type01* %in to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK: Function: test06.2
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; Test to verify call info about memfunc is moved to the cloned
; function, and type information gets remapped.
define void @test07(%struct.type02* %in1) {
  %ptr = bitcast %struct.type02* %in1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 16, i1 false)
  ret void
}
; CHECK: Function: test07.3
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %__DTT_struct.type02 = type { i32, i32, i32, i32 }


; This test verifies the types for memfunc calls involving multiple
; regions get updated. (Currently, the compiler only stores a single
; type list since all regions are required to be the same type, but
; this test will allow catching regressions if independent type lists
; were to be stored in the future.)
define void @test08(%struct.type01* %in1, %struct.type01* %in2) {
  %ptr1 = bitcast %struct.type01* %in1 to i8*
  %ptr2 = bitcast %struct.type01* %in2 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 8, i1 false)
  ret void
}
; CHECK: Function: test08.4
; CHECK: MemfuncInfo:
; CHECK:     Kind: memmove
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; Test to verify remapped dependent data types get their CallInfo objects
; updated.
define void @test09(%struct.type01dep* %in1, %struct.type01dep* %in2) {
  %ptr1 = bitcast %struct.type01dep* %in1 to i8*
  %ptr2 = bitcast %struct.type01dep* %in2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 16, i1 false)
  ret void
}
; CHECK: Function: test09.5
; CHECK: MemfuncInfo:
; CHECK:     Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %__DDT_struct.type01dep = type { %__DTT_struct.type01*, %__DTT_struct.type01* }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: %__DDT_struct.type01dep = type { %__DTT_struct.type01*, %__DTT_struct.type01* }


declare i8* @malloc(i64)
declare void @free(i8*)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8* , i8*, i64, i1)
