; UNSUPPORTED: enable-opaque-pointers
; RUN: opt <%s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s
; RUN: opt <%s -S -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -dtrans-aostosoa-index32 -dtrans-aostosoa-heur-override=struct.test01 -whole-program-assume 2>&1 | FileCheck %s


; This test verifies that GetElementPointer instructions get transformed
; properly during the AOS to SOA transformation when using a 32-bit peeling
; index.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32, %struct.test01*, %struct.test01dep*, i8* }

; This structure is used for verifying accesses within dependent data
; structures that get handled.
%struct.test01dep = type { i16, %struct.test01*, %struct.test01* }

; Pointer to the type being transformed.
@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

; Pointer to container that holds a pointer to type being transformed.
@g_test01depptr = internal unnamed_addr global %struct.test01dep* zeroinitializer


define i32 @main(i32 %argc, i8** %argv) {
  %alloc01 = call i8* @calloc(i64 10, i64 40)
  %struct01_mem = bitcast i8* %alloc01 to %struct.test01*

  call void @test01(i64 2)
  call void @test02()

  ret i32 0
}

; This function tests the basic transformations of GEP instructions when
; the index type is 32-bits.
define internal void @test01(i64 %idx1) {
; CHECK-LABEL: define internal void @test01(i64 %idx1)

; Verify 32-bit index type get used.
  %base = load %struct.test01*, %struct.test01** @g_test01ptr
; CHECK:  %base = load i32, i32* @g_test01ptr

  ; Test with GEP that accesses a constant array index into the SOA type.
  ; Because we are guaranteeing the indexing can be done with 32 bits, we
  ; first truncate the argument. (We don't currently perform constant folding
  ; during the GEP conversion, so an explicit trunc instruction will occur
  ; at this phase)
  %array_elem1 = getelementptr %struct.test01, %struct.test01* %base, i64 5
; CHECK: [[TRUNC1:%[0-9]+]] = trunc i64 5 to i32
; CHECK:  %array_elem1 = add i32 %base, [[TRUNC1]]

  ; Test with GEP that accesses a non-constant array index into the SOA type.
  ; Here we need to add the GEP index amount to the peeling index value.
  %arrayidx_elem2 = getelementptr %struct.test01, %struct.test01* %base, i64 %idx1
; CHECK: [[TRUNC2:%[0-9]+]] = trunc i64 %idx1 to i32
; CHECK:  %arrayidx_elem2 = add i32 %base, [[TRUNC2]]

  ; Test with GEP to access a field element, where the first GEP index equals 0.
  ; In this case, we do not need to perform an addition to the array index that
  ; will be accessed.
  %x_addr2 = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 0, i32 0
  store i32 20, i32* %x_addr2
; CHECK:  [[GLOB_ADDR3:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:  [[ARRAY_ADDR3:%[0-9]+]] = load i32*, i32** [[GLOB_ADDR3]]
; CHECK:  [[ZELEM_ZDDR3:%[0-9]+]] = zext i32 %array_elem1 to i64
; CHECK:  %x_addr2 = getelementptr i32, i32* [[ARRAY_ADDR3]], i64 [[ZELEM_ZDDR3]]
; CHECK:  store i32 20, i32* %x_addr2

  ; Test with GEP that directly uses index to get to a field element. Here the
  ; addition of the array_elem1 with index should be performed to determine the
  ; peeled index value for which array element of the SOA should be accessed.
  %y_addr3 = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 %idx1, i32 1
  %new_val = load i32, i32* %x_addr2
  store i32 %new_val, i32* %y_addr3
; CHECK:  [[GLOB_ADDR4:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:  [[ARRAY_ADDR4:%[0-9]+]] = load i32*, i32** [[GLOB_ADDR4]]
; CHECK:  [[TRUNC4:%[0-9]+]] = trunc i64 %idx1 to i32
; CHECK:  [[ELEM_ADDR4:%[0-9]+]] = add i32 %array_elem1, [[TRUNC4]]
; CHECK:  [[ZELEM_ADDR4:%[0-9]+]] = zext i32 [[ELEM_ADDR4]] to i64
; CHECK:  %y_addr3 = getelementptr i32, i32* [[ARRAY_ADDR4]], i64 [[ZELEM_ADDR4]]
; CHECK:  %new_val = load i32, i32* %x_addr2
; CHECK:  store i32 %new_val, i32* %y_addr3

  ; Test with GEP to access a field element that is a pointer to the type being
  ; transformed. In this case a inttoptr cast will be necessary between the pre/post
  ; processing phases, but will be removed by the time the IR is emitted. In the
  ; end, we should just see i64 types being used for the loaded value.
  %nextptr_addr4 = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 %idx1, i32 3
  %nextptr_val = load %struct.test01*, %struct.test01** %nextptr_addr4
  %nextptr_cmp = icmp eq %struct.test01* %nextptr_val, null
; CHECK:  [[GLOB_ADDR5:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 3
; CHECK:  [[ARRAY_ADDR5:%[0-9]+]] = load i32*, i32** [[GLOB_ADDR5]]
; CHECK:  [[TRUNC5:%[0-9]+]] = trunc i64 %idx1 to i32
; CHECK:  [[ELEM_ADDR5:%[0-9]+]] = add i32 %array_elem1, [[TRUNC5]]
; CHECK:  [[ZELEM_ADDR5:%[0-9]+]] = zext i32 [[ELEM_ADDR5]] to i64
; CHECK:  %nextptr_addr4 = getelementptr i32, i32* [[ARRAY_ADDR5]], i64 [[ZELEM_ADDR5]]
; CHECK:  %nextptr_val = load i32, i32* %nextptr_addr4
; CHECK:  %nextptr_cmp = icmp eq i32 %nextptr_val, 0

  ret void
}

; Test with using GEP instructions on types being transformed that are loaded from
; another structure.
define internal void @test02() {
; CHECK-LABEL: define internal void @test02()

  %dep_base = load %struct.test01dep*, %struct.test01dep** @g_test01depptr

  ; These instructions will be handled by the base class transformation
  %dep_field1 = getelementptr %struct.test01dep, %struct.test01dep* %dep_base, i64 0, i32 1
  %dep_field2 = getelementptr %struct.test01dep, %struct.test01dep* %dep_base, i64 0, i32 2

  %dep_field_val1 = load %struct.test01*, %struct.test01** %dep_field1
  %dep_field_val2 = load %struct.test01*, %struct.test01** %dep_field2

  ; These need to be converted by the AOS to SOA transformation.
  ; Note: currently each instruction is transformed independently, resulting
  ; the separate instructions for loading the base address for an element of
  ;  the structure of arrays. May need to revisit this if other transforms
  ;  do not remove the duplicates.
  %field1_addr = getelementptr %struct.test01, %struct.test01* %dep_field_val1, i64 0, i32 5
; CHECK:  [[GLOB_ADDR1:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 5
; CHECK:  [[ARRAY_ADDR1:%[0-9]+]] = load i8**, i8*** [[GLOB_ADDR1]]
; CHECK:  [[ZELEM_ADDR1:%[0-9]+]] = zext i32 %dep_field_val1 to i64
; CHECK:  %field1_addr = getelementptr i8*, i8** [[ARRAY_ADDR1]], i64 [[ZELEM_ADDR1]]

  %field2_addr = getelementptr %struct.test01, %struct.test01* %dep_field_val2, i64 0, i32 5
; CHECK:  [[GLOB_ADDR2:%[0-9]+]] = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 5
; CHECK:  [[ARRAY_ADDR2:%[0-9]+]] = load i8**, i8*** [[GLOB_ADDR2]]
; CHECK:  [[ZELEM_ADDR2:%[0-9]+]] = zext i32 %dep_field_val2 to i64
; CHECK:  %field2_addr = getelementptr i8*, i8** [[ARRAY_ADDR2]], i64 [[ZELEM_ADDR2]]

  %field1_val = load i8*, i8** %field1_addr
  %field2_val = load i8*, i8** %field2_addr

  ret void
}

declare i8* @calloc(i64, i64)
