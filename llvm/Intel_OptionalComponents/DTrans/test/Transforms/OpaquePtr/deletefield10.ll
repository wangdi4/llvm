; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-deletefieldop -S -o - %s | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='dtrans-deletefieldop' -S -o - %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the size argument of memfunc calls are correctly.

%struct.test = type { i32, i64, i32 }

; CHECK: %__DFT_struct.test = type { i32, i32 }

define i32 @main(i32 %argc, i8** "intel_dtrans_func_index"="1" %argv) !intel.dtrans.func.type !6 {
  ; Allocate an array of structures.
  %base = zext i32 %argc to i64
  %n = add i64 %base, 4
  %sz = mul i64 %n, 16
  %p = call i8* @malloc(i64 %sz)
  %p_test = bitcast i8* %p to %struct.test*

  ; Zero initialize the structures
  call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %sz, i1 false)

  ; Call a function to do something to the first structure.
  %val = call i32 @doSomething(%struct.test* %p_test)

  ; Copy the first structure to the second.
  %p2 = getelementptr %struct.test, %struct.test* %p_test, i64 1
  %p2_i8 = bitcast %struct.test* %p2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p, i8* %p2_i8, i64 16, i1 false)

  ; Move the contents of the first two structures to the third and fourth
  %p3 = getelementptr %struct.test, %struct.test* %p_test, i64 2
  %p3_i8 = bitcast %struct.test* %p3 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p, i8* %p3_i8, i64 32, i1 false)

  ; Free the buffer
  call void @free(i8* %p)
  ret i32 %val
}
; CHECK-LABEL: define i32 @main
; CHECK: %base = zext i32 %argc to i64
; CHECK: %n = add i64 %base, 4
; CHECK: %sz.dt = mul i64 %n, 8
; CHECK: %sz = mul i64 %n, 8
; CHECK: %p = call {{.*}} @malloc(i64 %sz.dt)

; Ignore parameter pointer types and any type extensions to the intrinsic
; function names.

; CHECK: call void @llvm.memset{{.*}}({{.*}} %p, i8 0, i64 %sz, i1 false)
; CHECK: call void @llvm.memcpy{{.*}}({{.*}} %p, {{.*}} %p2_i8, i64 8, i1 false)
; CHECK: call void @llvm.memmove{{.*}}({{.*}} %p, {{.*}} %p3_i8, i64 16, i1 false)


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
  %sum = add i32 %valA, %valC

  ret i32 %sum
}

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !9 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)
declare !intel.dtrans.func.type !10 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !11 void @llvm.memmove.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1" , i8* "intel_dtrans_func_index"="2", i64, i1)
declare !intel.dtrans.func.type !12 void @free(i8* "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{i8 0, i32 2}  ; i8**
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = distinct !{!7}
!10 = distinct !{!7, !7}
!11 = distinct !{!7, !7}
!12 = distinct !{!7}
!13 = !{!"S", %struct.test zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }

!intel.dtrans.types = !{!13}
