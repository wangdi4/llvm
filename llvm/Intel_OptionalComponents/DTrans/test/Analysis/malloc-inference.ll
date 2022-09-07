; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-allocations -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes="require<dtransanalysis>" -dtrans-print-allocations -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies identification of calls to malloc that infer the type of
; allocation based on the storage of the allocated memory into a field member
; of a structure when the stored type is i8*. It also verifies that writes
; using byte-flattend GEPs are properly recognized.

%struct.test1.S1 = type { i32, i32, i32, i32 }
%struct.test1.S2 = type { i32, %struct.test1.S1* }

define void @test1(%struct.test1.S2* %in) {
  ; Get address of the member where the store will occur.
  %ptr = getelementptr %struct.test1.S2, %struct.test1.S2* %in, i64 0, i32 1
  %cur_val = load %struct.test1.S1*, %struct.test1.S1** %ptr
  %cmp = icmp eq %struct.test1.S1* %cur_val, null
  br i1 %cmp, label %do_alloc, label %done

do_alloc:
  ; mem = malloc(sizeof(%struct.test1.S1));
  %mem = call noalias i8* @malloc(i64 16)

  ; Perform writes using byte-flattend GEPs
  %mem32 = bitcast i8* %mem to i32*
  store i32 0, i32* %mem32
  %mem8_1 = getelementptr i8, i8* %mem, i32 4
  %mem32_1 = bitcast i8* %mem8_1 to i32*
  store i32 1, i32* %mem32_1
  %mem8_2 = getelementptr i8, i8* %mem, i32 8
  %mem32_2 = bitcast i8* %mem8_2 to i32*
  store i32 2, i32* %mem32_2
  %mem8_3 = getelementptr i8, i8* %mem, i32 12
  %mem32_3 = bitcast i8* %mem8_3 to i32*
  store i32 3, i32* %mem32_3

  ; Store the allocated memory into the member of %struct.test1.S2. This
  ; should trigger the inference that the allocated type is %struct.test1.S1.
  %ptr_i8 = bitcast %struct.test1.S1** %ptr to i8**
  store i8* %mem, i8** %ptr_i8
  br label %done

done:
  ret void
}
; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK:   %mem = call noalias i8* @malloc(i64 16)
; CHECK:   Detected type: %struct.test1.S1 = type { i32, i32, i32, i32 }


; This test is similar to the above, except that the first
; field of the structure is a pointer, in which DTrans analysis
; has special processing of the ptr-to-ptr bitcasts. This case should
; still resolve the appropriate allocated type and byte-flattened GEP
; access types.
%struct.test2.P1 = type { i32*, i32, i32, i32 }
%struct.test2.P2 = type { i32, %struct.test2.P1* }

define void @test2(%struct.test2.P2* %in) {
  ; mem = malloc(sizeof(%struct.test2.P1));
  %ptr = getelementptr %struct.test2.P2, %struct.test2.P2* %in, i64 0, i32 1
  %cur_val = load %struct.test2.P1*, %struct.test2.P1** %ptr
  %cmp = icmp eq %struct.test2.P1* %cur_val, null
  br i1 %cmp, label %do_alloc, label %done

do_alloc:
  %mem = call noalias i8* @malloc(i64 24)

  ; Perform writes using byte-flattend GEPs
  %memp32 = bitcast i8* %mem to i32**
  store i32* null, i32** %memp32
  %mem8_1 = getelementptr i8, i8* %mem, i32 8
  %mem32_1 = bitcast i8* %mem8_1 to i32*
  store i32 1, i32* %mem32_1
  %mem8_2 = getelementptr i8, i8* %mem, i32 12
  %mem32_2 = bitcast i8* %mem8_2 to i32*
  store i32 2, i32* %mem32_2
  %mem8_3 = getelementptr i8, i8* %mem, i32 16
  %mem32_3 = bitcast i8* %mem8_3 to i32*
  store i32 3, i32* %mem32_3

  ; Store the allocated memory into the member of %struct.test2.P2. This
  ; should trigger the inference that the allocated type is %struct.test2.P1.
  %ptr_i8 = bitcast %struct.test2.P1** %ptr to i8**
  store i8* %mem, i8** %ptr_i8
  br label %done

done:
  ret void
}
; CHECK: dtrans: Detected allocation cast to pointer type
; CHECK:    %mem = call noalias i8* @malloc(i64 24)
; CHECK:    Detected type: %struct.test2.P1 = type { i32*, i32, i32, i32 }

; Verify that the fields of the allocated structure are marked as Written.
; Currently, these structure will be marked as "Unsafe pointer store"
; because collection of the bitcast instruction that generates "%ptr_i8"
; only contains the field member information of "%struct.test2.P2". Because
; "%struct.test2.P1" starts with a pointer type, the bitcast does not
; result in the type directly containing "%struct.test2.P1**" as an aliased
; type. This could be improved in the future.

; CHECK: DTRANS_StructInfo:
; CHECK-LABEL: LLVMType: %struct.test1.S1 = type { i32, i32, i32, i32 }
; CHECK: Field info: Written
; CHECK: Field info: Written
; CHECK: Field info: Written
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK-LABEL: %struct.test1.S2 = type { i32, %struct.test1.S1* }
; CHECK: Safety data: No issues found

; CHECK-LABEL: LLVMType: %struct.test2.P1 = type { i32*, i32, i32, i32 }
; CHECK: Field info: Written
; CHECK: Field info: Written
; CHECK: Field info: Written
; CHECK: Field info: Written
; CHECK: Safety data: Unsafe pointer store
; CHECK-LABEL: %struct.test2.P2 = type { i32, %struct.test2.P1* }
; CHECK: Safety data: Unsafe pointer store

declare noalias i8* @malloc(i64)
