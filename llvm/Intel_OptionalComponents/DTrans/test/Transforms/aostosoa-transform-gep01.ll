; Legacy pass manager runs:
; RUN: sed -e s/.T1:// %s | \
; RUN:   opt -whole-program-assume -S -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | \
; RUN:   FileCheck --check-prefix=CHECK-ALWAYS --check-prefix=CHECK-TEST1 %s

; RUN: sed -e s/.T2:// %s | \
; RUN:   opt -whole-program-assume -S -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | \
; RUN:   FileCheck %s --check-prefix=CHECK-ALWAYS --check-prefix=CHECK-TEST2

; New pass manager runs:
; RUN: sed -e s/.T1:// %s | \
; RUN: opt -whole-program-assume -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | \
; RUN:   FileCheck %s --check-prefix=CHECK-ALWAYS --check-prefix=CHECK-TEST1

; RUN: sed -e s/.T2:// %s | \
; RUN: opt -whole-program-assume -S -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | \
; RUN:   FileCheck %s --check-prefix=CHECK-ALWAYS --check-prefix=CHECK-TEST2


; This use of 'sed' above allows for this test to be used for verifying the
; behavior when the function is not cloned by the DTrans base class and also
; when the function is cloned.
;
; When 'T1:' is replaced by 'sed', the function signature does not change, and
; the function will not be cloned. In this case the results to be tested are
; the marked with CHECK-ALWAYS and CHECK-TEST1.
;
; When 'T2:' is replaced by 'sed', the function signature is defined to take
; a parameter of a type that needs to be replaced, causing the function to be
; cloned. In this case the results to be tested are marked with the
; CHECK-ALWAYS and CHECK-TEST2 cases.


; This test verifies that GetElementPointer instructions get transformed
; properly during the AOS to SOA transformation.


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

; These calls are for case 1 where we do not want the function to be cloned.
;T1:  call void @test01(i64 2)
;T1:  call void @test02()

; These calls are for case 2 where we want the function to be cloned.
;T2:  %base = load %struct.test01*, %struct.test01** @g_test01ptr
;T2:  call void @test01(i64 2, %struct.test01* %base)
;T2:  %dep_base = load %struct.test01dep*, %struct.test01dep** @g_test01depptr
;T2:  call void @test02(%struct.test01dep* %dep_base)

  ret i32 0
}

