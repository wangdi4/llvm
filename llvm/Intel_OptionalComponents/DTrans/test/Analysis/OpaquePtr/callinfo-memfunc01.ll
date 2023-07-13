; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s

; This test verifies memory function call info collection is done for
; types that are safe uses of the memfunc calls.

; --------------------------------------------------------------------------
; Tests for memset callinfo
; --------------------------------------------------------------------------

; This test checks when an entire structure is filled by memset.
%struct.test01 = type { i32, i16, i8 }
define void @test01(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !5 {
  %ptr = bitcast ptr %in1 to ptr
  call void @llvm.memset.p0.i64(ptr %ptr, i8 0, i64 8, i1 false)
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
define void @test02(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !7 {
  %ptr = bitcast ptr %in1 to ptr
  call void @llvm.memset.p0.i64(ptr %ptr, i8 0, i64 32, i1 false)
  ret void
}
; CHECK: Function: test02
; CHECK: MemfuncInfo:
; CHECK:     Kind: memset
; CHECK:     Complete: true
; CHECK:     Type: %struct.test02 = type { i32, i16, i8 }


; This test checks using memset to clear part of the structure, starting
; from the address of the start of the structure.
%struct.test03 = type { i32, i16, i16, i8 }
define void @test03(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !9 {
  %ptr = bitcast ptr %in1 to ptr
  call void @llvm.memset.p0.i64(ptr %ptr, i8 0, i64 6, i1 false)
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
; from the address of a field within the structure.
%struct.test04 = type { i64, i64, i64, i64, i64 }
define void @test04(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !12 {
  %faddr = getelementptr inbounds %struct.test04, ptr %in1, i64 0, i32 0
  %ptr = bitcast ptr %faddr to ptr
  call void @llvm.memset.p0.i64(ptr %ptr, i8 0, i64 24, i1 false)
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
; structure.
%struct.test05 = type { i64, i64, i64, i64, i64 }
define void @test05(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !14 {
  %faddr = getelementptr inbounds %struct.test05, ptr %in1, i64 0, i32 1
  %ptr = bitcast ptr %faddr to ptr
  call void @llvm.memset.p0.i64(ptr %ptr, i8 0, i64 24, i1 false)
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
define void @test06(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !16 {
  %faddr = getelementptr inbounds %struct.test06, ptr %in1, i64 0, i32 0
  %ptr = bitcast ptr %faddr to ptr
  call void @llvm.memset.p0.i64(ptr %ptr, i8 0, i64 40, i1 false)
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
; future support operations on nested structures, we may need to extend the
; modeling, or internally note that the operation on the outer structure
; is impacting an inner structure type.
%struct.test07.a = type { i32, i32, i32, i32, i32 }
%struct.test07.b = type { i32, %struct.test07.a }
define void @test07(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !19 {
  %faddr = getelementptr inbounds %struct.test07.b, ptr %in1, i64 0, i32 1
  %ptr = bitcast ptr %faddr to ptr
  call void @llvm.memset.p0.i64(ptr %ptr, i8 0, i64 20, i1 false)
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
define void @test08(ptr "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !22 {
  %arr_addr = bitcast ptr %in1 to ptr
  tail call void @llvm.memset.p0.i64(ptr %arr_addr, i8 0, i64 100, i1 false)
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

; The test checks calls to memcpy to copy the entire structure.
%struct.test09 = type { i32, i32 }
define void @test09(ptr "intel_dtrans_func_index"="1" %in1, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !24 {
  %ptr1 = bitcast ptr %in1 to ptr
  %ptr2 = bitcast ptr %in2 to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %ptr1, ptr %ptr2, i64 8, i1 false)
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


; The test checks calls to memcpy to copy the entire structure starting with
; starting address of the first field within the structure.
%struct.test10 = type { i32, i32, i32, i32 }
define void @test10(ptr "intel_dtrans_func_index"="1" %in1, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !26 {
  %faddr1 = getelementptr inbounds %struct.test10, ptr %in1, i64 0, i32 0
  %ptr1 = bitcast ptr %faddr1 to ptr
  %faddr2 = getelementptr inbounds %struct.test10, ptr %in2, i64 0, i32 0
  %ptr2 = bitcast ptr %faddr2 to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %ptr1, ptr %ptr2, i64 16, i1 false)
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


; The test checks calls to memcpy to copy a subset of fields of the structure
; starting with starting address of the structure.
%struct.test11 = type { i32, i32, i32, i32 }
define void @test11(ptr "intel_dtrans_func_index"="1" %in1, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !28 {
  %ptr1 = bitcast ptr %in1 to ptr
  %ptr2 = bitcast ptr %in2 to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %ptr1, ptr %ptr2, i64 8, i1 false)
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
define void @test12(ptr "intel_dtrans_func_index"="1" %in1, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !30 {
  %faddr1 = getelementptr inbounds %struct.test12, ptr %in1, i64 0, i32 1
  %ptr1 = bitcast ptr %faddr1 to ptr
  %faddr2 = getelementptr inbounds %struct.test12, ptr %in2, i64 0, i32 1
  %ptr2 = bitcast ptr %faddr2 to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %ptr1, ptr %ptr2, i64 24, i1 false)
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
define void @test13(ptr "intel_dtrans_func_index"="1" %in1, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !33 {
  %arr_addr1 = bitcast ptr %in1 to ptr
  %arr_addr2 = bitcast ptr %in2 to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %arr_addr1, ptr %arr_addr2, i64 200, i1 false)
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

; The test checks calls to memmove to copy the entire structure.
%struct.test14 = type { i32, i32 }
define void @test14(ptr "intel_dtrans_func_index"="1" %in1, ptr "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !35 {
  %ptr1 = bitcast ptr %in1 to ptr
  %ptr2 = bitcast ptr %in2 to ptr
  call void @llvm.memmove.p0.p0.i64(ptr %ptr1, ptr %ptr2, i64 8, i1 false)
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

declare !intel.dtrans.func.type !37 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)
declare !intel.dtrans.func.type !38 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !39 void @llvm.memmove.p0.p0.i64(ptr "intel_dtrans_func_index"="1" , ptr "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 0}  ; i8
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{i64 0, i32 0}  ; i64
!11 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!12 = distinct !{!11}
!13 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!14 = distinct !{!13}
!15 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!16 = distinct !{!15}
!17 = !{%struct.test07.a zeroinitializer, i32 0}  ; %struct.test07.a
!18 = !{%struct.test07.b zeroinitializer, i32 1}  ; %struct.test07.b*
!19 = distinct !{!18}
!20 = !{!21, i32 1}  ; [ 25 x i32 ]*
!21 = !{!"A", i32 25, !1}  ; [ 25 x i32 ]
!22 = distinct !{!20}
!23 = !{%struct.test09 zeroinitializer, i32 1}  ; %struct.test09*
!24 = distinct !{!23, !23}
!25 = !{%struct.test10 zeroinitializer, i32 1}  ; %struct.test10*
!26 = distinct !{!25, !25}
!27 = !{%struct.test11 zeroinitializer, i32 1}  ; %struct.test11*
!28 = distinct !{!27, !27}
!29 = !{%struct.test12 zeroinitializer, i32 1}  ; %struct.test12*
!30 = distinct !{!29, !29}
!31 = !{!32, i32 1}  ; [ 25 x i64 ]*
!32 = !{!"A", i32 25, !10}  ; [ 25 x i64 ]
!33 = distinct !{!31, !31}
!34 = !{%struct.test14 zeroinitializer, i32 1}  ; %struct.test14*
!35 = distinct !{!34, !34}
!36 = !{i8 0, i32 1}  ; i8*
!37 = distinct !{!36}
!38 = distinct !{!36, !36}
!39 = distinct !{!36, !36}
!40 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!41 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!42 = !{!"S", %struct.test03 zeroinitializer, i32 4, !1, !2, !2, !3} ; { i32, i16, i16, i8 }
!43 = !{!"S", %struct.test04 zeroinitializer, i32 5, !10, !10, !10, !10, !10} ; { i64, i64, i64, i64, i64 }
!44 = !{!"S", %struct.test05 zeroinitializer, i32 5, !10, !10, !10, !10, !10} ; { i64, i64, i64, i64, i64 }
!45 = !{!"S", %struct.test06 zeroinitializer, i32 5, !10, !10, !10, !10, !10} ; { i64, i64, i64, i64, i64 }
!46 = !{!"S", %struct.test07.a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!47 = !{!"S", %struct.test07.b zeroinitializer, i32 2, !1, !17} ; { i32, %struct.test07.a }
!48 = !{!"S", %struct.test09 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!49 = !{!"S", %struct.test10 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!50 = !{!"S", %struct.test11 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }
!51 = !{!"S", %struct.test12 zeroinitializer, i32 5, !10, !10, !10, !10, !10} ; { i64, i64, i64, i64, i64 }
!52 = !{!"S", %struct.test14 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52}
