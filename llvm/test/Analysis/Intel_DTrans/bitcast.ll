; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; This test currently fails because bitcast safety checks aren't implemented.
; All bitcasts are being marked as unsafe until proper handling is in place.
; XFAIL: *

; Cast of allocated buffer to a struct pointer.
%struct.test01 = type { i32, i32 }
define void @test1() {
  %p = call noalias i8* @malloc(i64 8)
  %s1 = bitcast i8* %p to %struct.test01*
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Cast of arbitrary i8* to struct pointer.
%struct.test02.a = type { i32, i8 }
%struct.test02.b = type { i32, i32 }
@p.test2 = external global %struct.test02.a
define void @test2() {
  %s = bitcast i8* getelementptr( %struct.test02.a, %struct.test02.a* @p.test2,
                                  i64 0, i32 1) to %struct.test02.b*
  ret void
}

; FIXME: %struct.test02.a should also be marked as unsafe here.
;        That's being missed because the analysis doesn't follow the field
;        address use yet.
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02.b = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of non-alloc pointer value
%struct.test03 = type { i32, i32 }
define void @test3(i8* %p) {
  %s = bitcast i8* %p to %struct.test03*
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of struct pointer to i8* for memset.
%struct.test04 = type { i32, i32 }
define void @test4(%struct.test04* %s) {
  %p = bitcast %struct.test04* %s to i8*
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i32 8, i1 false)
  ret void
}

; FIXME: Change this check when safe types get printed
; CHECK-NOT: LLVMType: %struct.test04 = type { i32, i32 }

; Cast of matched struct pointers to i8* for memcpy.
%struct.test05 = type { i32, i32 }
define void @test5(%struct.test05* %s1, %struct.test05* %s2) {
  %p1 = bitcast %struct.test05* %s1 to i8*
  %p2 = bitcast %struct.test05* %s2 to i8*
  call void @llvm.memcpy.p0i8.i64(i8* %p1, i8* %p2, i64 8, i32 8, i1 false)
  ret void
}

; FIXME: Change this check when safe types get printed
; CHECK-NOT: LLVMType: %struct.test05 = type { i32, i32 }

; Cast of mismatched struct pointers to i8* for memcpy.
%struct.test06.a = type { i32, i32 }
%struct.test06.b = type { i16, i16, i32 }
define void @test6(%struct.test06.a* %sa, %struct.test06.b* %sb) {
  %pa = bitcast %struct.test06.a* %sa to i8*
  %pb = bitcast %struct.test06.b* %sb to i8*
  call void @llvm.memcpy.p0i8.i64(i8* %pa, i8* %pb, i64 8, i32 8, i1 false)
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad casting

; Cast of matched struct pointers to i8* for memmove.
%struct.test07 = type { i32, i32 }
define void @test7(%struct.test07* %s1, %struct.test07* %s2) {
  %p1 = bitcast %struct.test07* %s1 to i8*
  %p2 = bitcast %struct.test07* %s2 to i8*
  call void @llvm.memmove.p0i8.i64(i8* %p1, i8* %p2, i64 8, i32 8, i1 false)
  ret void
}

; FIXME: Change this check when safe types get printed
; CHECK-NOT: LLVMType: %struct.test07 = type { i32, i32 }

; Cast of mismatched struct pointers to i8* for memmove.
%struct.test08.a = type { i32, i32 }
%struct.test08.b = type { i16, i16, i32 }
define void @test8(%struct.test08.a* %sa, %struct.test08.b* %sb) {
  %pa = bitcast %struct.test08.a* %sa to i8*
  %pb = bitcast %struct.test08.b* %sb to i8*
  call void @llvm.memmove.p0i8.i64(i8* %pa, i8* %pb, i64 8, i32 8, i1 false)
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test08.a = type { i32, i32 }
; CHECK: Safety data: Bad casting 
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test08.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad casting 

; Store of pointer to struct cast as an pointer-sized integer.
%struct.test09.a = type { i32, i32 }
%struct.test09.b = type { i32, i32, %struct.test09.a* }
define void @test9( %struct.test09.a* %sa, %struct.test09.b* %sb ) {
  %a.as.i = ptrtoint %struct.test09.a* %sa to i64
  %pb.a = getelementptr %struct.test09.b, %struct.test09.b* %sb, i64 0, i32 2
  %pb.a.as.pi = bitcast %struct.test09.a** %pb.a to i64*
  store i64 %a.as.i, i64* %pb.a.as.pi
  ret void
}

; FIXME: Change this check when safe types get printed
; CHECK-NOT: LLVMType: %struct.test09.a = type { i32, i32 }
; CHECK-NOT: LLVMType: %struct.test09.b = type { i32, i32, %struct.test09.a* }

; Mismatched store of pointer to struct cast as an pointer-sized integer.
%struct.test10.a = type { i32, i32 }
%struct.test10.b = type { i32, i32, %struct.test10.a* }
define void @test10( %struct.test10.b* %sb ) {
  %b.as.i = ptrtoint %struct.test10.b* %sb to i64
  %pb.a = getelementptr %struct.test10.b, %struct.test10.b* %sb, i64 0, i32 2
  %pb.a.as.pi = bitcast %struct.test10.a** %pb.a to i64*
  store i64 %b.as.i, i64* %pb.a.as.pi
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test10.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test10.b = type { i32, i32, %struct.test10.a* }
; CHECK: Safety data: Bad casting

; Copy of pointer to struct from one aggregate to another via GEP, bitcast and
; load/store.
%struct.test11.a = type { i32, %struct.test11.a* }
%struct.test11.b = type { i32, i32, %struct.test11.a* }
define void @test11( %struct.test11.a* %sa, %struct.test11.b* %sb ) {
  %ppaa = getelementptr %struct.test11.a, %struct.test11.a* %sa, i64 0, i32 1
  %ppaa.as.pi = bitcast %struct.test11.a** %ppaa to i64*
  %paa.as.i = load i64, i64* %ppaa.as.pi
  %ppba = getelementptr %struct.test11.b, %struct.test11.b* %sb, i64 0, i32 2
  %ppba.as.pi = bitcast %struct.test11.a** %ppba to i64*
  store i64 %paa.as.i, i64* %ppba.as.pi
  ret void
}

; FIXME: Change this check when safe types get printed
; CHECK-NOT: LLVMType: %struct.test11.a = type { i32, i%struct.test11.a* }
; CHECK-NOT: LLVMType: %struct.test11.b = type { i32, i32, %struct.test09.a* }

; Copy of pointer to struct from one aggregate to another via GEP, bitcast and
; load/store.
%struct.test12.a = type { i32, %struct.test12.b* }
%struct.test12.b = type { i32, i32, %struct.test12.a* }
define void @test12( %struct.test12.a* %sa, %struct.test12.b* %sb ) {
  %ppab = getelementptr %struct.test12.a, %struct.test12.a* %sa, i64 0, i32 1
  %ppab.as.pi = bitcast %struct.test12.b** %ppab to i64*
  %pab.as.i = load i64, i64* %ppab.as.pi
  %ppba = getelementptr %struct.test12.b, %struct.test12.b* %sb, i64 0, i32 2
  %ppba.as.pi = bitcast %struct.test12.a** %ppba to i64*
  store i64 %pab.as.i, i64* %ppba.as.pi
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test12.a = type { i32, %struct.test12.b* }
; CHECK: Safety data: Bad casting
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test12.b = type { i32, i32, %struct.test12.a* }
; CHECK: Safety data: Bad casting

; Cast of non-instruction to struct pointer.
; This case currently gets flagged because it uses an operator GEP.
; This use actually meets the pointer copy idiom and so is safe.
%struct.test13 = type { i32, %struct.test13* }
@p.test13 = external global %struct.test13
define void @test13(%struct.test13* %pa.arg) {
  %ppa.as.pi = bitcast %struct.test13** getelementptr(
                                          %struct.test13,
                                          %struct.test13* @p.test13,
                                          i64 0, i32 1) to i64*
  %pa.as.i = load i64, i64* %ppa.as.pi
  %ppa.arg = getelementptr %struct.test13, %struct.test13* %pa.arg, i64 0, i32 1
  %ppa.arg.as.pi = bitcast %struct.test13** %ppa.arg to i64*
  store i64 %pa.as.i, i64* %ppa.arg.as.pi
  ret void
}

; FIXME: Change this check when safe types get printed
; CHECK-NOT: LLVMType: %struct.test13 = type { i32, i%struct.test13* }

declare noalias i8* @malloc(i64)
declare void @free(i8* nocapture)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1)
declare void @llvm.memcpy.p0i8.i64(i8* nocapture writeonly,
                                   i8* nocapture readonly, i64, i32, i1)
declare void @llvm.memmove.p0i8.i64(i8* nocapture writeonly,
                                    i8* nocapture readonly, i64, i32, i1)
