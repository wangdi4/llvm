; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Call memcpy with matched struct pointers.
; This is an safe use.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %s1, %struct.test01* %s2) {
  %p1 = bitcast %struct.test01* %s1 to i8*
  %p2 = bitcast %struct.test01* %s2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found


; Call memcpy with mismatched struct pointers.
; This is an unsafe use.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i16, i16, i32 }
define void @test02(%struct.test02.a* %sa, %struct.test02.b* %sb) {
  %pa = bitcast %struct.test02.a* %sa to i8*
  %pb = bitcast %struct.test02.b* %sb to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pa, i8* %pb, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc manipulation
; CHECK-LABEL: LLVMType: %struct.test02.b = type { i16, i16, i32 }
; CHECK: No Value
; CHECK: No Value
; CHECK: No Value
; CHECK: Safety data: Bad memfunc manipulation


; Call memcpy with destination as a structure type, but source as a
; fundamental pointer type.
; This is an unsafe use.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %s1, i8* %b1) {
  %p1 = bitcast %struct.test03* %s1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %b1, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc manipulation

; Call memcpy with source as a structure type, but destination as a
; fundamental pointer type.
; This is an unsafe use.
%struct.test04 = type { i32, i32 }
define void @test04(i8* %b1, %struct.test04* %s1) {
  %p1 = bitcast %struct.test04* %s1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %b1, i8* %p1, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: No Value
; CHECK: No Value
; CHECK: Safety data: Bad memfunc manipulation


; Call memcpy with matched struct pointers, but only part of the
; structure is copied.
; This should be marked with the memfunc partial write safety.
%struct.test05 = type { i32, i32, i32, i32 }
define void @test05(%struct.test05* %s1, %struct.test05* %s2) {
  %p1 = bitcast %struct.test05* %s1 to i8*
  %p2 = bitcast %struct.test05* %s2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test05 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Memfunc partial write


; Test with memcpy that is performing pointer-to-pointer copy.
; This is a safe use.
%struct.test06.a = type { i32, i32, i32, i32 }
%struct.test06.b = type { i32, %struct.test06.a** }
define void @test06(%struct.test06.b* %dst, %struct.test06.b* %src, i32 %n) {
entry:
  %pp = getelementptr inbounds %struct.test06.b, %struct.test06.b* %dst, i64 0, i32 1
  %0 = bitcast %struct.test06.a*** %pp to i8**
  %1 = load i8*, i8** %0, align 8
  %pp1 = getelementptr inbounds %struct.test06.b, %struct.test06.b* %src, i64 0, i32 1
  %2 = bitcast %struct.test06.a*** %pp1 to i8**
  %3 = load i8*, i8** %2, align 8
  %conv = sext i32 %n to i64
  %mul = shl nsw i64 %conv, 3
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* %3, i64 %mul, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test06.a = type { i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK-LABEL: LLVMType: %struct.test06.b = type { i32, %struct.test06.a** }
; CHECK: Safety data: No issues found


; Test with memcpy that is passed a pointer to an array that is a
; member field of a parent structure, and the size being copied matches
; the size of the member field.
; This is a safe use.
%struct.test07 = type { i32, i32, i32, [50 x i32] }
define void @test07(%struct.test07* %dest, %struct.test07* %src) {
  %data = getelementptr inbounds %struct.test07, %struct.test07* %dest, i64 0, i32 3
  %t0 = bitcast [50 x i32]* %data to i8*
  %data1 = getelementptr inbounds %struct.test07, %struct.test07* %src, i64 0, i32 3
  %t1 = bitcast [50 x i32]* %data1 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 200, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test07 = type { i32, i32, i32, [50 x i32] }
; CHECK: Safety data: Memfunc partial write


; Test with memcpy that is passed a pointer to an array that is a
; member field of a parent structure, and the size equals the
; size of two adjacent fields
; The structure that contains it should be marked as unsafe.
%struct.test08 = type { i32, i32, i32, [50 x i32], [50 x i32] }
define void @test08(%struct.test08* %dest, %struct.test08* %src) {
  %data = getelementptr inbounds %struct.test08, %struct.test08* %dest, i64 0, i32 3
  %t0 = bitcast [50 x i32]* %data to i8*
  %data1 = getelementptr inbounds %struct.test08, %struct.test08* %src, i64 0, i32 3
  %t1 = bitcast [50 x i32]* %data1 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 400, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test08 = type { i32, i32, i32, [50 x i32], [50 x i32] }
; CHECK: Safety data: Memfunc partial write


; Test with memcpy that is passed a pointer to a structure that is a
; member field of a parent structure.
; This is a safe use because the entire sub-structure is being copied,
; and so is just the use of field of the parent structure.
%struct.test09.b = type { i32, i32, i32, %struct.test09.a }
%struct.test09.a = type { [20 x i32] }
define void @test09(%struct.test09.b* %dest, %struct.test09.b* %src) {
  %d = getelementptr inbounds %struct.test09.b, %struct.test09.b* %dest, i64 0, i32 3
  %t0 = bitcast %struct.test09.a* %d to i8*
  %d1 = getelementptr inbounds %struct.test09.b, %struct.test09.b* %src, i64 0, i32 3
  %t1 = bitcast %struct.test09.a* %d1 to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 80, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test09.a = type { [20 x i32] }
; CHECK: Safety data: Nested structure
; CHECK-LABEL: LLVMType: %struct.test09.b = type { i32, i32, i32, %struct.test09.a }
; CHECK: Safety data: Memfunc partial write | Contains nested structure


; Test with memcpy where the source and target types match, but the source
; pointer is a field within another structure, while the destination is not.
; This is could be considered a safe use, but for simplicity for the transforms
; that would need to rewrite the memcpy this will be marked as invalid, for now.
; If this is changed to be supported in the future, then transformation code may
; also need to be updated.
%struct.test10.a = type { i32, i32, i32, i32, i32 }
%struct.test10.b = type { i32, i32, i32, %struct.test10.a }
define void @test10(%struct.test10.a* %dest, %struct.test10.b* %src) {
  %dest_ptr = bitcast %struct.test10.a* %dest to i8*
  %src_addr = getelementptr inbounds %struct.test10.b, %struct.test10.b* %src, i64 0, i32 3
  %src_ptr = bitcast %struct.test10.a* %src_addr to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest_ptr, i8* %src_ptr, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test10.a = type { i32, i32, i32, i32, i32 }
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc manipulation | Nested structure
; CHECK-LABEL: LLVMType: %struct.test10.b = type { i32, i32, i32, %struct.test10.a }
; CHECK: No Value
; CHECK: No Value
; CHECK: No Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc manipulation | Contains nested structure


; Test with memcpy where the source and target types match, but the destination
; pointer is a field within another structure.
; This is could be considered a safe use, but for simplicity for the transforms
; that would need to rewrite the memcpy this will be marked as invalid, for now.
%struct.test11.a = type { i32, i32, i32, i32, i32 }
%struct.test11.b = type { i32, i32, i32, %struct.test11.a }
define void @test11(%struct.test11.a* %src, %struct.test11.b* %dest) {
  %src_ptr = bitcast %struct.test11.a* %src to i8*
  %dest_addr = getelementptr inbounds %struct.test11.b, %struct.test11.b* %dest, i64 0, i32 3
  %dest_ptr = bitcast %struct.test11.a* %dest_addr to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest_ptr, i8* %src_ptr, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test11.a = type { i32, i32, i32, i32, i32 }
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc manipulation | Nested structure
; CHECK-LABEL: LLVMType: %struct.test11.b = type { i32, i32, i32, %struct.test11.a }
; CHECK: No Value
; CHECK: No Value
; CHECK: No Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc manipulation | Contains nested structure


; Test with memcpy where the source and target types match, but the pointers
; are addresses of a pointer to structure field.
; This is a safe use.
%struct.test12.a = type { i32, i32, i32, i32, i32 }
%struct.test12.b = type { i32, i32, i32, %struct.test12.a* }
define void @test12(%struct.test12.b* %dest, %struct.test12.b* %src) {
  %dest_addr = getelementptr inbounds %struct.test12.b, %struct.test12.b* %dest, i64 0, i32 3
  %dest_ptr = bitcast %struct.test12.a** %dest_addr to i8*
  %src_addr = getelementptr inbounds %struct.test12.b, %struct.test12.b* %src, i64 0, i32 3
  %src_ptr = bitcast %struct.test12.a** %src_addr to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest_ptr, i8* %src_ptr, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test12.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK-LABEL: LLVMType: %struct.test12.b = type { i32, i32, i32, %struct.test12.a* }
; CHECK: Safety data: Memfunc partial write


; Test with memcpy where the source and target types match, but the pointers
; are addresses of a pointer to structure field, and the copy writes more than
; a single field.
; This is an unsafe use.
%struct.test13.a = type { i32, i32, i32, i32, i32 }
%struct.test13.b = type { i32, i32, i32, %struct.test13.a*, i64 }
define void @test13(%struct.test13.b* %dest, %struct.test13.b* %src) {
  %dest_addr = getelementptr inbounds %struct.test13.b, %struct.test13.b* %dest, i64 0, i32 3
  %dest_ptr = bitcast %struct.test13.a** %dest_addr to i8*
  %src_addr = getelementptr inbounds %struct.test13.b, %struct.test13.b* %src, i64 0, i32 3
  %src_ptr = bitcast %struct.test13.a** %src_addr to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest_ptr, i8* %src_ptr, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test13.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK-LABEL: LLVMType: %struct.test13.b = type { i32, i32, i32, %struct.test13.a*, i64 }
; CHECK: Safety data: Memfunc partial write


; This test checks using memcpy with the address of a scalar member of the
; structure
; This is an safe use, but needs to track that the memfunc did a partial structure write.
%struct.test14 = type { i32, i32, i32, i32, i32 }
define void @test14(%struct.test14* %a, %struct.test14* %b) {
  %d = getelementptr inbounds %struct.test14, %struct.test14* %a, i64 0, i32 2
  %t0 = bitcast i32* %d to i8*
  %s = getelementptr inbounds %struct.test14, %struct.test14* %b, i64 0, i32 2
  %t1 = bitcast i32* %s to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test14 = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: Memfunc partial write


; This test checks using memcpy with the address of a scalar member of the
; structure to verify both source and target data structures get marked.
; This is an unsafe use.
%struct.test15.a = type { i32, i32, i32 }
%struct.test15.b = type { i32, i32, i32, i32, i32 }
define void @test15(%struct.test15.a* %a, %struct.test15.b* %b) {
  %t0 = bitcast %struct.test15.a* %a to i8*
  %s = getelementptr inbounds %struct.test15.b, %struct.test15.b* %b, i64 0, i32 2
  %t1 = bitcast i32* %s to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test15.a = type { i32, i32, i32 }
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc manipulation
; CHECK-LABEL: LLVMType: %struct.test15.b = type { i32, i32, i32, i32, i32 }
; CHECK: No Value
; CHECK: No Value
; CHECK: No Value
; CHECK: No Value
; CHECK: No Value
; CHECK: Safety data: Bad memfunc manipulation


; This test checks using memcpy with a zero-sized copy.
; This is an safe use.
%struct.test16 = type { i32, i32, i32, i32, i32 }
define void @test16(%struct.test16* %a, %struct.test16* %b) {
  %d = getelementptr inbounds %struct.test16, %struct.test16* %a, i64 0, i32 2
  %t0 = bitcast i32* %d to i8*
  %s = getelementptr inbounds %struct.test16, %struct.test16* %b, i64 0, i32 2
  %t1 = bitcast i32* %s to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 0, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test16 = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found


; This test checks using memcpy to with the same types, but with a different
; set of fields within a structure.
; This could be considered a safe use, but it is not currently supported.
; If this is changed to be supported in the future, then transformation code may
; also need to be updated.
%struct.test17 = type { i32, i32, i32, i32, i32 }
define void @test17(%struct.test17* %a, %struct.test17* %b) {
  %d = getelementptr inbounds %struct.test17, %struct.test17* %a, i64 0, i32 1
  %t0 = bitcast i32* %d to i8*
  %s = getelementptr inbounds %struct.test17, %struct.test17* %b, i64 0, i32 3
  %t1 = bitcast i32* %s to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test17 = type { i32, i32, i32, i32, i32 }
; CHECK: No Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: No Value
; CHECK: No Value
; CHECK: Safety data: Bad memfunc manipulation

; This test checks using memcpy to with the same types, but with a different
; set of elements out of an array.
; This could be considered a safe use, but it is not currently supported.
; If this is changed to be supported in the future, then transformation code may
; also need to be updated.
%struct.test18 = type { i32, i32 }
%array.test18 = type [6 x %struct.test18*]
define void @test18(%array.test18* %a, %array.test18* %b) {
  %d = getelementptr inbounds %array.test18, %array.test18* %a, i64 0, i32 1
  %t0 = bitcast %struct.test18** %d to i8*
  %s = getelementptr inbounds %array.test18, %array.test18* %b, i64 0, i32 3
  %t1 = bitcast %struct.test18** %s to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %t0, i8* %t1, i64 24, i1 false)
  ret void
}

; In this case there is an array being copied from a global array to an
; array member of a structure. In this case, both the source and
; destination types have [8 x i8]* as their dominant types, and both
; are pointer to member elements. However, the current analysis will
; mark this as 'Unhandled use' because the memcpy pointer to member
; support requires the member to be part of a structure. This could
; be relaxed in the future.
%struct.test19 = type { i32, i32, i32, [8 x i8], i32 }
@test19.glob = private global %struct.test19* zeroinitializer
@test19.str = private unnamed_addr constant [8 x i8] c"0123456\00"
define void @test19(%struct.test19* %str_in) {
  %blob = call i8* @malloc(i64 24)
  store i8* %blob, i8** bitcast (%struct.test19** @test19.glob to i8**)
  %str_data = getelementptr inbounds i8, i8* %blob, i64 12
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %str_data, i8*getelementptr inbounds ([8 x i8], [8 x i8]* @test19.str, i64 0, i64 0), i64 31, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test19 = type { i32, i32, i32, [8 x i8], i32 }
; CHECK: Safety data: Global pointer | Unhandled use


; This case is equivalent to test19, but with the global array now being
; the destination, and the structure member being the source array. This case
; is also currently marked as unhandled use.
%struct.test20 = type { i32, i32, i32, [9 x i8], i32 }
@test20.glob = private global %struct.test20* zeroinitializer
@test20.str = private unnamed_addr constant [9 x i8] c"01234567\00"
define void @test20(%struct.test20* %str_in) {
  %blob = call i8* @malloc(i64 28)
  store i8* %blob, i8** bitcast (%struct.test20** @test20.glob to i8**)
  %str_data = getelementptr inbounds i8, i8* %blob, i64 12
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8*getelementptr inbounds ([9 x i8], [9 x i8]* @test20.str, i64 0, i64 0), i8* %str_data, i64 5, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test20 = type { i32, i32, i32, [9 x i8], i32 }
; CHECK: Safety data: Global pointer | Unhandled use

; Array types get printed last so these checks aren't with their IR.

; CHECK-LABEL: LLVMType: [6 x %struct.test18*]
; CHECK: Safety data: Unhandled use

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare noalias i8* @malloc(i64)
