; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s


; This test verifies that null pointers of types being converted
; by AOS-to-SOA get converted to a 64-bit integer 0.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i64, %struct.test01*, i16** }
%struct.test02 = type { i32, %struct.test01* }

; Verify a zero initialized pointer to the type gets the initializer updated.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer
; CHECK: @g_test01ptr = internal unnamed_addr global i64 0

; Verify a pointer to the type in another structure gets the initializer
; updated.
@g_test02inst = internal unnamed_addr global %struct.test02 { i32 1, %struct.test01* null }
; CHECK: @g_test02inst = internal unnamed_addr global %__SOADT_struct.test02 { i32 1, i64 0 }


define i32 @main(i32 %argc, i8** %argv) {
  %struct_array = call i8* @malloc(i64 320)
  %st_ptr = bitcast i8* %struct_array to %struct.test01*
  call void @test01(%struct.test01* %st_ptr)

  %ptr_array = call i8* @malloc(i64 16)
  %st_ptr_array = bitcast i8* %ptr_array to %struct.test01**
  call void @test02(%struct.test01** %st_ptr_array)

  %ptrptr_array = call i8* @malloc(i64 16)
  %st_ptrptr_array = bitcast i8* %ptrptr_array to %struct.test01***
  call void @test03(%struct.test01*** %st_ptrptr_array)

  ret i32 0
}

; This test verifies that pointers to null of the structure type being
; transformed, such as in compare and phi instructions get updated.
; test01 will be cloned to a new function taking an i64 as the input parameter.
define void @test01(%struct.test01* %in) {
  %cmp = icmp eq %struct.test01* %in, null
  br i1 %cmp, label %is_null, label %not_null
is_null:
  br label %merge
not_null:
  br label %merge
merge:
  %chosen = phi %struct.test01* [ %in, %not_null ], [ null, %is_null ]
  ret void
}
; CHECK: define internal void @test01.1(i64 %in)
; CHECK:   %cmp = icmp eq i64 %in, 0
; CHECK:   %chosen = phi i64 [ %in, %not_null ], [ 0, %is_null ]


; This test verifies that pointers to null for store instruction get
; updated.
define void @test02(%struct.test01** %in) {
  store %struct.test01* null, %struct.test01** %in
  ret void
}
; CHECK: define internal void @test02.2(i64* %in)
; CHECK:   store i64 0, i64* %in


; This test verifies that pointers-to-pointers of the structure type
; being transformed, are just converted to pointers to the peeled
; index type.
define void @test03(%struct.test01*** %in) {
  store %struct.test01** null, %struct.test01*** %in
  ret void
}
; CHECK define internal void@test03.3(i64** %in) {
; CHECK:   store i64* null, i64** %in

declare i8* @malloc(i64)
