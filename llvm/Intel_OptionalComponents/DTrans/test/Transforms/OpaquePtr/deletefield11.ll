; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the size argument of realloc calls are correctly
; updated when the function is cloned.
; (The function cloning only occurs with the non-opaque pointer form because
; the function signature changes. Once opaque pointers are in use, this case
; may no longer be relevant because the signature will not change.)

%struct.test = type { i32, i64, i32 }
; CHECK: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Allocate a structure.
  %p = call i8* @realloc(i8* null, i64 16)
  %p_test = bitcast i8* %p to %struct.test*

  ; Call a function to do something.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ret i32 %val
}
; CHECK-LABEL: define i32 @main
; CHECK: %p = call {{.*}} @realloc({{.*}} null, i64 8)


define i32 @doSomething(%struct.test* "intel_dtrans_func_index"="1" %p_test) !intel.dtrans.func.type !4 {
  ; Get pointers to each field
  %p_test_A = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 0
  %p_test_B = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 1
  %p_test_C = getelementptr %struct.test, %struct.test* %p_test, i64 0, i32 2

  ; read and write A and C
  store i32 1, i32* %p_test_A
  %valA = load i32, i32* %p_test_A
  store i32 2, i32* %p_test_C
  %valC = load i32, i32* %p_test_C

  ; Calculate a new size
  %mul = mul i32 64, %valC
  %sz = zext i32 %mul to i64

  ; Change the size of the buffer
  %p = bitcast %struct.test* %p_test to i8*
  %ra = call i8* @realloc(i8* %p, i64 %sz)
  ; FIXME: This use of %ra after the realloc should NOT be necessary, but we
  ;        don't transfer the type alias information from the realloc operatnd.
  %p_test2 = bitcast i8* %ra to %struct.test*
  %p_test2_A = getelementptr %struct.test, %struct.test* %p_test2, i64 0, i32 0

  ; Use the value where we had the size constant.
  icmp eq i32 128, %mul

  ; Free the buffer
  call void @free(i8* %ra)

  ret i32 %valA
}

; CHECK-LABEL: define internal i32 @doSomething
; CHECK: %mul.dt = mul i32 32, %valC
; CHECK: %mul = mul i32 64, %valC
; CHECK: %sz = zext i32 %mul.dt to i64
; CHECK: %ra = call {{.*}} @realloc({{.*}} %p, i64 %sz)
; CHECK: icmp eq i32 128, %mul

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @realloc(i8* "intel_dtrans_func_index"="2", i64) #0
declare !intel.dtrans.func.type !9 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("realloc") allocsize(1) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{i8 0, i32 2}  ; i8**
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7, !7}
!9 = distinct !{!7}
!10 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!10}
