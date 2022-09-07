; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Show that non-escaping conditions do not invalidate the single alloc function

%struct.MYSTRUCT = type { i8* }

@globalstruct = internal global %struct.MYSTRUCT zeroinitializer, align 8

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

declare void @llvm.memmove.p0i8.p0i8.i64(i8* , i8*, i64, i1)

define i32 @main() {
  %t0 = tail call noalias i8* @malloc(i64 100)
  store i8* %t0, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0), align 8
  %t1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0
  %t2 = load i8*, i8** %t1, align 8
  %t3 = load i8, i8* %t2, align 8
  %t4 = icmp eq i8* %t2, null
  br i1 %t4, label %t5, label %t6
t5:
  tail call void @free(i8* %t2)
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t2, i8* %t2, i64 5, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %t2, i8 0, i64 24, i1 false)
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %t2, i8* %t2, i64 8, i1 false)
  %t7 = getelementptr inbounds i8, i8* %t2, i64 0
  %t8 = load i8, i8* %t7, align 8
  br label %t5
t6:
  store i8* null, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0), align 8
  %t9 = sext i8 %t3 to i32
  ret i32 %t9
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Read Written
; CHECK: Multiple Value
; CHECK: Single Alloc Function: i8* (i64)* @malloc
; CHECK: Safety data: Global instance
