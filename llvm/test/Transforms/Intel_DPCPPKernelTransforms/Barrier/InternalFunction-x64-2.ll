; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s
;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is not crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      2. Kernel "main" stores "%y" value to offset 8 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      5. function "foo" loads "%x" value from offset 8 in the special buffer before xor.
;;      6. function "foo" does not store ret value ("%x") in the special buffer before ret.
;;*****************************************************************************

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"

target triple = "x86_64-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i64 %x) #0 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %lid = call i64 @__builtin_get_local_id(i32 0)
  %y = xor i64 %x, %lid
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  %z = call i64 @foo(i64 %y)
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: xor
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: %1 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %1 to i64*
; CHECK: %SBIndex4 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset5 = add nuw i64 %SBIndex4, 0
; CHECK: %2 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset5
; CHECK: %pSB_LocalId6 = bitcast i8* %2 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId6
; CHECK: store i64 %loadedValue, i64* %pSB_LocalId
; CHECK: br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i64 @foo
; CHECK: br label %
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret
}

; CHECK-LABEL: define i64 @foo
define i64 @foo(i64 %x) #1 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %y = xor i64 %x, %x
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  ret i64 %x
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK: %SBIndex1 = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset2 = add nuw i64 %SBIndex1, 8
; CHECK: %0 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset2
; CHECK: %pSB_LocalId3 = bitcast i8* %0 to i64*
; CHECK: %loadedValue4 = load i64, i64* %pSB_LocalId3
; CHECK: %y = xor i64 %loadedValue4, %loadedValue4
; CHECK: br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB0:                                          ; preds = %LoopEnd_2
; CHECK: %SBIndex = load i64, i64* %pCurrSBIndex
; CHECK: %SB_LocalId_Offset = add nuw i64 %SBIndex, 8
; CHECK: %8 = getelementptr inbounds i8, i8* %pSB, i64 %SB_LocalId_Offset
; CHECK: %pSB_LocalId = bitcast i8* %8 to i64*
; CHECK: %loadedValue = load i64, i64* %pSB_LocalId
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret i64 %loadedValue
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare i64 @__builtin_get_local_id(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { "dpcpp-no-barrier-path"="false" }
