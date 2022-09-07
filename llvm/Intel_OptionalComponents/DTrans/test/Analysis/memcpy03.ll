; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test that memcpy analysis does not crash when the destination pointer
; is an opaque or empty structure type. Previously, this crashed due to
; trying to track the values written to structure fields. (CMPLRLLVM-33256)

%struct.test01 = type opaque
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type opaque

@var01 = internal global %struct.test01* zeroinitializer
@p81 = internal global i8 zeroinitializer
@p82 = internal global i8 zeroinitializer

define void @test1(i64 %offset) {
  %p1 = call %struct.test01** @getter1()
  %p2 = call i8* @getter2()
  %p2.cast = bitcast i8* %p2 to %struct.test01**
  %p3 = call i8* @getter3()
  %p3.cast = bitcast i8* %p3 to %struct.test01*
  %gep1 = getelementptr inbounds %struct.test01*, %struct.test01** %p2.cast, i64 %offset
  %gep2 = getelementptr inbounds %struct.test01*, %struct.test01** %p1, i64 %offset
  %gep2.cast = bitcast %struct.test01** %gep2 to i8**
  %load = load i8*, i8** %gep2.cast
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %load, i8* %p3, i64 8, i1 false)
  ret void
}

define %struct.test01** @getter1() {
  ret %struct.test01** @var01
}

define i8* @getter2() {
  ret i8* @p81
}

define i8* @getter3() {
  ret i8* @p82
}

%struct.test02 = type {}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02 = type {}

@var02 = internal global %struct.test02* zeroinitializer
define void @test2(i64 %offset) {
  %p1 = call %struct.test02** @getter4()
  %p2 = call i8* @getter2()
  %p2.cast = bitcast i8* %p2 to %struct.test02**
  %p3 = call i8* @getter3()
  %p3.cast = bitcast i8* %p3 to %struct.test02*
  %gep1 = getelementptr inbounds %struct.test02*, %struct.test02** %p2.cast, i64 %offset
  %gep2 = getelementptr inbounds %struct.test02*, %struct.test02** %p1, i64 %offset
  %gep2.cast = bitcast %struct.test02** %gep2 to i8**
  %load = load i8*, i8** %gep2.cast
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %load, i8* %p3, i64 8, i1 false)
  ret void
}

define %struct.test02** @getter4() {
  ret %struct.test02** @var02
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
