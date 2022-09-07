; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies the behavior of pointer carried safety checks being
; propagated to the base type of pointer members within a structure that
; violates a safety condition.

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"


; This case should trigger a "System Object" as a pointer carried
; safety condition when a LibFunc is called with an object type.
;
; Embed a pointer to a user type into the definition of a system
; object for testing. This is not the real definition of the system
; object struct.stat
%struct.stat = type { i32, %struct.test00.a* }
%struct.test00.a = type { i32, i32, i8* }
define void @test00() {
 %stat = alloca %struct.stat
 %res = call i32 @__fxstat64(i32 1, i32 5, %struct.stat* %stat)

  ret void
}
; CHECK-LABEL:  LLVMType: %struct.stat = type { i32, %struct.test00.a* }
; CHECK: Safety data:
; CHECK-SAME: System object
; CHECK-LABEL: LLVMType: %struct.test00.a = type { i32, i32, i8* }
; CHECK: Safety data:
; CHECK-SAME: System object


; This case should trigger the "Ambiguous pointer target" safety bit
; on the contained structures, and as a pointer carried safety bit.
%struct.test01.a = type { i32, i32 }
%struct.test01.b = type { i64, %struct.test01.a* }
%struct.test01.c = type { %struct.test01.b }
%struct.test01.d = type { %struct.test01.b }

define void @test01() {
  %mem = call i8* @malloc(i64 16)
  %c = bitcast i8* %mem to %struct.test01.c*
  %fc0 = getelementptr %struct.test01.c, %struct.test01.c* %c, i64 0, i32 0 ,i32 0
  store i64 1, i64* %fc0

  %d = bitcast i8* %mem to %struct.test01.d*
  %fd0 = getelementptr %struct.test01.d, %struct.test01.d* %d, i64 0, i32 0, i32 0
  store i64 2, i64* %fd0

  call void @llvm.memset.p0i8.i64(i8* %mem, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01.a = type { i32, i32 }
; CHECK: Safety data:
; CHECK-SAME: Ambiguous pointer target
; CHECK-LABEL: LLVMType: %struct.test01.b = type { i64, %struct.test01.a* }
; CHECK: Safety data:
; CHECK-SAME: Ambiguous pointer target
; CHECK-LABEL: LLVMType: %struct.test01.c = type { %struct.test01.b }
; CHECK: Safety data:
; CHECK-SAME: Ambiguous pointer target
; CHECK-LABEL: LLVMType: %struct.test01.d = type { %struct.test01.b }
; CHECK: Safety data:
; CHECK-SAME: Ambiguous pointer target


; This case should treat the "Bad memfunc manipulation" as pointer carried
; to the referenced structure, %struct.test02.a.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i64, %struct.test02.a* }
define void @test02(%struct.test02.b* %in) {
  %local = alloca [16 x i8]
  %dest = bitcast [16 x i8]* %local to i8*

  %src = bitcast %struct.test02.b* %in to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest, i8* %src, i64 16, i1 false)

  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data:
; CHECK-SAME: Bad memfunc manipulation
; CHECK-LABEL: LLVMType: %struct.test02.b = type { i64, %struct.test02.a* }
; CHECK: Safety data:
; CHECK-SAME: Bad memfunc manipulation


; This case triggers the "Unsafe pointer merge" safety condition, and
; checks that it is pointer carried.
%struct.test03.a = type { i32, i32, %struct.test03.c* }
%struct.test03.b = type { i64, i64, %struct.test03.d* }
%struct.test03.c = type { i32 }
%struct.test03.d = type { i32 }
define void @test03(%struct.test03.a** %p1, %struct.test03.b** %p2) {
  %p1n = bitcast %struct.test03.a** %p1 to i64*
  %p2n = bitcast %struct.test03.b** %p2 to i64*
  %p3n = select i1 undef, i64* %p1n, i64* %p2n
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test03.a = type { i32, i32, %struct.test03.c* }
; CHECK: Safety data:
; CHECK-SAME: Unsafe pointer merge
; CHECK-LABEL: LLVMType: %struct.test03.b = type { i64, i64, %struct.test03.d* }
; CHECK: Safety data:
; CHECK-SAME: Unsafe pointer merge
; CHECK-LABEL: LLVMType: %struct.test03.c = type { i32 }
; CHECK: Safety data:
; CHECK-SAME: Unsafe pointer merge
; CHECK-LABEL: LLVMType: %struct.test03.d = type { i32 }
; CHECK: Safety data:
; CHECK-SAME: Unsafe pointer merge


; This case triggers the "Unhandled use" safety condition, and
; checks that is a pointer carried.
%struct.test04.a = type { %struct.test04.c*, i32, i32 }
%struct.test04.b = type { %struct.test04.d*, i32, i32, i32 }
%struct.test04.c = type { i32 }
%struct.test04.d = type { i64 }
define void @test04(%struct.test04.a* %p1, %struct.test04.b* %p2) {
  %t1 = ptrtoint %struct.test04.a* %p1 to i64
  %t2 = ptrtoint %struct.test04.b* %p2 to i64
  %offset = sub i64 %t1, %t2
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04.a = type { %struct.test04.c*, i32, i32 }
; CHECK: Safety data:
; CHECK-SAME: Unhandled use
; CHECK-LABEL: LLVMType: %struct.test04.b = type { %struct.test04.d*, i32, i32, i32 }
; CHECK: Safety data:
; CHECK-SAME: Unhandled use
; CHECK-LABEL: LLVMType: %struct.test04.c = type { i32 }
; CHECK: Safety data:
; CHECK-SAME: Unhandled use
; CHECK-LABEL: LLVMType: %struct.test04.d = type { i64 }
; CHECK: Safety data:
; CHECK-SAME: Unhandled use


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare  i8* @malloc(i64)
declare i32 @__fxstat64(i32, i32, %struct.stat*)
