; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the index arguments of GEP instructions are correctly
; updated when they are accessing a field within a fixed array of a structure
; with a deleted field, and that the size of the allocation of the array is
; updated.

%struct.test = type { i32, i64, i32 }

define i32 @doSomething([4 x %struct.test]* "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !6 {
  ; Get pointers to each field
  %p_test_A = getelementptr [4 x %struct.test], [4 x %struct.test]* %p_test,
                            i64 0, i32 1, i32 0
  %p_test_B = getelementptr [4 x %struct.test], [4 x %struct.test]* %p_test,
                            i64 0, i32 1, i32 1
  %p_test_C = getelementptr [4 x %struct.test], [4 x %struct.test]* %p_test,
                            i64 0, i32 1, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  %sum = add i32 %valA, %valC
  ret i32 %sum
}

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !8 {
  ; Allocate a structure.
  %p = call i8* @malloc(i64 64)
  %p_test = bitcast i8* %p to [4 x %struct.test]*

  ; Call a function to do something.
  %val = call i32 @doSomething([4 x %struct.test]* %p_test)

  ; Free the structure
  call void @free(i8* %p)
  ret i32 %val
}

; CHECK: %__DFT_struct.test = type { i32, i32 }

; CHECK-LABEL: define i32 @main
; CHECK: %p = call i8* @malloc(i64 32)
; CHECK: %p_test = bitcast i8* %p to [4 x %__DFT_struct.test]*
; CHECK: %val = call i32 @doSomething.1([4 x %__DFT_struct.test]* %p_test)


; CHECK-LABEL: define internal i32 @doSomething.1
; CHECK: %p_test_A = getelementptr [4 x %__DFT_struct.test],
; CHECK-SAME:                      [4 x %__DFT_struct.test]* %p_test,
; CHECK-SAME:                      i64 0, i32 1, i32 0
; CHECK-NOT: %p_test_B = getelementptr
; CHECK: %p_test_C = getelementptr [4 x %__DFT_struct.test],
; CHECK-SAME:                      [4 x %__DFT_struct.test]* %p_test,
; CHECK-SAME:                      i64 0, i32 1, i32 1

declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !11 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{!4, i32 1}  ; [4 x %struct.test]*
!4 = !{!"A", i32 4, !5}  ; [4 x %struct.test]
!5 = !{%struct.test zeroinitializer, i32 0}  ; %struct.test
!6 = distinct !{!3}
!7 = !{i8 0, i32 2}  ; i8**
!8 = distinct !{!7}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9}
!11 = distinct !{!9}
!12 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!12}
