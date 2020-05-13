; RUN: opt -dpcpp-kernel-barrier %s -S -o - | FileCheck %s

;;*****************************************************************************
; This test checks the Barrier pass
;; The case: kernel "main" with barrier instruction and the non-uniform value "%y"
;;           that crosses the barrier instruction and is an input to function "foo",
;;           which contains barrier itself and returns i32 value type that is not crossing barrier!
;; The expected result:
;;      1. Kernel "main" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      2. Kernel "main" stores "%y" value to offset 4 in the special buffer before calling "foo".
;;      3. Kernel "main" is still calling function "foo"
;;      4. function "foo" contains no more barrier/__builtin_dpcpp_kernel_barrier_dummyinstructions
;;      5. function "foo" loads "%x" value from offset 4 in the special buffer before xor.
;;      6. function "foo" does not store ret value ("%x") in the special buffer before ret.
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
; CHECK-LABEL: define void @main
define void @main(i32 %x) #0 {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %lid = call i32 @__builtin_get_local_id(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 1)
  %z = call i32 @foo(i32 %y)
  br label %L3
L3:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  ret void
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: xor
;;;; TODO: add regular expression for the below values.
; CHECK: L2:
; CHECK:   %SBIndex = load i32, i32* %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK:   %pSB_LocalId = bitcast i8* [[GEP0]] to i32*
; CHECK:   [[SBIndex1:%SBIndex[a-zA-Z0-9]+]] = load i32, i32* %pCurrSBIndex
; CHECK:   [[SB_LocalId_Offset1:%SB_LocalId_Offset[a-zA-Z0-9]+]] = add nuw i32 [[SBIndex1]], 0
; CHECK:   [[GEP1:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i32 [[SB_LocalId_Offset1]]
; CHECK:   [[pSB_LocalId1:%pSB_LocalId[a-zA-Z0-9]+]] = bitcast i8* [[GEP1]] to i32*
; CHECK:   %loadedValue = load i32, i32* [[pSB_LocalId1]]
; CHECK:   store i32 %loadedValue, i32* %pSB_LocalId
; CHECK:   br label %CallBB
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK: call i32 @foo
; CHECK: br label %
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret
}

; CHECK: define i32 @foo
define i32 @foo(i32 %x) {
L1:
  call void @__builtin_dpcpp_kernel_barrier_dummy()
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  ret i32 %x
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB1:
; CHECK:   [[SBIndex0:%SBIndex[a-zA-Z0-9]+]] = load i32, i32* %pCurrSBIndex
; CHECK:   [[SB_LocalId_Offset0:%SB_LocalId_Offset[a-zA-Z0-9]+]] = add nuw i32 [[SBIndex0]], 4
; CHECK:   [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i32 [[SB_LocalId_Offset0]]
; CHECK:   [[pSB_LocalId0:%pSB_LocalId[a-zA-Z0-9]+]] = bitcast i8* [[GEP0]] to i32*
; CHECK:   [[loadedValue0:%loadedValue[a-zA-Z0-9]+]] = load i32, i32* [[pSB_LocalId0]]
; CHECK:   %y = xor i32 [[loadedValue0]], [[loadedValue0]]
; CHECK:   br label %L2
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
;;;; TODO: add regular expression for the below values.
; CHECK: SyncBB0:
; CHECK:   %SBIndex = load i32, i32* %pCurrSBIndex
; CHECK:   %SB_LocalId_Offset = add nuw i32 %SBIndex, 4
; CHECK:   [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds i8, i8* %pSB, i32 %SB_LocalId_Offset
; CHECK:   %pSB_LocalId = bitcast i8* [[GEP0]] to i32*
; CHECK:   %loadedValue = load i32, i32* %pSB_LocalId
;; TODO_END ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier_dummy
; CHECK-NOT: @__builtin_dpcpp_kernel_barrier
; CHECK: ret i32 %loadedValue
}

declare void @__builtin_dpcpp_kernel_barrier(i32)
declare i32 @__builtin_get_local_id(i32)
declare void @__builtin_dpcpp_kernel_barrier_dummy()

attributes #0 = { "dpcpp-no-barrier-path"="false" "sycl_kernel" }
attributes #1 = { "dpcpp-no-barrier-path"="false" }
