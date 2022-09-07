; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; These tests are to check that functions that read/write fields are identified

%struct.atest01 = type {i32, i64*}
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

define void @atest01b(%struct.atest01* %in) {
 %ifield = getelementptr %struct.atest01, %struct.atest01* %in, i64 0, i32 0
 %ival = load i32, i32* %ifield

 ret void
}

define void @atest01c(%struct.atest01* %in) {
 %pfield = getelementptr %struct.atest01, %struct.atest01* %in, i64 0, i32 1
 %pval = load i64*, i64** %pfield

 ret void
}
; CHECK-LABEL: LLVMType: %struct.atest01 = type { i32, i64* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers: atest01b
; CHECK: Writers: atest01a
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i64*
; CHECK: Readers: atest01c
; CHECK: Writers: atest01a
; CHECK: RWState: top

; Test with byte-flattened GEPs used for reads/writes
%struct.atest02 = type {i32, i64*}
define void @atest02a() {
 %mem = call i8* @malloc(i64 16)
 %st = bitcast i8* %mem to %struct.atest02*

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

define void @atest02b(%struct.atest02* %in) {
 %mem = bitcast %struct.atest02* %in to i8*
 %ifield_i8ptr = getelementptr i8, i8* %mem, i64 0
 %ifield = bitcast i8* %ifield_i8ptr to i32*

 %ival = load i32, i32* %ifield

 ret void
}

define void @atest02c(%struct.atest02* %in) {
 %mem = bitcast %struct.atest02* %in to i8*
 %pfield_i8ptr = getelementptr i8, i8* %mem, i64 8
 %pfield = bitcast i8* %pfield_i8ptr to i64**
 %pval = load i64*, i64** %pfield

 ret void
}
; CHECK-LABEL: LLVMType: %struct.atest02 = type { i32, i64* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers: atest02b
; CHECK: Writers: atest02a
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i64*
; CHECK: Readers: atest02c
; CHECK: Writers: atest02a
; CHECK: RWState: top

; Test with write done via a memset for the full structure.
%struct.atest03 = type {i32, i64*}
define void @atest03a() {
 %mem = call i8* @malloc(i64 16)
 %st = bitcast i8* %mem to %struct.atest03*
 call void @llvm.memset.p0i8.i64(i8* %mem, i8 0, i64 16, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest03 = type { i32, i64* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest03a
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i64*
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest03a
; CHECK: RWState: top

; Test with write done via a memset for a portion of the structure.
%struct.atest04 = type {i32, i64*}
define void @atest04a() {
 %mem = call i8* @malloc(i64 16)
 %st = bitcast i8* %mem to %struct.atest04*

 %mem1 = getelementptr i8, i8* %mem, i8 8
 call void @llvm.memset.p0i8.i64(i8* %mem1, i8 0, i64 8, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest04 = type { i32, i64* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i64*
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest04a
; CHECK: RWState: top

; Test with read done via memcpy of entire structure
%struct.atest05 = type {i32, i64*}
define void @atest05a(%struct.atest05* %in) {
 %mem = call i8* @malloc(i64 16)
 %st = bitcast i8* %mem to %struct.atest05*

 %src = bitcast %struct.atest05* %in to i8*
 call void @llvm.memcpy.p0i8.p0i8.i64(i8* %mem, i8* %src, i64 16, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest05 = type { i32, i64* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers: atest05a
; CHECK: Writers: atest05a
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i64*
; CHECK: Readers: atest05a
; CHECK: Writers: atest05a
; CHECK: RWState: top

; Test with read done via memcpy of 1 field of structure
%struct.atest06 = type {i32, i64*}
define void @atest06a(%struct.atest06* %in) {
 %mem = call i8* @malloc(i64 16)
 %st = bitcast i8* %mem to %struct.atest06*

 %field = getelementptr %struct.atest06, %struct.atest06* %in, i64 0, i32 1
 %src = bitcast i64** %field to i8*
 %dst = getelementptr i8, i8* %mem, i64 8
 call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 8, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest06 = type { i32, i64* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i64*
; CHECK: Readers: atest06a
; CHECK: Writers: atest06a
; CHECK: RWState: top

; Test with read done via memcpy for a subset of the structure starting from
; the address of the structure, instead of a field member
%struct.atest07 = type {i32, i64*}
define void @atest07a(%struct.atest07* %in) {
 %mem = call i8* @malloc(i64 16)
 %st = bitcast i8* %mem to %struct.atest07*

 %src = bitcast %struct.atest07* %in to i8*
 call void @llvm.memcpy.p0i8.p0i8.i64(i8* %mem, i8* %src, i64 8, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest07 = type { i32, i64* }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers: atest07a
; CHECK: Writers: atest07a
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i64*
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: RWState: top

; Test with memset involving nested structure
%struct.atest08a = type { i32, i32 }
%struct.atest08b = type { i32, %struct.atest08a }
define void @atest08(%struct.atest08b* %in1) {
  %faddr = getelementptr inbounds %struct.atest08b, %struct.atest08b* %in1, i64 0, i32 1
  %ptr = bitcast %struct.atest08a* %faddr to i8*
  call void @llvm.memset.p0i8.i64(i8* %ptr, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest08a = type { i32, i32 }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest08
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i32
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest08
; CHECK: RWState: top
; CHECK-LABEL: LLVMType: %struct.atest08b = type { i32, %struct.atest08a }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: %struct.atest08a = type { i32, i32 }
; CHECK: Readers:{{ *$}}
; CHECK: Writers: atest08
; CHECK: RWState: top

; Test with memcpy involving nested structure
%struct.atest09a = type { i32, i32 }
%struct.atest09b = type { i32, %struct.atest09a }
define void @atest09(%struct.atest09b* %in1, %struct.atest09b* %in2) {
  %src_faddr = getelementptr inbounds %struct.atest09b, %struct.atest09b* %in1, i64 0, i32 1
  %dst_faddr = getelementptr inbounds %struct.atest09b, %struct.atest09b* %in2, i64 0, i32 1

  %src = bitcast %struct.atest09a* %src_faddr to i8*
  %dst = bitcast %struct.atest09a* %dst_faddr to i8*
 call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dst, i8* %src, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.atest09a = type { i32, i32 }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers: atest09
; CHECK: Writers: atest09
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: i32
; CHECK: Readers: atest09
; CHECK: Writers: atest09
; CHECK: RWState: top
; CHECK-LABEL: LLVMType: %struct.atest09b = type { i32, %struct.atest09a }
; CHECK: 0)Field LLVM Type: i32
; CHECK: Readers:{{ *$}}
; CHECK: Writers:{{ *$}}
; CHECK: RWState: top
; CHECK: 1)Field LLVM Type: %struct.atest09a = type { i32, i32 }
; CHECK: Readers: atest09
; CHECK: Writers: atest09
; CHECK: RWState: top

declare dso_local i8* @malloc(i64)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
