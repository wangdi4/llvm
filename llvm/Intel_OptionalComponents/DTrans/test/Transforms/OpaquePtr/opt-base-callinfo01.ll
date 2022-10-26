; REQUIRES: asserts
; RUN: opt  < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.type01,struct.type02 -debug-only=dtransop-optbase 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-NONOPAQUE
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.type01,struct.type02 -debug-only=dtransop-optbase 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

%struct.noclonetype01 = type { i32, i32 }
%struct.type01 = type { i32, i16, i8 }
%struct.type01dep = type { %struct.type01*, %struct.type01* }
%struct.type02 = type { i32, i32, i32, i32 }

; This test verifies the types stored in the callinfo
; get updated after the type remapping occurs.

define void @test01() {
  %call = call i8* @malloc(i64 64)
  %pts = bitcast i8* %call to %struct.type01*
  %field = getelementptr %struct.type01, %struct.type01* %pts, i64 0, i32 0
  %ptv = bitcast %struct.type01* %pts to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK-LABEL: Call info after transforming functions
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
; are not changed.
define void @test02() {
  %call = call i8* @malloc(i64 64)
  %pts = bitcast i8* %call to %struct.noclonetype01*
  %field = getelementptr %struct.noclonetype01, %struct.noclonetype01* %pts, i64 0, i32 0
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
  %local1 = alloca %struct.type01*, !intel_dtrans_type !4
  %local1_ptr = load %struct.type01*, %struct.type01** %local1
  %ptr = bitcast %struct.type01* %local1_ptr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)
  ret void
}
; CHECK: Function: test03
; CHECK: Instruction:   call void @llvm.memset{{.*}}
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; This test verifies the types for memfunc calls involving multiple
; regions get updated. (Currently, the compiler only stores a single
; type list since all regions are required to be the same type, but
; this test will allow catching regressions if independent type lists
; are to be stored in the future.)
define void @test04() {
  %local1 = alloca %struct.type02*, !intel_dtrans_type !5
  %local2 = alloca %struct.type02*, !intel_dtrans_type !5
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
; function, and the types get updated for the type remapping.
define "intel_dtrans_func_index"="1" %struct.type01* @test05() !intel.dtrans.func.type !6 {
  %call = tail call i8* @malloc(i64 64)
  %p = bitcast i8* %call to %struct.type01*
  ret %struct.type01* %p
}
; When pointers are not opaque, the remaining functions in this test should
; all be cloned, verify the call info gets associated with the cloned function.
; Check lines for opaque pointers are placeholders, and not used by this test
; yet.

; CHECK-NONOPAQUE: Function: test05.1
; CHECK-OPAQUE: Function: test05{{ *$}}
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; This test verifies the call info gets moved to the cloned
; function, and the types get updated from the type remapping.
; (Same behavior as test05, but uses call to 'free')
define void @test06(%struct.type01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %ptv = bitcast %struct.type01* %in to i8*
  call void @free(i8* %ptv)
  ret void
}
; CHECK-NONOPAQUE: Function: test06.2
; CHECK-OPAQUE: Function: test06{{ *$}}
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %__DTT_struct.type01 = type { i32, i16, i8 }


; Test to verify call info about memfunc is moved to the cloned
; function, and type information gets remapped.
define void @test07(%struct.type02* "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !8 {
  %ptr = bitcast %struct.type02* %in1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-NONOPAQUE: Function: test07.3
; CHECK-OPAQUE: Function: test07{{ *$}}
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %__DTT_struct.type02 = type { i32, i32, i32, i32 }


; This test verifies the types for memfunc calls involving multiple
; regions get updated. (Currently, the compiler only stores a single
; type list since all regions are required to be the same type, but
; this test will allow catching regressions if independent type lists
; are to be stored in the future.)
define void @test08(%struct.type01* "intel_dtrans_func_index"="1" %in1, %struct.type01* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !9 {
  %ptr1 = bitcast %struct.type01* %in1 to i8*
  %ptr2 = bitcast %struct.type01* %in2 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 8, i1 false)
  ret void
}
; CHECK-NONOPAQUE: Function: test08.4
; CHECK-OPAQUE: Function: test08{{ *$}}
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
define void @test09(%struct.type01dep* "intel_dtrans_func_index"="1" %in1, %struct.type01dep* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !11 {
  %ptr1 = bitcast %struct.type01dep* %in1 to i8*
  %ptr2 = bitcast %struct.type01dep* %in2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %ptr1, i8* %ptr2, i64 16, i1 false)
  ret void
}
; CHECK-NONOPAQUE: Function: test09.5
; CHECK-OPAQUE: Function: test09{{ *$}}
; CHECK: MemfuncInfo:
; CHECK:     Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK-NONOPAQUE:  Type: %__DDT_struct.type01dep = type { %__DTT_struct.type01*, %__DTT_struct.type01* }
; CHECK-OPAQUE:     Type: %struct.type01dep = type { %struct.type01*, %struct.type01* }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK-NONOPAQUE:  Type: %__DDT_struct.type01dep = type { %__DTT_struct.type01*, %__DTT_struct.type01* }
; CHECK-OPAQUE:     Type: %struct.type01dep = type { %struct.type01*, %struct.type01* }

declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !14 void @free(i8* "intel_dtrans_func_index"="1") #1
declare !intel.dtrans.func.type !15 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)
declare !intel.dtrans.func.type !16 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !17 void @llvm.memmove.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1" , i8* "intel_dtrans_func_index"="2", i64, i1)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 0}  ; i8
!4 = !{%struct.type01 zeroinitializer, i32 1}  ; %struct.type01*
!5 = !{%struct.type02 zeroinitializer, i32 1}  ; %struct.type02*
!6 = distinct !{!4}
!7 = distinct !{!4}
!8 = distinct !{!5}
!9 = distinct !{!4, !4}
!10 = !{%struct.type01dep zeroinitializer, i32 1}  ; %struct.type01dep*
!11 = distinct !{!10, !10}
!12 = !{i8 0, i32 1}  ; i8*
!13 = distinct !{!12}
!14 = distinct !{!12}
!15 = distinct !{!12}
!16 = distinct !{!12, !12}
!17 = distinct !{!12, !12}
!18 = !{!"S", %struct.noclonetype01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!19 = !{!"S", %struct.type01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!20 = !{!"S", %struct.type01dep zeroinitializer, i32 2, !4, !4} ; { %struct.type01*, %struct.type01* }
!21 = !{!"S", %struct.type02 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }

!intel.dtrans.types = !{!18, !19, !20, !21}
