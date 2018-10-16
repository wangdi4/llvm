; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; NOTE: This test is now obsolete, as the isSafeLoadForSingleAllocFunction
; member function of DTransInstVisitor has been removed.  This was done
; because it was too conservative.

; Show escaping conditions do invalidate the single alloc function

; Check a use that does not involve any of the checked instruction types

%struct.MYSTRUCT1 = type { i8* }
@globalstruct1 = internal global %struct.MYSTRUCT1 zeroinitializer, align 8
declare noalias i8* @malloc(i64)

define i32 @foo_1() {
  %t0 = tail call noalias i8* @malloc(i64 100)
  store i8* %t0, i8** getelementptr inbounds (%struct.MYSTRUCT1, %struct.MYSTRUCT1* @globalstruct1, i64 0, i32 0), align 8
  %t1 = getelementptr inbounds %struct.MYSTRUCT1, %struct.MYSTRUCT1* @globalstruct1, i64 0, i32 0
  %t2 = load i8*, i8** %t1, align 8
  %t3 = ptrtoint i8* %t2 to i64
  %t4 = trunc i64 %t3 to i32
  ret i32 %t4
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT1 = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Read Written
; CHECK: Multiple Value
; CHECK-NOT: Bottom Alloc Function
; CHECK: Safety data: Global instance

; Check a call that is not to free or a mem intrinsic

%struct.MYSTRUCT2 = type { i8* }
@globalstruct2 = internal global %struct.MYSTRUCT2 zeroinitializer, align 8
declare void @notafree(i8* nocapture)

define i32 @foo_2() {
  %t0 = tail call noalias i8* @malloc(i64 100)
  store i8* %t0, i8** getelementptr inbounds (%struct.MYSTRUCT2, %struct.MYSTRUCT2* @globalstruct2, i64 0, i32 0), align 8
  %t1 = getelementptr inbounds %struct.MYSTRUCT2, %struct.MYSTRUCT2* @globalstruct2, i64 0, i32 0
  %t2 = load i8*, i8** %t1, align 8
  tail call void @notafree(i8* %t2)
  %t3 = ptrtoint i8* %t2 to i64
  %t4 = trunc i64 %t3 to i32
  ret i32 %t4
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT2 = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Read Written
; CHECK: Multiple Value
; CHECK-NOT: Bottom Alloc Function
; CHECK: Safety data: Global instance

; Check a icmp that is not against null

%struct.MYSTRUCT3 = type { i8* }
@globalstruct3 = internal global %struct.MYSTRUCT3 zeroinitializer, align 8
declare void @notarealfunc(i32)

define i32 @foo_3() {
  %t0 = tail call noalias i8* @malloc(i64 100)
  store i8* %t0, i8** getelementptr inbounds (%struct.MYSTRUCT3, %struct.MYSTRUCT3* @globalstruct3, i64 0, i32 0), align 8
  %t1 = getelementptr inbounds %struct.MYSTRUCT3, %struct.MYSTRUCT3* @globalstruct3, i64 0, i32 0
  %t2 = load i8*, i8** %t1, align 8
  %t3 = icmp eq i8* %t2, %t2
  br i1 %t3, label %t4, label %t5
t4:
; <label>:4:
  tail call void @notarealfunc(i32 0)
  br label %t5
t5:
; <label>:5:
  tail call void @notarealfunc(i32 1)
  %t6 = ptrtoint i8* %t2 to i64
  %t7 = trunc i64 %t6 to i32
  ret i32 %t7
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT3 = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Read Written
; CHECK: Multiple Value
; CHECK-NOT: Bottom Alloc Function
; CHECK: Safety data: Global instance

; Check an intrinsic that is not a mem intrinsic

%struct.MYSTRUCT4 = type { i8* }

@globalstruct4 = internal global %struct.MYSTRUCT4 zeroinitializer, align 8
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)

define i32 @foo_4() {
  %t0 = tail call noalias i8* @malloc(i64 100)
  store i8* %t0, i8** getelementptr inbounds (%struct.MYSTRUCT4, %struct.MYSTRUCT4* @globalstruct4, i64 0, i32 0), align 8
  %t1 = getelementptr inbounds %struct.MYSTRUCT4, %struct.MYSTRUCT4* @globalstruct4, i64 0, i32 0
  %t2 = load i8*, i8** %t1, align 8
  call void @llvm.lifetime.start.p0i8(i64 0, i8* nonnull %t2)
  call void @llvm.lifetime.end.p0i8(i64 0, i8* nonnull %t2)
  %t3 = ptrtoint i8* %t2 to i64
  %t4 = trunc i64 %t3 to i32
  ret i32 %t4
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT4 = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Read Written
; CHECK: Multiple Value
; CHECK-NOT: Bottom Alloc Function
; CHECK: Safety data: Global instance

; Check a GEP instruction that does not feed a load

%struct.MYSTRUCT5 = type { i8* }
@globalstruct5 = internal global %struct.MYSTRUCT5 zeroinitializer, align 8
declare void @noir(i8*)

define i32 @foo_5() {
  %t0 = tail call noalias i8* @malloc(i64 100)
  store i8* %t0, i8** getelementptr inbounds (%struct.MYSTRUCT5, %struct.MYSTRUCT5* @globalstruct5, i64 0, i32 0), align 8
  %t1 = getelementptr inbounds %struct.MYSTRUCT5, %struct.MYSTRUCT5* @globalstruct5, i64 0, i32 0
  %t2 = load i8*, i8** %t1, align 8
  %t4 = getelementptr inbounds i8, i8* %t2, i64 0
  tail call void @noir(i8* %t4)
  %t3 = load i8*, i8** %t1, align 8
  %t5 = ptrtoint i8* %t3 to i64
  %t6 = trunc i64 %t5 to i32
  ret i32 %t6
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT5 = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Read Written
; CHECK: Multiple Value
; CHECK-NOT: Bottom Alloc Function
; CHECK: Safety data: Global instance