; This function tests the basic transformations of GEP instructions.
;T1: define void @test01(i64 %idx1) {
;T2: define void @test01(i64 %idx1, %struct.test01* %base) {
; CHECK-TEST1: define internal void @test01(i64 %idx1)
; CHECK-TEST2: define internal void @test01.1(i64 %idx1, i64 %base)

;T1:  %base = load %struct.test01*, %struct.test01** @g_test01ptr
; CHECK-TEST1:  %base = load i64, i64* @g_test01ptr

  ; Test with GEP that accesses a constant array index into the SOA type.
  %array_elem1 = getelementptr %struct.test01, %struct.test01* %base, i64 5
; CHECK-ALWAYS:  %array_elem1 = add i64 %base, 5

  ; Test with GEP that accesses a field element. This will require loading
  ; the 2nd pointer field from the structure of arrays, and then indexing into
  ; that array.
  %z_addr1 = getelementptr %struct.test01, %struct.test01* %array_elem1, i32 0, i32 2
  store i32 -1, i32* %z_addr1
; CHECK-ALWAYS:  %1 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK-ALWAYS:  %2 = load i32*, i32** %1
; CHECK-ALWAYS:  %z_addr1 = getelementptr i32, i32* %2, i64 %array_elem1
; CHECK-ALWAYS:  store i32 -1, i32* %z_addr1

  ; Test with GEP that accesses a non-constant array index into the SOA type.
  ; Here we need to add the GEP index amount to the peeling index value.
  %arrayidx_elem2 = getelementptr %struct.test01, %struct.test01* %base, i64 %idx1
; CHECK-ALWAYS:  %arrayidx_elem2 = add i64 %base, %idx1

  ; Test with GEP to access a field element, where the first GEP index equals 0.
  ; In this case, we do not need to perform an addition to the array index that
  ; will be accessed.
  %x_addr2 = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 0, i32 0
  store i32 20, i32* %x_addr2
; CHECK-ALWAYS:  %3 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK-ALWAYS:  %4 = load i32*, i32** %3
; CHECK-ALWAYS:  %x_addr2 = getelementptr i32, i32* %4, i64 %array_elem1
; CHECK-ALWAYS:  store i32 20, i32* %x_addr2

  ; Test with GEP that directly uses index to get to a field element. Here the
  ; addition of the array_elem1 with index should be performed to determine the
  ; peeled index value for which array element of the SOA should be accessed.
  %y_addr3 = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 %idx1, i32 1
  %new_val = load i32, i32* %x_addr2
  store i32 %new_val, i32* %y_addr3
; CHECK-ALWAYS:  %5 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK-ALWAYS:  %6 = load i32*, i32** %5
; CHECK-ALWAYS:  %7 = add i64 %array_elem1, %idx1
; CHECK-ALWAYS:  %y_addr3 = getelementptr i32, i32* %6, i64 %7
; CHECK-ALWAYS:  %new_val = load i32, i32* %x_addr2
; CHECK-ALWAYS:  store i32 %new_val, i32* %y_addr3

  ; Test with GEP to access a field element that is a pointer to the type being
  ; transformed. In this case a inttoptr cast will be necessary between the pre/post
  ; processing phases, but will be removed by the time the IR is emitted. In the
  ; end, we should just see i64 types being used for the loaded value.
  %nextptr_addr4 = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 %idx1, i32 3
  %nextptr_val = load %struct.test01*, %struct.test01** %nextptr_addr4
  %nextptr_cmp = icmp eq %struct.test01* %nextptr_val, null
; CHECK-ALWAYS:  %8 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 3
; CHECK-ALWAYS:  %9 = load i64*, i64** %8
; CHECK-ALWAYS:  %10 = add i64 %array_elem1, %idx1
; CHECK-ALWAYS:  %nextptr_addr4 = getelementptr i64, i64* %9, i64 %10
; CHECK-ALWAYS:  %nextptr_val = load i64, i64* %nextptr_addr4
; CHECK-ALWAYS:  %nextptr_cmp = icmp eq i64 %nextptr_val, 0

  ; Test with GEP to access a field element that is a pointer to a dependent type being
  ; transformed. In this case a bitcast will be necessary between the pre/post
  ; processing phases, but will be removed by the time the IR is emitted.
  %parentptr_addr4 = getelementptr %struct.test01, %struct.test01* %array_elem1, i64 %idx1, i32 4
  %parentptr_val = load %struct.test01dep*, %struct.test01dep** %parentptr_addr4
  %parentptr_cmp = icmp eq %struct.test01dep* %parentptr_val, null
; CHECK-ALWAYS:  %11 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 4
; CHECK-ALWAYS:  %12 = load %__SOADT_struct.test01dep**, %__SOADT_struct.test01dep*** %11
; CHECK-ALWAYS:  %13 = add i64 %array_elem1, %idx1
; CHECK-ALWAYS:  %parentptr_addr4 = getelementptr %__SOADT_struct.test01dep*, %__SOADT_struct.test01dep** %12, i64 %13
; CHECK-ALWAYS:  %parentptr_val = load %__SOADT_struct.test01dep*, %__SOADT_struct.test01dep** %parentptr_addr4
; CHECK-ALWAYS:  %parentptr_cmp = icmp eq %__SOADT_struct.test01dep* %parentptr_val, null

  ret void
}

; Test with using GEP instructions on types being transformed that are loaded from
; another structure.
;T1: define void @test02() {
;T2: define void @test02(%struct.test01dep* %dep_base) {
; CHECK-TEST1: define internal void @test02()
; CHECK-TEST2: define internal void @test02.2(%__SOADT_struct.test01dep* %dep_base)

;T1:  %dep_base = load %struct.test01dep*, %struct.test01dep** @g_test01depptr

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
; CHECK-ALWAYS:  %1 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 5
; CHECK-ALWAYS:  %2 = load i8**, i8*** %1
; CHECK-ALWAYS:  %field1_addr = getelementptr i8*, i8** %2, i64 %dep_field_val1

  %field2_addr = getelementptr %struct.test01, %struct.test01* %dep_field_val2, i64 0, i32 5
; CHECK-ALWAYS:  %3 = getelementptr %__SOA_struct.test01, %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 5
; CHECK-ALWAYS:  %4 = load i8**, i8*** %3
; CHECK-ALWAYS:  %field2_addr = getelementptr i8*, i8** %4, i64 %dep_field_val2

  %field1_val = load i8*, i8** %field1_addr
  %field2_val = load i8*, i8** %field2_addr

  ret void
}

declare i8* @calloc(i64, i64)
