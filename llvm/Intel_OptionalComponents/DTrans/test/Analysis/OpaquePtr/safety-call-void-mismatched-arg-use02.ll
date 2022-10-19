; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test passing a pointer to an aggregate type as an i8* type which results in a
; "Mismatched argument use" safety flag.

; Test with using the i8* parameter as an aggregate type that is incompatible
; with the use in the caller.
%struct.test01.a = type { i64, i32, i32 }
%struct.test01.b = type { i32, i32, i32, i32 }
define i1 @test01less(i8* "intel_dtrans_func_index"="1" %p0, i8* "intel_dtrans_func_index"="2" %p1) !intel.dtrans.func.type !4 {
  %ps0 = bitcast i8* %p0 to %struct.test01.b*
  %ps1 = bitcast i8* %p1 to %struct.test01.b*
  %fs0 = getelementptr %struct.test01.b, %struct.test01.b* %ps0, i64 0, i32 0
  %fs1 = getelementptr %struct.test01.b, %struct.test01.b* %ps1, i64 0, i32 0
  %v0 = load i32, i32* %fs0
  %v1 = load i32, i32* %fs1
  %cmp = icmp slt i32 %v0, %v1
  ret i1 %cmp
}

define void @test01(%struct.test01.a** "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %ps_addr0 = getelementptr %struct.test01.a*, %struct.test01.a** %pStruct, i64 0
  %ps_addr1 = getelementptr %struct.test01.a*, %struct.test01.a** %pStruct, i64 1
  %ps0 = load %struct.test01.a*, %struct.test01.a** %ps_addr0
  %ps1 = load %struct.test01.a*, %struct.test01.a** %ps_addr1
  %less = call i1 bitcast (i1 (i8*, i8*)* @test01less to
               i1 (%struct.test01.a*, %struct.test01.a*)*)(%struct.test01.a* %ps0, %struct.test01.a* %ps1)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.a
; CHECK: Safety data: Mismatched argument use{{ *}}
; CHECK: End LLVMType: %struct.test01.a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01.b
; CHECK: Safety data: Mismatched argument use{{ *}}
; CHECK: End LLVMType: %struct.test01.b

; Test with using the i8* parameter as a non-aggregate type that is incompatible
; with the use in the caller.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02** "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !8 {
  %ps_addr0 = getelementptr %struct.test02*, %struct.test02** %pStruct, i64 0
  %ps_addr1 = getelementptr %struct.test02*, %struct.test02** %pStruct, i64 1
  %ps0 = load %struct.test02*, %struct.test02** %ps_addr0
  %ps1 = load %struct.test02*, %struct.test02** %ps_addr1
  %less = call i1 bitcast (i1 (i8*, i8*)* @test02less to
               i1 (%struct.test02*, %struct.test02*)*)(%struct.test02* %ps0, %struct.test02* %ps1)
  ret void
}

define i1 @test02less(i8* "intel_dtrans_func_index"="1" %p0, i8* "intel_dtrans_func_index"="2" %p1) !intel.dtrans.func.type !9 {
  %ps0 = bitcast i8* %p0 to i32*
  %ps1 = bitcast i8* %p1 to i32*
  %v0 = load i32, i32* %ps0
  %v1 = load i32, i32* %ps1
  %cmp = icmp slt i32 %v0, %v1
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Mismatched argument use{{ *}}
; CHECK: End LLVMType: %struct.test02


; Test with using the i8* parameter as an ambiguous aggregate type in the callee.
%struct.test03.a = type { i64, i32, i32 }
%struct.test03.b = type { i32, i32, i32, i32 }
%struct.test03.c = type { i32, i32, i64 }
define i1 @test03less(i8* "intel_dtrans_func_index"="1" %p0, i8* "intel_dtrans_func_index"="2" %p1) !intel.dtrans.func.type !10 {
  %ps0 = bitcast i8* %p0 to %struct.test03.b*
  %ps1 = bitcast i8* %p1 to %struct.test03.b*
  %fs0 = getelementptr %struct.test03.b, %struct.test03.b* %ps0, i64 0, i32 0
  %fs1 = getelementptr %struct.test03.b, %struct.test03.b* %ps1, i64 0, i32 0
  %v0 = load i32, i32* %fs0
  %v1 = load i32, i32* %fs1
  %cmp0 = icmp slt i32 %v0, %v1

  %ps2 = bitcast i8* %p0 to %struct.test03.c*
  %ps3 = bitcast i8* %p1 to %struct.test03.c*
  %fs2 = getelementptr %struct.test03.c, %struct.test03.c* %ps2, i64 0, i32 2
  %fs3 = getelementptr %struct.test03.c, %struct.test03.c* %ps3, i64 0, i32 2
  %v2 = load i64, i64* %fs2
  %v3 = load i64, i64* %fs3
  %cmp1 = icmp slt i64 %v2, %v3

  %cmp = or i1 %cmp0, %cmp1
  ret i1 %cmp
}

define void @test03(%struct.test03.a** "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !12 {
  %ps_addr0 = getelementptr %struct.test03.a*, %struct.test03.a** %pStruct, i64 0
  %ps_addr1 = getelementptr %struct.test03.a*, %struct.test03.a** %pStruct, i64 1
  %ps0 = load %struct.test03.a*, %struct.test03.a** %ps_addr0
  %ps1 = load %struct.test03.a*, %struct.test03.a** %ps_addr1
  %less = call i1 bitcast (i1 (i8*, i8*)* @test03less to
               i1 (%struct.test03.a*, %struct.test03.a*)*)(%struct.test03.a* %ps0, %struct.test03.a* %ps1)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03.a
; CHECK: Safety data: Mismatched argument use{{ *}}
; CHECK: End LLVMType: %struct.test03.a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03.b
; CHECK: Safety data: Ambiguous GEP | Mismatched argument use{{ *}}
; CHECK: End LLVMType: %struct.test03.b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03.c
; CHECK: Safety data: Ambiguous GEP | Mismatched argument use{{ *}}
; CHECK: End LLVMType: %struct.test03.c


!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3, !3}
!5 = !{%struct.test01.a zeroinitializer, i32 2}  ; %struct.test01.a**
!6 = distinct !{!5}
!7 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!8 = distinct !{!7}
!9 = distinct !{!3, !3}
!10 = distinct !{!3, !3}
!11 = !{%struct.test03.a zeroinitializer, i32 2}  ; %struct.test03.a**
!12 = distinct !{!11}
!13 = !{!"S", %struct.test01.a zeroinitializer, i32 3, !1, !2, !2} ; { i64, i32, i32 }
!14 = !{!"S", %struct.test01.b zeroinitializer, i32 4, !2, !2, !2, !2} ; { i32, i32, i32, i32 }
!15 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !2} ; { i32, i32 }
!16 = !{!"S", %struct.test03.a zeroinitializer, i32 3, !1, !2, !2} ; { i64, i32, i32 }
!17 = !{!"S", %struct.test03.b zeroinitializer, i32 4, !2, !2, !2, !2} ; { i32, i32, i32, i32 }
!18 = !{!"S", %struct.test03.c zeroinitializer, i32 3, !2, !2, !1} ; { i32, i32, i64 }

!intel.dtrans.types = !{!13, !14, !15, !16, !17, !18}
