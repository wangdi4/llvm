; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; These tests are to check that functions that are readers and writers fields
; are identified.

; Simple test with direct access to fields via GEPs.
%struct.atest01 = type { i32, i64* }
define void @atest01a() {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.atest01*
  %ifield = getelementptr %struct.atest01, %struct.atest01* %st, i64 0, i32 0
  store i32 0, i32* %ifield
  %armem = call i8* @malloc(i64 128)
  %ar = bitcast i8* %armem to i64*
  %pfield = getelementptr %struct.atest01, %struct.atest01* %st, i64 0, i32 1
  store i64* %ar, i64** %pfield
  ret void
}

define void @atest01b(%struct.atest01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !4 {
  %ifield = getelementptr %struct.atest01, %struct.atest01* %in, i64 0, i32 0
  %ival = load i32, i32* %ifield
  ret void
}

define void @atest01c(%struct.atest01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  %pfield = getelementptr %struct.atest01, %struct.atest01* %in, i64 0, i32 1
  %pval = load i64*, i64** %pfield
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest01
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Read Written
; CHECK: Readers: atest01b
; CHECK: Writers: atest01a
; CHECK: 1)Field
; CHECK: DTrans Type: i64*
; CHECK: Field info: Read Written
; CHECK: Readers: atest01c
; CHECK: Writers: atest01a
; CHECK: End LLVMType: %struct.atest01

; Test with byte-flattened GEPs used for reads/writes
%struct.atest02 = type {i32, i64*}
@var02 = internal global %struct.atest02* zeroinitializer, !intel_dtrans_type !6

define void @atest02a() {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.atest02*
  store %struct.atest02* %st, %struct.atest02** @var02
  %ifield_i8ptr = getelementptr i8, i8* %mem, i64 0
  %ifield = bitcast i8* %ifield_i8ptr to i32*
  store i32 0, i32* %ifield
  %armem = call i8* @malloc(i64 128)
  %ar = bitcast i8* %armem to i64*
  %pfield_i8ptr = getelementptr i8, i8* %mem, i64 8
  %pfield = bitcast i8* %pfield_i8ptr to i64**
  store i64* %ar, i64** %pfield
  ret void
}

define void @atest02b(%struct.atest02* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %mem = bitcast %struct.atest02* %in to i8*
  %ifield_i8ptr = getelementptr i8, i8* %mem, i64 0
  %ifield = bitcast i8* %ifield_i8ptr to i32*
  %ival = load i32, i32* %ifield
  ret void
}

define void @atest02c(%struct.atest02* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  %mem = bitcast %struct.atest02* %in to i8*
  %pfield_i8ptr = getelementptr i8, i8* %mem, i64 8
  %pfield = bitcast i8* %pfield_i8ptr to i64**
  %pval = load i64*, i64** %pfield
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest02
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Read Written
; CHECK: Readers: atest02b
; CHECK: Writers: atest02a
; CHECK: 1)Field
; CHECK: DTrans Type: i64*
; CHECK: Field info: Read Written
; CHECK: Readers: atest02c
; CHECK: Writers: atest02a
; CHECK: End LLVMType: %struct.atest02

; Test with write done via a memset for the complete structure.
%struct.atest03 = type { i32, i64* }
@var03 = internal global %struct.atest03* zeroinitializer, !intel_dtrans_type !9

define void @atest03a() {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.atest03*
  store %struct.atest03* %st, %struct.atest03** @var03
  call void @llvm.memset.p0i8.i64(i8* %mem, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest03
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest03a
; CHECK: 1)Field
; CHECK: DTrans Type: i64*
; CHECK: Field info: Written
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest03a
; CHECK: End LLVMType: %struct.atest03

; Test with write done via a memset for a subset of the structure fields.
%struct.atest04 = type { i32, i64* }
@var04 = internal global %struct.atest04* zeroinitializer, !intel_dtrans_type !10

define void @atest04a() {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.atest04*
  store %struct.atest04* %st, %struct.atest04** @var04
  %mem1 = getelementptr i8, i8* %mem, i8 8
  call void @llvm.memset.p0i8.i64(i8* %mem1, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest04
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: 1)Field
; CHECK: DTrans Type: i64*
; CHECK: Field info: Written ComplexUse
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest04a
; CHECK: End LLVMType: %struct.atest04

; Test that read done via memcpy of the entire structure tracks the function as
; a Reader for mod-ref analysis, but does not mark the 'Read' property of the
; field to allow for identifying unused values.
%struct.atest05 = type { i32, i64* }
@var05 = internal global %struct.atest05* zeroinitializer, !intel_dtrans_type !11

define void @atest05a(%struct.atest05* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !12 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.atest05*
  store %struct.atest05* %st, %struct.atest05** @var05
  %src = bitcast %struct.atest05* %in to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %mem, i8* %src, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest05
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers: atest05a
; CHECK: Writers: atest05a
; CHECK: 1)Field
; CHECK: DTrans Type: i64*
; CHECK: Field info: Written
; CHECK: Readers: atest05a
; CHECK: Writers: atest05a
; CHECK: End LLVMType: %struct.atest05

; Test with read done via memcpy of 1 field of structure.
%struct.atest06 = type { i32, i64* }
@var06 = internal global %struct.atest06* zeroinitializer, !intel_dtrans_type !13

define void @atest06a(%struct.atest06* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !14 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.atest06*
  store %struct.atest06* %st, %struct.atest06** @var06
  %field = getelementptr %struct.atest06, %struct.atest06* %in, i64 0, i32 1
  %src = bitcast i64** %field to i8*
  %dst = getelementptr i8, i8* %mem, i64 8
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest06
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: 1)Field
; CHECK: DTrans Type: i64*
; CHECK: Field info: Written ComplexUse
; CHECK: Readers: atest06a
; CHECK: Writers: atest06a
; CHECK: End LLVMType: %struct.atest06

; Test with read done via memcpy for a subset of the structure starting from
; the address of the structure, instead of a field member.
%struct.atest07 = type { i32, i64* }
@var07 = internal global %struct.atest07* zeroinitializer, !intel_dtrans_type !15

define void @atest07a(%struct.atest07* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !16 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.atest07*
  store %struct.atest07* %st, %struct.atest07** @var07
  %src = bitcast %struct.atest07* %in to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %mem, i8* %src, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest07
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: Readers: atest07a
; CHECK: Writers: atest07a
; CHECK: 1)Field
; CHECK: DTrans Type: i64*
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: End LLVMType: %struct.atest07

; Test with memset involving nested structure.
%struct.atest08a = type { i32, i32 }
%struct.atest08b = type { i32, %struct.atest08a }

define void @atest08(%struct.atest08b* "intel_dtrans_func_index"="1" %in1) !intel.dtrans.func.type !19 {
  %faddr = getelementptr inbounds %struct.atest08b, %struct.atest08b* %in1, i64 0, i32 1
  %ptr = bitcast %struct.atest08a* %faddr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest08a
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest08
; CHECK: 1)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest08
; CHECK: End LLVMType: %struct.atest08a

; CHECK-LABEL: LLVMType: %struct.atest08b
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: 1)Field
; CHECK: DTrans Type: %struct.atest08a
; CHECK: Field info: Written ComplexUse
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest08
; CHECK: End LLVMType: %struct.atest08b

; Test with memcpy involving nested structure.
%struct.atest09a = type { i32, i32 }
%struct.atest09b = type { i32, %struct.atest09a }
define void @atest09(%struct.atest09b* "intel_dtrans_func_index"="1" %in1, %struct.atest09b* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !22 {
  %src_faddr = getelementptr inbounds %struct.atest09b, %struct.atest09b* %in1, i64 0, i32 1
  %dst_faddr = getelementptr inbounds %struct.atest09b, %struct.atest09b* %in2, i64 0, i32 1
  %src = bitcast %struct.atest09a* %src_faddr to i8*
  %dst = bitcast %struct.atest09a* %dst_faddr to i8*
 call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest09a
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers: atest09
; CHECK: Writers: atest09
; CHECK: 1)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers: atest09
; CHECK: Writers: atest09
; CHECK: End LLVMType: %struct.atest09a

; CHECK-LABEL: LLVMType: %struct.atest09b
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: 1)Field
; CHECK: DTrans Type: %struct.atest09a = type { i32, i32 }
; CHECK: Field info: Written ComplexUse
; CHECK: Readers: atest09
; CHECK: Writers: atest09
; CHECK: End LLVMType: %struct.atest09b

; Test with memcpy involving one type to a field of another structure of that
; type.
%struct.atest10a = type { i32, i32 }
%struct.atest10b = type { i32, %struct.atest10a }
define void @atest10(%struct.atest10a* "intel_dtrans_func_index"="1" %in1, %struct.atest10b* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !26 {
  %dst_faddr = getelementptr inbounds %struct.atest10b, %struct.atest10b* %in2, i64 0, i32 1
  %src = bitcast %struct.atest10a* %in1 to i8*
  %dst = bitcast %struct.atest10a* %dst_faddr to i8*
 call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest10a
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers: atest10
; CHECK: Writers: atest10
; CHECK: 1)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers: atest10
; CHECK: Writers: atest10
; CHECK: End LLVMType: %struct.atest10a

; CHECK-LABEL: LLVMType: %struct.atest10b
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: 1)Field
; CHECK: DTrans Type: %struct.atest10a = type { i32, i32 }
; CHECK: Field info: Written ComplexUse
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest10
; CHECK: End LLVMType: %struct.atest10b

%struct.atest11a = type { i32, i32 }
%struct.atest11b = type { i32, %struct.atest11a }
define void @atest11(%struct.atest11b* "intel_dtrans_func_index"="1" %in1, %struct.atest11a* "intel_dtrans_func_index"="2" %in2) !intel.dtrans.func.type !30 {
  %src_faddr = getelementptr inbounds %struct.atest11b, %struct.atest11b* %in1, i64 0, i32 1
  %src = bitcast %struct.atest11a* %src_faddr to i8*
  %dst = bitcast %struct.atest11a* %in2 to i8*
 call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest11a
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers: atest11
; CHECK: Writers: atest11
; CHECK: 1)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers: atest11
; CHECK: Writers: atest11
; CHECK: End LLVMType: %struct.atest11a

; CHECK-LABEL: LLVMType: %struct.atest11b
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: 1)Field
; CHECK: DTrans Type: %struct.atest11a = type { i32, i32 }
; CHECK: Field info: ComplexUse
; CHECK: Readers: atest11
; CHECK: Writers:{{ *$}}
; CHECK: End LLVMType: %struct.atest11b

declare !intel.dtrans.func.type !32 "intel_dtrans_func_index"="1" i8* @malloc(i64)
declare !intel.dtrans.func.type !33 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !34 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{%struct.atest01 zeroinitializer, i32 1}  ; %struct.atest01*
!4 = distinct !{!3}
!5 = distinct !{!3}
!6 = !{%struct.atest02 zeroinitializer, i32 1}  ; %struct.atest02*
!7 = distinct !{!6}
!8 = distinct !{!6}
!9 = !{%struct.atest03 zeroinitializer, i32 1}  ; %struct.atest03*
!10 = !{%struct.atest04 zeroinitializer, i32 1}  ; %struct.atest04*
!11 = !{%struct.atest05 zeroinitializer, i32 1}  ; %struct.atest05*
!12 = distinct !{!11}
!13 = !{%struct.atest06 zeroinitializer, i32 1}  ; %struct.atest06*
!14 = distinct !{!13}
!15 = !{%struct.atest07 zeroinitializer, i32 1}  ; %struct.atest07*
!16 = distinct !{!15}
!17 = !{%struct.atest08a zeroinitializer, i32 0}  ; %struct.atest08a
!18 = !{%struct.atest08b zeroinitializer, i32 1}  ; %struct.atest08b*
!19 = distinct !{!18}
!20 = !{%struct.atest09a zeroinitializer, i32 0}  ; %struct.atest09a
!21 = !{%struct.atest09b zeroinitializer, i32 1}  ; %struct.atest09b*
!22 = distinct !{!21, !21}
!23 = !{%struct.atest10a zeroinitializer, i32 0}  ; %struct.atest10a
!24 = !{%struct.atest10a zeroinitializer, i32 1}  ; %struct.atest10a*
!25 = !{%struct.atest10b zeroinitializer, i32 1}  ; %struct.atest10b*
!26 = distinct !{!24, !25}
!27 = !{%struct.atest11a zeroinitializer, i32 0}  ; %struct.atest11a
!28 = !{%struct.atest11b zeroinitializer, i32 1}  ; %struct.atest11b*
!29 = !{%struct.atest11a zeroinitializer, i32 1}  ; %struct.atest11a*
!30 = distinct !{!28, !29}
!31 = !{i8 0, i32 1}  ; i8*
!32 = distinct !{!31}
!33 = distinct !{!31, !31}
!34 = distinct !{!31}
!35 = !{!"S", %struct.atest01 zeroinitializer, i32 2, !1, !2} ; {i32, i64*}
!36 = !{!"S", %struct.atest02 zeroinitializer, i32 2, !1, !2} ; {i32, i64*}
!37 = !{!"S", %struct.atest03 zeroinitializer, i32 2, !1, !2} ; {i32, i64*}
!38 = !{!"S", %struct.atest04 zeroinitializer, i32 2, !1, !2} ; {i32, i64*}
!39 = !{!"S", %struct.atest05 zeroinitializer, i32 2, !1, !2} ; {i32, i64*}
!40 = !{!"S", %struct.atest06 zeroinitializer, i32 2, !1, !2} ; {i32, i64*}
!41 = !{!"S", %struct.atest07 zeroinitializer, i32 2, !1, !2} ; {i32, i64*}
!42 = !{!"S", %struct.atest08a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!43 = !{!"S", %struct.atest08b zeroinitializer, i32 2, !1, !17} ; { i32, %struct.atest08a }
!44 = !{!"S", %struct.atest09a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!45 = !{!"S", %struct.atest09b zeroinitializer, i32 2, !1, !20} ; { i32, %struct.atest09a }
!46 = !{!"S", %struct.atest10a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!47 = !{!"S", %struct.atest10b zeroinitializer, i32 2, !1, !23} ; { i32, %struct.atest10a }
!48 = !{!"S", %struct.atest11a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!49 = !{!"S", %struct.atest11b zeroinitializer, i32 2, !1, !27} ; { i32, %struct.atest11a }

!intel.dtrans.types = !{!35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49}
